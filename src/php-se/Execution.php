<?php
require_once __DIR__ ."/Definition/Variable.php";
include_once __DIR__ ."/Definition/GlobalVariable.php";
require_once __DIR__ ."/Definition/Slice.php";

//use PhpParser;
use PhpParser\Node;
use PhpParser\Node\Stmt;
use PhpParser\Node\Expr;

class Execution{
    public $ReturnTypes = [];

    public $ReturnTainted = false;

    public $XSS;
    public $IsTainted;


    public $Constraints = [];
    public $XSSConstraints = [];
    public $XSSDataConstraints = [];

    public function ExecutionPerNode($Start, $End, $Slice) {
        $CurrentNode = $Start;
        $reachend = false;
        if($CurrentNode != NULL) {
            //  echo $CurrentNode->Type, "\n";
            if($CurrentNode->Type == IfStmt) {
                $size = count($CurrentNode->Conditions);
                $slice_array = [];
                for($i = 0; $i < $size; $i ++) {
                    //  TODO copy slice
                    $SliceCopy = CloneObject($Slice);
                    $Cond = $this->ExecutionPerExpr($CurrentNode->Conditions[$i], $SliceCopy);
                
                    Operation::realprint($Cond);

                    if($CurrentNode->Conditions[$i] instanceof PhpParser\Node)
                        $line = $CurrentNode->Conditions[$i]->getLine();
                    else $line = -1;
                    $SliceCopy->Constraints[] = $Cond;

                    $slice_array[] = $SliceCopy;
                    $Body = $CurrentNode->Bodies[$i];
                    $this->ExecutionPerNode($Body[0], $Body[1], $SliceCopy);
                    $this->ExecutionPerNode($CurrentNode->Child, NULL, $SliceCopy);
                    //return true;
                }
                return true;
            }
            else if($CurrentNode->Type == Stmt) {
                $this->ExecutionPerExpr($CurrentNode->Stmt, $Slice);
                if($this->XSS){
                    //$this->SaveConstraints($Slice->XSSConstraints, "XSS");
                    //$this->SaveConstraints($Slice->XSSDataConstraints, "XSSData");
                    $this->XSS = false;  
                }
                $this->ExecutionPerNode($CurrentNode->Child, NULL, $Slice);
                return true;
            }
            $this->ExecutionPerNode($CurrentNode->Child, NULL, $Slice);
        }
        else {
            print("end\n");
            $this->SaveConstraints($Slice->Constraints);
        }
        return true;
    }

    public function SaveConstraints($Constraints, $Type = NULL) {
        $str = "";
        foreach($Constraints as $cond) {
            $str .= Operation::realprint($cond);
        }
        if(!in_array($str, $this->Constraints))
            $this->Constraints[] = $str;
    }
        

    public function DumpConstraints($ClassName, $MethodName, $Params) {

        $pstr = implode(",", $Params);
        $cst =  implode("===\n", $this->Constraints);
        $xsscst = implode("===\n", $this->XSSConstraints);
        $xssdatacst = implode("===\n", $this->XSSDataConstraints);
        $MethodName = str_replace("/", "-", $MethodName);
        file_put_contents($ClassName . "-" . $MethodName . ".txt", $pstr . "\n---\n" . $cst);
        file_put_contents($ClassName . "-" . $MethodName . "-XSS.txt", $pstr . "\n---\n" . $xsscst);
        file_put_contents($ClassName . "-" . $MethodName . "-XSSData.txt", $pstr . "\n---\n" . $xssdatacst);
    }
    
    /**
     * Execute an expression based current 
     * variable values. It updates variabls in $Slice 
     * according to the $Expr passed to.
     * @param Expr expression 
     * @param Path slice
     * @return return Expr.
     */
    public function ExecutionPerExpr($Expr, $Slice) {

        //print get_class($Expr);
        if($Expr == NULL)
            return new Variable(); 
        if($Expr instanceof Stmt\Expression) {
            return $this->ExecutionPerExpr($Expr->expr, $Slice);
        }
        elseif($Expr instanceof Expr\ConstFetch) {
            $default =  new Variable();
            $default->Value = $Expr;
            if(strtolower($Expr->name) == 'true') {
                $default->Types = [Variable::Bool_];
            }
            elseif(strtolower($Expr->name) == 'false') {
                $default->Types = [Variable::Bool_];
            }
            return $default;
        }
        elseif($Expr instanceof Expr\Assign or
            $Expr instanceof Expr\AssignRef) {
            $ret = $this->ExecutionPerExpr($Expr->expr, $Slice);
            $this->StoreValueToSlice($Expr->var, $ret, $Slice);
            return $ret;
        }
        elseif($Expr instanceof Expr\AssignOp) {
            // Convert assignOp to assign
            $ExprType = \get_class($Expr);
            $ExprType = str_replace('AssignOp', 'BinaryOp', $ExprType);
            $BinaryExpr = new $ExprType($Expr->var, $Expr->expr);
            $Assign = new Expr\Assign($Expr->var,$BinaryExpr);
            return $this->ExecutionPerExpr($Assign, $Slice);
        }
        elseif($Expr instanceof Expr\BinaryOp) {
            #echo "binaryop l", get_class($Expr->left), "\n";
            $lhstemp = $this->ExecutionPerExpr($Expr->left, $Slice);
            #echo "binaryop r", get_class($Expr->right), "\n";
            //print(printexpr($lhstemp->Value));
            $rhstemp = $this->ExecutionPerExpr($Expr->right, $Slice);
            $ExprType = \get_class($Expr);
            $ExprType = explode("\\", $ExprType)[4];
            $ret = Operation::{$ExprType}($lhstemp, $rhstemp);  
            //print get_class($ret);
            $ret->Value = $Expr;
            $ret->Sources = $lhstemp->Sources;
            foreach($rhstemp->Sources as $s) {
                if(!in_array($s, $ret->Sources))
                    $ret->Sources[] = $s;
            }
           

            switch ($ExprType) {
            case "BooleanAnd":  case "BooleanOr":       case "Equal":
            case "Greater":     case "GreaterOrEqual":  case "Identical":
            case "LogicalAnd":  case "LogicalOr":       case "LogicalXor":
            case "NotEqual":    case "NotIdentical":
            case "Smaller":     case "SmallerOrEqual":  case "SmallerOrEqual":
                $ret->Types = [Variable::Bool_];
                break;
            case "Minus":   case "Mul": case "Plus":
            case "Div":     case "Pow": 
                $ret->Types = [Variable::Double_];
                break;
            case "BitwiseAnd":  case "BitwiseOr":   case "BitwiseXor":
            case "Mod":         case "ShiftLeft":   case "ShiftRight":  
            case "Spaceship":
                // Bitwise operations automatically convert to Int_ type.
                $ret->Types = [Variable::Int_];
                break;
            default:
                $ret->Types = [Variable::Unknown_];
                break;
            }
            return $ret;
        }
        elseif($Expr instanceof BitwiseNot) {
            $ret = $this->ExecutionPerExpr($Expr->expr, $Slice);
            $ret = Variable::BitwiseNot($ret); 
            $ret->Types = [Variable::Int_];
            return $ret;
        }
        elseif($Expr instanceof BooleanNot) {
            $ret = $this->ExecutionPerExpr($Expr->expr, $Slice);
            $ret = Variable::BooleanNot($ret); 
            $ret->Types = [Variable::Bool_];
            return $ret;
        }
        elseif($Expr instanceof Expr\Ternary) {
            //$ = $this->ExecutionPerExpr($Expr->cond, $Slice);
            $if = $this->ExecutionPerExpr($Expr->if, $Slice);
            $else = $this->ExecutionPerExpr($Expr->else, $Slice);
            foreach($else->Types as $t)
                if(!in_array($t, $if->Types))
                    $if->Types[] = $t;
            return $if;
        }
        elseif($Expr instanceof Expr\PostInc) {
            $ret = Evaluate($Expr->var, $Slice);
            $ret = Variable::Plus($ret, 1);
            $ret->Types = [Variable::Double_];
            return $ret;
        }
        elseif($Expr instanceof Expr\PostDec) {
            $ret = Evaluate($Expr->var, $Slice);
            $ret = Variable::Minus($ret, 1);
            $ret->Types = [Variable::Double_];
            return $ret;
        }
        elseif($Expr instanceof Expr\PreInc) {
            $ret = Evaluate($Expr->var, $Slice);
            $ret = Variable::Plus($ret, 1);
            $ret->Types = [Variable::Double_];
            return $ret;
        }
        elseif($Expr instanceof Expr\PreDec) {
            $ret = Evaluate($Expr->var, $Slice);
            $ret = Variable::Minus($ret, 1);
            $ret->Types = [Variable::Double_];
            return $ret;
        }
        elseif($Expr instanceof Expr\UnaryPlus) {
            $ret = $this->ExecutionPerExpr($Expr->expr, $Slice);
            $ret = Variable::Minus(0, $ret);
            $ret->Types = [Variable::Double_];
            return $ret;
        }
        elseif($Expr instanceof Expr\UnaryMinus) {
            $ret = $this->ExecutionPerExpr($Expr->expr, $Slice);
            $ret = Variable::Minus(0, $ret);
            $ret->Types = [Variable::Double_];
            return $ret;
        }
        elseif($Expr instanceof Expr\FuncCall || 
            $Expr instanceof Expr\MethodCall || 
            $Expr instanceof Expr\StaticCall) {
            $Args = [];
            $IsTainted = false;
            $SQLiTaint = false;
            foreach($Expr->args as $arg) {
                //echo get_class($arg);
                $temp = $this->ExecutionPerExpr($arg->value, $Slice);
                $Args[] = $temp;
                if($temp instanceof Variable) {
                    if($temp->IsTainted) {
                        $IsTainted = true;
                    }
                    if($temp->IsTainted && $temp->Sanitized == false) {
                        $SQLiTaint = true;
                    }
                }
            }
            $Function = FetchFunction($Expr, $Slice);

            $CallSite = new Variable(NULL);
            $CallSite->setCallSite($Function[0], $Function[1], $Args);
            if($Function[2] instanceof MyFunction) {
                //if($Function[2]->FuncVisit == false) {
                    $Analyzer = new Execution();
                    $NewSlice = new Slice($Function[2]->ClassName, 
                        $Function[2]->FuncName, $Function[2]->FileName);
                    $pos = 0;
                    CloneObject($Slice->Constraints, $NewSlice->Constraints);
                    foreach($Function[2]->Params as $paramname => $value) {
                        $pos = count($NewSlice->VariableValues);
                        $newparam =  new Variable("argtype" . (string)$pos);
                        $newparam->Value = new PhpParser\Node\Expr\Variable($paramname);
                        if(isset($Args[$pos]))
                            $newparam->IsTainted = $Args[$pos]->IsTainted;
                        $newparam->Sources[] = 'arg' . (string)$pos;
                        //CopyObject($Args[$pos], $newparam);
                        $NewSlice->Variables[$paramname] = $pos;
                        $NewSlice->VariableValues[$pos] = $newparam;
                        $pos ++;
                    }
                    $Function[2]->FuncVisit = true;
                    if($Expr instanceof Expr\MethodCall) {
                        $Object = Evaluate($Expr->var, $Slice);
                        if($Expr->var->name == 'this' or $Expr->var->name == 'self') {
                            $NewSlice->TargetObject = $Slice->TargetObject;
                        }
                        else{
                            $NewSlice->TargetObject = $Object;
                        }
                    }
                    $Analyzer->ExecutionPerNode($Function[2]->Body[0], $Function[2]->Body[1], $NewSlice);
                    GlobalVariable::$Analyzers[$Function[0]][$Function[1]] = $Analyzer;
                    $Analyzer->DumpConstraints($Function[0], $Function[1], array_keys($Function[2]->Params));
                //}
                /*
                elseif(isset(GlobalVariable::$Analyzers[$Function[0]][$Function[1]])) {
                    print("has visited" . $Function[0]);
                    $Analyzer = GlobalVariable::$Analyzers[$Function[0]][$Function[1]];
                }
                */
            }
            return $CallSite;
        }

        elseif($Expr instanceof Expr\Variable){
            $Variable = Evaluate($Expr, $Slice);
            if(!$Variable instanceof Variable) {
                $Variable = new Variable();
                $Variable->Value = $Expr;
            }
            return $Variable;
        }
        elseif($Expr instanceof Expr\ArrayDimFetch) {
            $Variable= Evaluate($Expr, $Slice);
            $exprstring = printexpr($Expr);
            if(!$Variable instanceof Variable) {
                $Variable = new Variable();
                $Variable->Value = $Expr;
            }
            return $Variable;
        }
        elseif($Expr instanceof Expr\PropertyFetch) {
            $Variable = Evaluate($Expr, $Slice);
            if(!$Variable instanceof Variable) {
                $Variable = new Variable();
                $Variable->Value = $Expr;
            }
            $exprstring = printexpr($Expr);
            return $Variable;
        }
        elseif($Expr instanceof Expr\StaticPropertyFetch) {
            $Variable = Evaluate($Expr, $Slice);
            if(!$Variable instanceof Variable) {
                $Variable = new Variable();
                $Variable = new Variable();
                $Variable->Value = $Expr;
            }
            $exprstring = printexpr($Expr);
            return $Variable;
        }
        elseif($Expr instanceof Expr\Array_) {
            $Array = new Variable(Variable::Array_);
            //todo
            foreach($Expr->items as $Item) {
                $Array->AddItem($Item->key, $Item->value);
            }
            return $Array; // todo check return;
        }
        elseif($Expr instanceof Expr\New_) {
            //  TODO needs run __constructor function
            //  TODO default values of arguments
            // 1. check the classname, and properties
            // 2. check whether it has the constructor
            // 3. call the function... the state can be resued. 
            // we can first call it and then direclty copy the object
            $ret = new Variable(Variable::Object_);
            if(is_string($Expr->class)) {
                $ClassName = $Expr->class;
        }
        // first flood
        elseif($Expr->class instanceof Node\Name) {
            $ClassName = implode('', $Expr->class->parts);
        }
        $ret->ClassName = $ClassName;
        // TODO rename the property store to the global address space
        // in case of the conflict
        if(isset(GlobalVariable::$AllClasses[$ClassName])) {
            $class = GlobalVariable::$AllClasses[$ClassName];
            foreach($class->Props as $propname => $prop) {
                $p = new Variable(); // todo clone of not?
                $p->Value = CloneObject($prop); 
                // problematic
                // type inference and propagation
                $ret->Props[$propname] = $p;
            }
        }
        return $ret;
        }
        elseif($Expr instanceof Node\Scalar\Encapsed) {
            $ret = new Variable(Variable::String_);
            $ret->Value = $Expr;
            foreach($Expr->parts as $part) {
                //echo get_class($part), "\n";
                $temp = $this->ExecutionPerExpr($part, $Slice);
                if($temp->IsTainted) {
                    $ret->IsTainted = true;
                }
            }
            return $ret;
        }
        elseif($Expr instanceof Node\Scalar\EncapsedStringPart) {
            $ret = new Variable(Variable::String_);
            $ret->Value = $Expr;
            return $ret;
        }
        elseif($Expr instanceof Node\Scalar) {
            $ret = new Variable();
            $ret->Value = $Expr;
            $ExprType = \get_class($Expr);
            $ExprType = explode("\\", $ExprType)[3];
            switch ($ExprType) {
            case "String_":
                $ret->Types = [Variable::String_];
                break;
            case "LNumber":
                $ret->Types = [Variable::Int_];
                break;
            case "DNumber":
                $ret->Types = [Variable::Double_];
                break;
            default:
                $ret->Types = [Variable::Unknown_];
                break;
            }
            return $ret;
        }
        elseif($Expr instanceof Stmt\Isset_) {
            $TempIsset = new Variable(Variable::Bool_);
            foreach($Expr->vars as $Var) {
                $TempIsset->Vars[] = $this->ExecutionPerExpr($Var, $Slice);
            }
            return $TempIsset;
        }
        elseif($Expr instanceof Stmt\Return_) {
            $ret = $this->ExecutionPerExpr($Expr->expr, $Slice);
            return $ret;
        }
        elseif($Expr instanceof Expr\Cast) {
            $Variable = $this->ExecutionPerExpr($Expr->expr, $Slice);
            $ExprType = explode("\\", \get_class($Expr))[4];
            $ret =  Variable::Cast($Variable, $ExprType);
            //echo $ExprType;
            if($ExprType != "Unset_") {
                $ret->Types = [$ExprType];//Variable::Int_;
            }
            else{
                $ret->Types = [Variable::Unknown_];
            }
            return $ret;
        }
        elseif($Expr instanceof Expr\Print_) {
            $Variable = $this->ExecutionPerExpr($Expr->expr, $Slice);
            if($Variable->IsTainted) {
                $this->XSS = true;
                // print("tainted sink\n");
                $this->XSSDataConstraints[] =  printexpr($Variable->Value) . "\n";
                $str = "";
                foreach($Slice->Constraints as $cond) {
                    $str .= Operation::realprint($cond);
                }
                $this->XSSConstraints[] = $str;
            }
        }

        elseif($Expr instanceof Stmt\Echo_) {
            foreach($Expr->exprs as $e) {
                $Variable = $this->ExecutionPerExpr($e, $Slice);
                if($Variable->IsTainted) {
                    $this->XSS = true;
                    // print("tainted sink\n");
                    $this->XSSDataConstraints[] =  printexpr($Variable->Value) . "\n";
                    $str = "";
                    foreach($Slice->Constraints as $cond) {
                        $str .= Operation::realprint($cond);
                    }
                    $this->XSSConstraints[] = $str;
                }
            }
        }
        if(isset($Expr->expr)) {
            return $this->ExecutionPerExpr($Expr->expr, $Slice);
            //$default->IsTainted = $Variable->IsTainted;
        }
        elseif(isset($Expr->var)){
            return $this->ExecutionPerExpr($Expr->var, $Slice);
            //$default->IsTainted = $Variable->IsTainted;
        }
        $default =  new Variable(Variable::Unknown_);
        $default->Value = CloneObject($Expr);
        return $default;
    }

    /**
     * Store value specificed in Slice->Value to Var
     * @param Expr $Var
     * @param array array of Slice
     * @return array array of Slice with value stored
     * XXX Check whether need to use CloneObject
    */
    public function StoreValueToSlice($Var, $Value, $Slice) {
        //  TODO direct return $Slice for now.
        if($Var instanceof Expr\Variable) {
            $this->StoreToVariable($Var, $Value, $Slice);
        }
        elseif($Var instanceof Expr\ArrayDimFetch) {
            $this->StoreToArrayItem($Var, $Value, $Slice);
        }
        elseif($Var instanceof Expr\PropertyFetch) {
            $this->StoreToObjectProperty($Var, $Value, $Slice);
        }
        elseif($Var instanceof Expr\StaticPropertyFetch) {
            $this->StoreToObjectProperty($Var, $Value, $Slice);
        }
        else {
            //echo get_class($Var);
        }
        return $Slice;
    }

    /**
     * Store to object property
     * @param Expr property location
     * @param Slice
     * @return int 0 for success, -1 for not.
     */
    public function StoreToObjectProperty($Var, $Value, $Slice) {
        if($Var->var->name === 'this' or $Var->var->name === 'self') {
            $Object = $Slice->TargetObject;
        }
        else
            $Object = Evaluate($Var->var, $Slice); // get address of object
        if(is_string($Var->name)) {
            $propname = $var->name;
        }
        elseif($Var->name instanceof Node\Identifier) {
            $propname = $Var->name->name;
        }
        return 0;
        
        if(!isset($Object->Props[$propname])) {
            $pos = count($Object->Props);
            $Object->Props[$propname] = new Variable();
        }
        else{
            $Object->Props[$propname] = $Value;
        }
        return 0;
    }

    /**
     * Store variable value to slice.
     * @param string|Expr varname to locate
     * @param Slice
     * @return int 0 for success, -1 for not.
     */
    public function StoreToVariable($Var, $Value, $Slice) {
        $pos = count($Slice->VariableValues);
        $Slice->Variables[$Var->name] = $pos;
        $Slice->VariableValues[$pos] = $Value;
    }

    /**
     * Store value to array item, array can be multiple dimision
     * @param Expr Location of arrayitem
     * @param Slice 
     * @return int 0 for success, -1 for not.
     */
    public function StoreToArrayItem($Var, $Value, $Slice) {
        $Array = Evaluate($Var->var, $Slice);
        if(!$Array or !$Array instanceof Variable)
            return;

        //echo get_class($Var->var);
        if(is_null($Var->dim)) {
            $Array->AddItem(NULL, $Value);
        }
        elseif($Var->dim instanceof Node\Scalar\String_) {
            $Array->AddItem($Var->dim, $Value);
        }
        elseif($Var->dim instanceof Node\Scalar\LNumber) {
            $Array->AddItem($Var->dim, $Value);
        }
        elseif($Var->dim instanceof Node\Scalar\DNumber) {
            $Array->AddItem($Var->dim, $Value);
        }
        else{
            $dim = Evaluate($Var->dim, $Slice);
            if(is_null($dim)) {
                $Array->AddItem(NULL, $Value);
            }
            elseif($dim instanceof Node\Scalar\String_) {
                $Array->AddItem($dim, $Value);
            }
            elseif($dim instanceof Node\Scalar\LNumber) {
                $Array->AddItem($dim, $Value);
            }
            elseif($dim instanceof Node\Scalar\DNumber) {
                $Array->AddItem($dim, $Value);
            }
            else{
                if($Array) //todo
                $Array->AddItem($dim, $Value); 
            }
        }
        return 0;
    }
}

/**
 *
 * @return return the object of method
 * if unfound, return the function name, it might 
 * be built-in functions.
 */
function FetchFunction($Expr, $Slice) {

    if($Expr->name instanceof Node\Identifier) {
        $FuncName = $Expr->name->name;
    }
    elseif(is_string($Expr->name)) {
        $FuncName = $Expr->name;
    }
    elseif($Expr->name instanceof Node\Name) {
        $FuncName = implode("", $Expr->name->parts);
    }
    else{
        echo "FetchFunction FuncName ", get_class($Expr->class), "\n";
        return [NULL, NULL, NULL];
    }

    if ($Expr instanceof Expr\StaticCall) {
        if(is_string($Expr->class)) {
            $ClassName = $Expr->class;
        }
        elseif($Expr->class instanceof Node\Name) {
            $ClassName = implode('', $Expr->class->parts);
        }
        else{
            echo "FetchFunction StaticCall class ", get_class($Expr->class), "\n";
            return [NULL, $FuncName, NULL];
        }
        if($ClassName == 'self' || $ClassName == 'this') {
            $ClassName = $Slice->ClassName;
        }
    }
    elseif($Expr instanceof Expr\FuncCall){
        $ClassName = MAIN_CLASS;
    }
    else{ // methodcall
        // TODO this->a()
        //  TODO... call a->b();
        $Object = Evaluate($Expr->var, $Slice);
        $ClassName = $Object->ClassName;
    }
    $temp = FindFunction($ClassName, $FuncName);
    return [$ClassName, $FuncName, $temp];
}

function FindFunction($ClassName, $FuncName) {
    //global $GlobalVariable;
    if($ClassName != "") {
        if(isset(GlobalVariable::$AllClasses[$ClassName]) && 
            isset(GlobalVariable::$AllClasses[$ClassName]->ClassMethods[$FuncName])) {
            return GlobalVariable::$AllClasses[$ClassName]->ClassMethods[$FuncName];
        }
        elseif($ClassName == MAIN_CLASS) {
            return -1; // built-in functions we assume
        }
        else{
            return NULL;
        }
    }
    else{
        return NULL;
    }
}

/**
 * Evaluate a $Var based on Slice
 * @param Expr|Variable 
 * @param Slice $Slice
 * @return return the obejct from it without copy
 */
function Evaluate($Var, $Slice, $arrkey = false) {
    if($Var instanceof Expr\Variable) {
        if(is_string($Var->name)) {
            //  TODO $this
            if($Var->name == 'this') {
                return $Slice->TargetObject;
            }
            elseif(!isset($Slice->Variables[$Var->name])) {
                $pos = count($Slice->VariableValues);
                $Slice->Variables[$Var->name] = $pos;
                $tmp =  new Variable();
                $tmp->Value = $Var;
                if($arrkey){
                    $Slice->VariableValues[$pos] = $tmp;
                }
                else {

                    $Slice->VariableValues[$pos] = $tmp; 
                }
            }
            else{
                $pos = $Slice->Variables[$Var->name];
            }
            return $Slice->VariableValues[$pos];
        }
        return NULL;
    }
    elseif($Var instanceof Expr\ArrayDimFetch) {
        //  TODO: This part is dangerous
        $Array = Evaluate($Var->var, $Slice, $arrkey=true);
        //$DimAddress = Evaluate($Var->dim);
        $FromGlobal = false;
        if($Var->var instanceof Expr\Variable)
            if(is_string($Var->var->name)) {
                $arrayname = $Var->var->name;
                if(strcasecmp($arrayname,"_GET") == 0 or strcasecmp($arrayname, "_POST") == 0 or strcasecmp($arrayname, "_FILES") == 0 or
                     strcasecmp($arrayname, "_COOKIE")== 0 or  strcasecmp($arrayname, "_SERVER") == 0){
                    $FromGlobal = true;
                }
            }
            else {
                return NULL;
            }
            if($Array instanceof Variable) {
                $Array->IsTainted = $FromGlobal;
                $item = $Array->FindKey($Var->dim); // check this function todo
                if($item === NULL) {
                    // Rethink about it. avoid to use global variables because of the conflict name
                    $key = new Node\Scalar\LNumber(count($Array->Items));
                    $item = new ArrayItem($key, $Var);// append
                    $Array->Items[count($Array->Items)] = $item;
                    //print("global111");
                }
                $item->IsTainted = $FromGlobal;
                $item->Types[] = ($FromGlobal) ? Variable::String_ : Variable::Unknown_;
                return $item;
            }
        return NULL;
    }
    elseif($Var instanceof Expr\PropertyFetch) {
        if($Var->var->name === 'this' or $Var->var->name === 'self') {
            $Object = $Slice->TargetObject;
        }
        else
            $Object = Evaluate($Var->var, $Slice); // get address of object
        if(is_string($Var->name)) {
            $propname = $var->name;
        }
        elseif($Var->name instanceof Node\Identifier) {
            $propname = $Var->name->name;
        }
        if(!isset($Object->Props[$propname])) {
            $pos = count($Object->Props);
            $Object->Props[$propname] = new Variable();
        }
        CopyObject($Object, $Object->Props[$propname]);
        return $Object->Props[$propname];
    }
    elseif($Var instanceof Expr\StaticPropertyFetch) {
        //  TODO imitate above

        if(is_string($Var->class)) {
            $ClassName = $Var->class;
        }
        elseif($Var->class instanceof Node\Name) {
            $ClassName = implode('', $VAr->class->parts);
        }
        else{
            return defaultvar($Var);
        }
        if($Var->name instanceof Node\VarLikeIdentifier) {
            $PropertyName = $Var->name->name;
        }
        elseif(is_string($Var->name)) {
            $PropertyName = $Var->name;
        }
        else{
            return defaultvar($Var, $t);
        }

        if($ClassName == "static" or $ClassName == "self") {
            $Object = $Slice->StaticClasses[$Slice->ClassName];
        }
        else{
            $Object = $Slice->StaticClasses[$ClassName];
        }
        if(!isset($Object->Props[$ObjectName])) {
            $pos = count($Object->VariableValues);
            $Object->Porps[$ObjectName] = new Variable();
        }
        if($Object == NULL)
            $Object = new Variable(Variable::Object_);
        return $Object->Props[$PropertyName];
    }
    else{
        //echo "In Evaluate: Unexpected Var ", get_class($Var), "\n";
    }
    return NULL;
}

function printexpr($expr) {
    $printer = new PhpParser\PrettyPrinter\Standard;
    if($expr instanceof PhpParser\Node\Expr)
        return $printer->prettyPrintExpr($expr);
    return "null";
}

function checktype($lhs, $rhs){
    if((in_array(Variable::String_, $lhs->Types) and
        in_array(Variable::String_, $rhs->Types)) or
        (in_array(Variable::String_, $lhs->Types) and
        in_array(Variable::Double_, $rhs->Types)) or
        (in_array(Variable::String_, $lhs->Types) and
        in_array(Variable::Int_, $rhs->Types)) or
        (in_array(Variable::Double_, $lhs->Types) and
        in_array(Variable::String_, $rhs->Types)) or
        (in_array(Variable::Int_, $lhs->Types) and
        in_array(Variable::String_, $rhs->Types)))
        return True;
    return false;
}

function checktypearr($lhs, $rhs){
    if((in_array(Variable::String_, $lhs) and
        in_array(Variable::String_, $rhs)) or
        (in_array(Variable::String_, $lhs) and
        in_array(Variable::Double_, $rhs)) or
        (in_array(Variable::String_, $lhs) and
        in_array(Variable::Int_, $rhs)) or
        (in_array(Variable::Double_, $lhs) and
        in_array(Variable::String_, $rhs)) or
        (in_array(Variable::Int_, $lhs) and
        in_array(Variable::String_, $rhs)))
        return True;
    return false;

}


function defaultvar($Expr, $taint = false) {
    $ret = new Variable();
    $exprstring = printexpr($Expr);
    if($taint)
        $ret->IsTainted = true;
    $ret->Value = $Expr;
    return $ret;
}
