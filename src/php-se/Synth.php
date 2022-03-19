<?php
/*
Translate PHP into C (constraints).
*/
include_once __DIR__ . "/vendor/autoload.php";

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
        //print get_class($node);
        if ($node instanceof Node\Expr) {
            $varname = varname($node);
            // print(get_class($node));
            // print($varname . "\n");
            $type = null;
            if ($node instanceof Expr\BinaryOp\Equal or
                $node instanceof Expr\BinaryOp\Equal or
                $node instanceof Expr\BinaryOp\NotEqual or
                $node instanceof Expr\BinaryOp\NotIdentical or
                $node instanceof Expr\BinaryOp\Identical) {
                // treat two set as identical.
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
            elseif ($node instanceof Expr\BinaryOp) {
                $left = varname($node->left);
                $right = varname($node->right);
                $this->typesets[$left] = $this->typesets[$right] = 'php_number';
            }
            elseif ($node instanceof Expr\FuncCall and strpos($varname, 'str') === 0) {
                $this->typesets[$varname] = 'php_string';
            }
            elseif ($node instanceof Node\Scalar\LNumber) {
                $this->typesets[$varname] = 'php_number';
            }
            else {
                $this->typeset[$varname] = 'php_string';
            }
            $this->exprs[] = $node;


            if ($node instanceof Node\Expr\Variable or $node instanceof Node\Expr\ArrayDimFetch) {
                if(! in_array($varname, $this->variables))
                    $this->variables[] = $varname;
            }
        }
    }
}

function parse($code) {

    try {
        $parser = (new ParserFactory)->create(ParserFactory::PREFER_PHP7);
        $stmts = $parser->parse($code);
        return $stmts;
    } catch(PhpParser\Error $e) {
        echo "Parsing error: ", $e->getMessage();
        exit(1);
    }
}


function myvisit($stmts) {
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
    return $visitor;
}
$program_template = 
"#include \"php-built-in.h\"
int main() {
VARIABLE_DEFINITION
    if(CONTROL_FLOW) {
        if(DATA_FLOW){
            assert(0); // synthesized checkpoint
        }
    }
}";
$CC_file= $argv[1];
$DC_file = $argv[2];
if(!file_exists($CC_file) or !file_exists($DC_file))
    exit(1);

$CC_file_fp = fopen($CC_file, "r");
$content = fread($CC_file_fp, filesize($CC_file));
fclose($CC_file_fp);
$temp = explode("---\n", $content);
$CC_cst = explode("===\n", $temp[1]);

$DC_file_fp = fopen($DC_file, "r");
$content = fread($DC_file_fp, filesize($DC_file));
fclose($DC_file_fp);
$temp = explode("---\n", $content);
$DC_cst = explode("===\n", $temp[1]);


for($idx = 0; $idx < count($CC_cst); $idx ++) {
    $c = $CC_cst[$idx];
    $code = substr( "(" . str_replace("\n", ") && (", $c) . ")", 0, -6);
    //print($code); 
    $cc = str_replace(array('$', '[', ']', '\'', '"'), '_', $code);
    //print($cc);
    $code = str_replace("\n", ";\n", $c);
    $stmts = parse("<?php\n" . $code);
    $cc_visitor = myvisit($stmts);

    $c = $DC_cst[$idx];
    $code = substr( "(" . str_replace("\n", ") && (", $c) . ")", 0, -6);
    $dc = str_replace(array('$', '[', ']', '\'', '"'), '_', $code);
    $code = str_replace("\n", ";\n", $c);
    $stmts = parse("<?php\n" . $code);
    $dc_visitor = myvisit($stmts);

    $definition = '';
    foreach($cc_visitor->variables as $var) {
        $definition .= "    " . $cc_visitor->typesets[$var] . " " . "$var;\n" ;
    }
    // foreach($dc_visitor->variables as $var) {
    //     print("$var");
    //     $definition .= "    " . $dc_visitor->typesets[$var] . " " . "$var;\n" ;
    // }

    $program = str_replace('VARIABLE_DEFINITION', $definition, $program_template);
    $program = str_replace('CONTROL_FLOW', $cc, $program);
    $program = str_replace('DATA_FLOW', $dc, $program);
    echo $program;
    file_put_contents("main.c", $program);
}

