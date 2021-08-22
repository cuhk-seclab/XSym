<?php
/*
Translate PHP into C (constraints).
*/
include_once __DIR__ . "/vendor/autoload.php";

$program_template = "
#include \"php-built-in.h\"
int sync_prof() {
    VARIABLE_DEFINITION

    if(CONTROL_FLOW) {
        if(DATA_FLOW){
            assert(0); // synthesized checkpoint
        }
    }
}
";

use PhpParser\ParserFactory;
use PhpParser\NodeVisitorAbstract;
use PhpParser\Node;
use PhpParser\Node\Expr;
use PhpParser\NodeTraverser;
use PhpParser\PrettyPrinter\Standard as Printer;

function varname ($expr) {
    $printer = new Printer;
    return str_replace(array('$', '[', ']', '\'', '"'), '_', $printer->prettyPrintExpr($expr));

}
class exprvisitor extends NodeVisitorAbstract {
    public $exprs = [];
    public $variables = [];
    public $typesets = [];
    public $sametypes = [];
    public $counter = 0;

    public function enterNode (Node $node) {
        if ($node instanceof Node\Expr) {
            $varname = varname($node);

            $type = null;
            if ($node instanceof Expr\BinaryOp\Equal or
                $node instanceof Expr\BinaryOp\Equal or
                $node instanceof Expr\BinaryOp\NotEqual or
                $node instanceof Expr\BinaryOp\NotIdentical or
                $node instanceof Expr\BinaryOp\Identical) {
                /// treat two set as identical.
                $left = varname($node->left);
                $right = varname($node->right);
                $this->typesets[$varname] = 'php_boolean';

                if(array_key_exists($left, $this->typesets)){
                    $this->typesets[$right] = $this->typesets[$left];
                }
                elseif(array_key_exists($right, $this->typesets)){
                    $this->typesets[$left] = $this->typesets[$right];
                }
                else{
                    if(array_key_exists($left, $this->sametypes) && array_key_exists($left, $this->sametypes)){
                        if($this->sametypes[$left] != $this->sametypes[$right]) {
                            foreach($this->sametypes as $var => $idx) {
                                if($idx == $this->sametypes[$left]) {
                                    $this->sametypes[$var] = $this->sametypes[$right];
                                }
                            }
                        }

                    }
                    elseif(array_key_exists($left, $this->sametypes)){
                        $this->sametypes[$right] = $this->sametypes[$left];
                    }
                    elseif(array_key_exists($right, $this->sametypes)){
                        $this->sametypes[$left] = $this->sametypes[$right];
                    }
                    else{
                        $this->sametypes[$left] = $this->sametypes[$right] = $this->counter;
                        $this->counter ++;
                    }
                }
            }
            elseif (strpos(get_class($node),'Logical') !== false or
                strpos(get_class($node),'Boolean') !== false or
                strpos($varname, 'is') === 0
            ) {
                // logic operations
                $this->typesets[$varname] = 'php_boolean';
            }
            elseif ($node instanceof Expr\BinaryOp\Concat or
                strpos(get_class($node),'String') !== false) {
                $this->typesets[$varname] = 'php_string';
            }
            elseif ($node instanceof Expr\FuncCall and strpos($varname, 'str') === 0) {
                $this->typesets[$varname] = 'php_string';
            }
            else {
                $this->typeset[$varname] = 'php_number';
            }
            $this->exprs[] = $node;


            if ($node instanceof Node\Expr\Variable or $node instanceof Node\Expr\ArrayDimFetch) {
                if(! in_array($varname, $this->variables))
                    $this->variables[] = varname($node);
            }
        }
    }
}

function parse($file) {
    $file_f = fopen($file, "r");
    $parser = (new ParserFactory)->create(ParserFactory::PREFER_PHP7);
    $code = fread($file_f, filesize($file));
    fclose($file_f);	
    try {
        $stmts = $parser->parse($code);
        return $stmts;
    } catch(PhpParser\Error $e) {
        echo "Parsing" . $file . " error: ", $e->getMessage();
        exit(1);
    }
}

$file = "test.php";
$stmts = parse($file);

/** 
 * Nodetraverser traverses all the nodes with root-first order.
 * Thus type sets are collected.
 **/
$traverser = new NodeTraverser();
$visitor = new exprvisitor();
$traverser->addVisitor($visitor);
$traverser->traverse($stmts);
$exprs = $visitor->exprs;


$sets = [];
foreach($visitor->sametypes as $var => $idx){
    if (! array_key_exists($idx, $sets)) {
        $sets[$idx] = [$var];
    }
    else{
        $sets[$idx][] = $var;
    }
}

foreach($sets as $idx => $vars) {
    $type = null;
    foreach($vars as $var) {
        if(array_key_exists($var, $visitor->typesets)) {
            $type = $visitor->typesets[$var];
            break;
        }
    }
    if ($type != null) {
        foreach($vars as $var)
            $visitor->typesets[$var] = $type;
    }
    else {
        foreach($vars as $var)
            $visitor->typesets[$var] = "php_string";
    }
}

echo "==========\n";
$definition = '';
foreach($visitor->variables as $var) {
    $definition .= "\t" . $visitor->typesets[$var] . " " . "$var;\n" ;
}

echo str_replace('VARIABLE_DEFINITION', $definition, $program_template);
