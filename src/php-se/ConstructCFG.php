<?php
use PhpParser\ParserFactory;
use PhpParser\Node;
use PhpParser\Node\Expr;
use PhpParser\Node\Expr\AssignOp;
use PhpParser\Node\Expr\BinaryOp;
use PhpParser\Node\Name;
use PhpParser\Node\Scalar;
use PhpParser\Node\Scalar\MagicConst;
use PhpParser\Node\Cast;
use PhpParser\Node\Stmt;
use PhpParser\PrettyPrinterAbstract;

include_once __DIR__ ."/vendor/autoload.php";
include_once __DIR__ . "/Utility.php";
include_once __DIR__ . "/Definition/Constants.php";
include_once __DIR__ . "/Definition/CFGNode.php";
include_once __DIR__ . "/Definition/MyFunction.php";
include_once __DIR__ . "/Definition/MyClass.php";


$nodeindex = 0;
/**
 * Construct CFGs for a PHP App
 * @param string $PathToFile 
 * @return array [MyFunctions, MyClasses]
 */
function ConstructAppCFG($AppPath) {
    $FileList = FindAllFiles($AppPath);
    $AllClasses = [];
    $AllClasses[MAIN_CLASS] = new MyClass(MAIN_CLASS);
    foreach($FileList as $FileName) {
        $Result = ConstructFileCFG($FileName);
        $TempMainClass = $Result[0];
        $TempOtherClass = $Result[1];
        foreach($TempMainClass->ClassMethods as $MethodName => $MethodItem)
            $AllClasses[MAIN_CLASS]->ClassMethods[$MethodName] = $MethodItem;
        foreach($TempOtherClass as $ClassName => $ClassItem)
            $AllClasses[$ClassName] = $ClassItem;
    }
    return $AllClasses;
}

/**
 * Construct CFGs for a PHP file
 * @param string $PathToFile 
 * @return array [MyFunctions, MyClasses]
 */
function ConstructFileCFG($PathToFile) {
	$FileHandle = fopen($PathToFile, "r");
	$Parser = (new ParserFactory)->create(ParserFactory::PREFER_PHP7);
	$Contents = fread($FileHandle, Filesize($PathToFile));
	fclose($FileHandle);	
	$Stmts=array();
	try {
		$Stmts = $Parser->parse($Contents);	
	} catch(PhpParser\Error $e) {
	  	echo 'Parse Error: ', $e->getMessage();
	}
    $MainClass = new MyClass(MAIN_CLASS);
    $Classes = [];
	foreach($Stmts as $Stmt) {
	    if($Stmt instanceof Stmt\Function_){ 
            $FunctionCFG = ConstructFunctionCFG($Stmt, MAIN_CLASS);
            $FunctionCFG->FileName = $PathToFile;
            $MainClass->ClassMethods[$FunctionCFG->FuncName] = $FunctionCFG;
	    }
        elseif($Stmt instanceof PhpParser\Node\Stmt\Class_) {
            $Class = constructClassCFG($Stmt);
            $Class->FileName = $PathToFile;
            $Classes[$Class->ClassName] = $Class;
            if($Stmt->extends instanceof Node\Name) {
                $Class->Extends = implode('', $Stmt->extends->parts);
            }
        }
    }
    $FileMainCFG = new MyFunction(MAIN_CLASS, $PathToFile);
    $FileMainCFG->FileName = $PathToFile;
    $FileMainCFG->Body = ConstructCFG($Stmts);
    $MainClass->ClassMethods[$PathToFile] = $FileMainCFG;
    return [$MainClass, $Classes];
}


/**
 * Construct CFG for a function node
 * @param \Stmt\Function_ 
 * @return MyFunction represent the function
 */
function ConstructFunctionCFG($FunctionNode, $ClassName) {
    $ResultFunction = new MyFunction($ClassName, (string)$FunctionNode->name);
    $ResultFunction->Body = constructCFG($FunctionNode->stmts);
    //  param->var Expr/Variable => default
    foreach($FunctionNode->params as $Param)
        if($Param->var->name instanceof Node\Name)
            $ResultFunction->Params[implode('', $Param->var->name->parts)] = $Param->default;
        elseif(is_string($Param->var->name))
            $ResultFunction->Params[$Param->var->name] = $Param->default;
    return $ResultFunction;
}

/**
 * Construct CFG for a class node
 * @param \Stmt\Class_ $ClassNode
 * @return MyClass represent the class
 */
function ConstructClassCFG($ClassNode) {
    $ResultClass = new MyClass((string)$ClassNode->name);
    foreach($ClassNode->stmts as $StmtItem) { 
        if($StmtItem instanceof Stmt\Property) {
            foreach($StmtItem->props as $Property) {
                if($Property->default)
                    $ResultClass->Props[(string)$Property->name] = $Property->default;
                else
                    $ResultClass->Props[(string)$Property->name] = NULL;
            }
        }
        else if ($StmtItem instanceof Stmt\ClassMethod) {
            $FunctionCFG = ConstructFunctionCFG($StmtItem, $ClassNode->name);
            $ResultClass->ClassMethods[$FunctionCFG->FuncName] = $FunctionCFG;
        }
        else if($StmtItem instanceof Stmt\ClassConst) {
            foreach($StmtItem->consts as $Const)
                $ResultClass->Consts[(string)$Const->Name->Name] = $Const->value;
        }
    }
    $ClassNode->extends;
    return $ResultClass;
}


function ConstructCFG($Statements){
    global $nodeindex;
	$Entry = new CFGNodeAbstract($nodeindex ++);
	$Entry->Parent = NULL;
	$Entry->Type = Start;
    if(is_NULL($Statements) or count($Statements) == 0) {
	    $End = new CFGNodeAbstract($nodeindex ++);
        $End->Parent = $Entry;
        $Entry->Child = $End;
        $End->Type = End;
        return [$Entry, $End];
    }
	$Parent = $Entry;
    foreach ($Statements as $Stmt) {
        if ($Stmt instanceof Stmt\If_) {
            $Parent = ProcessIfStmt($Stmt, $Parent);
        }
        else if ($Stmt instanceof Stmt\Foreach_ || $Stmt instanceof Stmt\For_ || 
            $Stmt instanceof Stmt\While_ || $Stmt instanceof Stmt\Do_){
            $Parent = ProcessLoopStmtAsIfStmt($Stmt, $Parent);
        }
        else if($Stmt instanceof Stmt\Switch_)
            $Parent = ProcessSwitchStmt($Stmt, $Parent);
        else if($Stmt instanceof Stmt\TryCatch) {
            continue;
        }
        else if ($Stmt instanceof Stmt\Namespace_){
            continue;
        }
        elseif(($Stmt instanceof Stmt\Nop) || ($Stmt instanceof Stmt\Function_) || 
            ($Stmt instanceof Stmt\Class_)) {
            continue;
        }
        else {                    
            $GeneralNode = new CFGNodeAbstract($nodeindex ++);
            $GeneralNode->Type = Stmt;
            $GeneralNode->Stmt = $Stmt;
            $GeneralNode->Parent = $Parent;
            $Parent->Child = $GeneralNode;
            $Parent = $GeneralNode;
        }
    }
    $End = new CFGNodeAbstract($nodeindex ++);
    $End->Type = End;
    $Parent->Child = $End;
    $End->Parent = $Parent;
    $End->Child = NULL;
    return [$Entry, $End];
}
/**
 * process ifstmt in AST
 * @param Stmt\If_ $IfStmt
 * @param CFGNode $Parent
 * @return CFGNode 
 */
function ProcessIfStmt($IfStmt, $Parent) {
    global $nodeindex;
    $LocalIndex = 0;
    $IfConditions = [];
    $IfBodies = [];
    $IfConditions[$LocalIndex] = $IfStmt->cond;
    $IfBodies[$LocalIndex ++] = ConstructCFG($IfStmt->stmts);
    foreach($IfStmt->elseifs as $elseif) {
        $IfConditions[$LocalIndex] = $elseif->cond;
        $IfBodies[$LocalIndex ++] = ConstructCFG($elseif->stmts);
    }
    if ($IfStmt->else) {
        $IfConditions[$LocalIndex] = NULL;
        $IfBodies[$LocalIndex ++] = ConstructCFG($IfStmt->else->stmts);
    }
    $IfNode = new CFGIfNode($nodeindex ++, $IfConditions, $IfBodies);
    $Parent->Child = $IfNode;
    $IfNode->Parent = $Parent;
    return $IfNode;
}

/**
 * Process loopstmt in AST, translate to IF-ELSE
 * @param Stmt\Loop_ $LoopStmt
 * @param CFGNode $Parent
 * @return CFGNode 
 */
function ProcessLoopStmtAsIfStmt($LoopStmt, $Parent) {
    // TODO Check: The cond/expr in loopstmt can be array or individual expr
    global $nodeindex;
    $Condtions = [];
    if(isset($LoopStmt->cond))
        $Conditions[] = $LoopStmt->cond;
    else
        $Conditions[] = $LoopStmt->expr;
    $LoopBodies = [];
    $LoopBodies[] = ConstructCFG($LoopStmt->stmts);
    $LoopBodies[] = NULL;
    $IfNode = new CFGIfNode($nodeindex ++, $Conditions, $LoopBodies);
    if($LoopStmt instanceof Stmt\Foreach_) {
        $Assign = new PhpParser\Node\Expr\Assign($LoopStmt->valueVar, $LoopStmt->expr);
        $GeneralNode = new CFGNodeAbstract($nodeindex ++);
        $GeneralNode->Type = Stmt;
        $GeneralNode->Stmt = $Assign;
        $GeneralNode->Parent = $Parent;
        $Parent->Child = $GeneralNode;
        $Parent = $GeneralNode;
    }
    $IfNode->Parent = $Parent;
    $Parent->Child = $IfNode;
    return $IfNode;
}
/**
 * process loopstmt in AST
 * @param Stmt\Loop_ $LoopStmt
 * @param CFGNode $Parent
 * @return CFGNode 
 */
function ProcessLoopStmt($LoopStmt, $Parent) {
    global $nodeindex;
    $Init = $Loop = NULL;
    $LooptType = NULL;
    $Conditions = NULL;
    if ($LoopStmt instanceof Stmt\Foreach_) {
        $Conditions = $LoopStmt->expr;
        $LoopType = ForeachLoop;
    }
    elseif ($LoopStmt instanceof Stmt\For_) {
        $Init = $LoopStmt->init;
        $Conditions = $LoopStmt->cond;
        $Loop = $LoopStmt->loop; 
        $LoopType = ForLoop;
    }
    elseif ($LoopStmt instanceof Stmt\While_) {
        $Conditions = $LoopStmt->cond;
        $LoopType = WhileLoop;
    }
    else if ($LoopStmt instanceof Stmt\Do_) {
        $Condtions = $LoopStmt->cond;
        $LoopType = DoWhileLoop;
    }
    $LoopBody = ConstructCFG($LoopStmt->stmts);
    $LoopNode = new CFGLoopNode($nodeindex ++, $LoopType, $Conditions, $LoopBody, $Init, $Loop);
    $LoopNode->Parent = $Parent;
    $Parent->Child = $LoopNode;
    return $LoopNode;
}

/**
 * process loopstmt in AST
 * @param Stmt\Loop_ $LoopStmt
 * @param CFGNode $Parent
 * @return CFGNode 
 */
function ProcessSwitchStmt($SwitchStmt, $Parent) {
    global $nodeindex;
    $LocalIndex = 0;
    $Conditions = [];
    $Bodies = [];
    foreach($SwitchStmt->cases as $case){        
        if($case->cond){
            $Bodies[$LocalIndex] = ConstructCFG($case->stmts);
            $Condtions[$LocalIndex ++] = new BinaryOp\Equal($case->cond, CloneObject($SwitchStmt->cond));
        }
        else
            $default = $case;
    }
    if(isset($default)){
        $Bodies[$LocalIndex] = NULL;
        $Bodies[$LocalIndex ++] = ConstructCFG($default->stmts);
    }
    $IfNode = new CFGIfNode($nodeindex ++, $Condtions, $Bodies);
    $Parent->Child = $IfNode;
    $IfNode->Parent = $Parent;
    return $IfNode;
}
