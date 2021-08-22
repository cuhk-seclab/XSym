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

    public $FromDatabase = false;
    public $FromEncrypt = false;
    public $DataAccess = false;
    public $FuncFromDatabase = false;
    public $FuncFromEncrypt = false;
    public $FuncDataAccess = false;

    public $ReturnFromDatabase = false;
    public $ReturnTainted = false;
    public $ReturnFromEncrypt = false;
    public $ReturnDataAccess = false;


    public $LooseComps = []; // record all loose comparisons and clone


    public $LooseComp1 = false;
    public $LooseComp2 = false;
    public $SQLi = false;
    public $XSS = false;
    public $ACC = false;
    public $Auth = false;

    public function clean(){
        $this->LooseComp1 = false;
        $this->LooseComp2 = false;
        $this->SQLi = false;
        $this->XSS = false;
        $this->DataAccess = false;
        $this->Auth = false;
    }

    public function ExecutionPerNode($Start, $End, $Slice) {
        $CurrentNode = $Start;
        $reachend = false;
        while($CurrentNode != NULL and $CurrentNode != $End) {
            if($CurrentNode->Type == IfStmt) {
                $size = count($CurrentNode->Conditions);
                $slice_array = [];
                for($i = 0; $i < $size; $i ++) {
                    //  TODO copy slice
                    $SliceCopy = CloneObject($Slice);
                    $this->clean();
                    $Cond = $this->ExecutionPerExpr($CurrentNode->Conditions[$i], $SliceCopy);
                    if($CurrentNode->Conditions[$i] instanceof PhpParser\Node)
                        $line = $CurrentNode->Conditions[$i]->getLine();
                    else $line = -1;
                    $SliceCopy->Constraints[] = $Cond; // Add constraints!!
                    // Every slice maintains a constraint set.
                    //

                    $slice_array[] = $SliceCopy;
                    $Body = $CurrentNode->Bodies[$i];
                    $this->ExecutionPerNode($Body[0], $Body[1], $SliceCopy);
                }
                // $this->accumulate($Slice, $slice_array);
            }
            else if($CurrentNode->Type == Stmt) {
                $this->clean();
                $this->ExecutionPerExpr($CurrentNode->Stmt, $Slice);
            }
            $CurrentNode = $CurrentNode->Child;
        }
        return true;
    }

    public function accumulate($Slice, $slices) {
        foreach($slices as $s){
            foreach($s->Variables as $name => $id){
                if(!array_key_exists($name, $Slice->Variables)) {
                    $pid = count($Slice->Variables);
                    $Slice->Variables[$name] = $pid;
                    $Slice->VariableValues[$pid] = $s->VariableValues[$id];
                }
                else{
                    $pid = $Slice->Variables[$name];
                }
                if($s->VariableValues[$id]->IsTainted)
                    $Slice->VariableValues[$pid]->IsTainted = true;
                if($s->VariableValues[$id]->FromDatabase)
                    $Slice->VariableValues[$pid]->FromDatabase = true;
                if($s->VariableValues[$id]->IsTainted)
                    $Slice->VariableValues[$pid]->FromEncrypt = true;

                // propagate all types
                foreach($s->VariableValues[$id]->Types as $type) {
                    if(!in_array($type, $Slice->VariableValues[$pid]->Types))
                        $Slice->VariableValues[$pid]->Types[] = $type;
                }
                /*
                if($s->VariableValues[$id]->IsTainted)
                    $Slice->VariableValues[$pid]-> = true;
                 */
            }
        }
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
        global $BothTaintInfo, $OneTaintInfo, $FromEncryptInfo, $FromDatabaseInfo;
        global $XSSInfo, $SQLiInfo, $DataAccessInfo, $LooseCompInfo;

        if($Expr == NULL)
            return new Variable(); // return null
        //echo get_class($Expr), "\n";
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
            //echo "binaryop l", get_class($Expr->left), "\n";
            $lhstemp = $this->ExecutionPerExpr($Expr->left, $Slice);
            //echo "binaryop r", get_class($Expr->right), "\n";
            $rhstemp = $this->ExecutionPerExpr($Expr->right, $Slice);
            $ExprType = \get_class($Expr);
            $ExprType = explode("\\", $ExprType)[4];
            $ret = Variable::{$ExprType}($lhstemp, $rhstemp);  
            $ret->Value = $Expr;
            $ret->Sources = $lhstemp->Sources;
            foreach($rhstemp->Sources as $s) {
                if(!in_array($s, $ret->Sources))
                    $ret->Sources[] = $s;
            }
            if($lhstemp->FromEncrypt || $rhstemp->FromEncrypt) {
                $ret->FromEncrypt = true;
            }
            if($lhstemp->FromDatabase || $rhstemp->FromDatabase) {
                //echo "so here from db\n";
                $ret->FromDatabase = true;
            }
            if($lhstemp->IsTainted || $rhstemp->IsTainted) {
                //echo "so here one taint\n";
                $ret->IsTainted = true;
            }

            if($lhstemp->LooseComp || $rhstemp->LooseComp) {
                $ret->LooseComp = true; // actually this shall only for logical op
            }
            if($lhstemp->DataAccess || $rhstemp->DataAccess) {
                $ret->DataAccess = true;
                $this->FuncDataAccess = true;
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
            if(isloosecomp($Expr)){
                $this->LooseComps[] = CloneObject($ret);
                if($lhstemp->IsTainted && $rhstemp->IsTainted) {

                    if(!$Expr->right instanceof Node\Scalar && !$Expr->left instanceof Node\Scalar) {
                        $strtemplate = "BothTaint\t$Slice->ClassName\t$Slice->FuncName\t$Slice->FileName\t".$Expr->getLine() . "\n";
                        if(!in_array($strtemplate, $BothTaintInfo))
                            $BothTaintInfo[] = $strtemplate;
                        if(checktype($lhstemp, $rhstemp)) {
                            $this->LooseComp2 = true;
                        }
                        if($Expr instanceof Expr\BinaryOp\Equal or $Expr instanceof Expr\BinaryOp\NotEqual)
                            if(($lhstemp->FromEncrypt or $rhstemp->FromEncrypt) or 
                                ($lhstemp->FromDatabase or $rhstemp->FromDatabase)) {
                                $this->Auth = True;
                            }
                    }
                }
                elseif($lhstemp->IsTainted || $rhstemp->IsTainted) {

                    if(!$Expr->right instanceof Node\Scalar && !$Expr->left instanceof Node\Scalar) {
                        $strtemplate =  "OneTaint\t$Slice->ClassName\t$Slice->FuncName\t$Slice->FileName\t".$Expr->getLine() . "\n";
                        if(!in_array($strtemplate, $OneTaintInfo))
                            $OneTaintInfo[] = $strtemplate;

                        if(checktype($lhstemp, $rhstemp)) {
                            $this->LooseComp1 = true;
                        }
                        if($Expr instanceof Expr\BinaryOp\Equal or $Expr instanceof Expr\BinaryOp\NotEqual)
                            if(($lhstemp->FromEncrypt or $rhstemp->FromEncrypt) or 
                                ($lhstemp->FromDatabase or $rhstemp->FromDatabase)) {
                                $this->Auth = True;
                            }
                    }
                }
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
                if($Function[2]->FuncVisit == false) {
                    $Analyzer = new Execution();
                    $NewSlice = new Slice($Function[2]->ClassName, 
                        $Function[2]->FuncName, $Function[2]->FileName);
                    $pos = 0;
                    foreach($Function[2]->Params as $paramname => $value) {
                        $pos = count($NewSlice->VariableValues);
                        $newparam =  new Variable("argtype" . (string)$pos);
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
                }
                elseif(isset(GlobalVariable::$Analyzers[$Function[0]][$Function[1]])) {
                    $Analyzer = GlobalVariable::$Analyzers[$Function[0]][$Function[1]];
                }
            }
            if(isset($Analyzer)) {
                if($Analyzer->FuncFromDatabase) {
                    $this->FromDatabase = true;
                    $this->FuncFromDatabase = true;
                }
                if($Analyzer->FuncFromEncrypt) {
                    $this->FromEncrypt = true;
                    $this->FuncFromEncrypt = true;
                }
                if($Analyzer->FuncDataAccess) {
                    $this->DataAccess = true;
                    $this->FuncDataAccess = true;
                }
                foreach($Analyzer->ReturnTypes as $t) {
                    if(substr($t, 0, 3) == 'arg'){
                        $pos = (int)substr($t, 7, strlen($t) - 7);
                        if(isset($Args[$pos]) and $Args[$pos] instanceof Variable)
                            foreach($Args[$pos]->Types as $tt) {
                                if(!in_array($tt, $CallSite->Types))
                                    $CallSite->Types[] = $tt;
                            }
                    }
                    elseif(!in_array($t, $CallSite->Types))
                        $CallSite->Types[] = $t;
                }
                if($Analyzer->ReturnTainted || $IsTainted)
                    $CallSite->IsTainted = true;
                if($Analyzer->ReturnFromEncrypt)
                    $CallSite->FromEncrypt = true;
                if($Analyzer->ReturnFromDatabase)
                    $CallSite->FromDatabase = true;
                if($Analyzer->ReturnDataAccess)
                    $CallSite->DataAccess = true;

                foreach($Analyzer->LooseComps as $comp) {
                    $lhstemp = $comp->Left;
                    $rhstemp = $comp->Right;
                    $compexpr = $comp->Value;
                    foreach($lhstemp->Sources as $s) {
                        if(substr($s, 0, 3) == 'arg') {
                            $pos = (int)substr($s, 3, strlen($s) - 3);
                            if(isset($Args[$pos]) && $Args[$pos] instanceof Variable){
                                if($Args[$pos]->IsTainted)
                                    $lhstemp->IsTainted = true;
                                if($Args[$pos]->FromDatabase)
                                    $lhstemp->FromDatabase = true;
                                if($Args[$pos]->FromEncrypt)
                                    $lhstemp->FromEncrypt = true;
                            }
                        }
                    }

                    foreach($rhstemp->Sources as $s) {
                        if(substr($s, 0, 3) == 'arg') {
                            $pos = (int)substr($s, 3, strlen($s) - 3);
                            if(isset($Args[$pos]) && $Args[$pos] instanceof Variable){
                                if($Args[$pos]->IsTainted)
                                    $rhstemp->IsTainted = true;
                                if($Args[$pos]->FromDatabase)
                                    $rhstemp->FromDatabase = true;
                                if($Args[$pos]->FromEncrypt)
                                    $rhstemp->FromEncrypt = true;
                            }
                        }
                    }

                    $lhstypescopy = [];//$lhstemp->Types;
                    foreach($lhstemp->Types as $t) {
                        if(substr($t, 0, 3) == 'arg'){
                            $pos = (int)substr($t, 7, strlen($t) - 7);
                            if(isset($Args[$pos]) and $Args[$pos] instanceof Variable)
                                foreach($Args[$pos]->Types as $tt) {
                                    if(!in_array($tt, $lhstypescopy))
                                        $lhstypescopy[] = $tt;
                                }
                        }
                        elseif(!in_array($t, $lhstypescopy))
                            $lhstypescopy[] = $t;
                    }
                    $rhstypescopy = [];//$lhstemp->Types;
                    foreach($rhstemp->Types as $t) {
                        if(substr($t, 0, 3) == 'arg'){
                            $pos = (int)substr($t, 7, strlen($t) - 7);
                            if(isset($Args[$pos]) and $Args[$pos] instanceof Variable)
                                foreach($Args[$pos]->Types as $tt) {
                                    if(!in_array($tt, $rhstypescopy))
                                        $rhstypescopy[] = $tt;
                                }
                        }
                        elseif(!in_array($t, $rhstypescopy))
                            $rhstypescopy[] = $t;
                    }

                    if($lhstemp->IsTainted && $rhstemp->IsTainted ) {
                        $strtemplate = "BothTaint\t$Slice->ClassName\t$Slice->FuncName\t$Slice->FileName\t".$compexpr->getLine() . "\n";
                        if(!in_array($strtemplate, $BothTaintInfo))
                            $BothTaintInfo[] = $strtemplate;

                        if(!$compexpr->right instanceof Node\Scalar && !$compexpr->left instanceof Node\Scalar) {
                            if(checktypearr($lhstypescopy, $rhstypescopy)) {

                                $this->LooseComp2 = true;
                            }
                            if($compexpr instanceof Expr\BinaryOp\Equal or $compexpr instanceof Expr\BinaryOp\NotEqual)
                                if(($lhstemp->FromEncrypt or $rhstemp->FromEncrypt) or 
                                    ($lhstemp->FromDatabase or $rhstemp->FromDatabase)) {
                                    $this->Auth = True;
                                }
                        }
                    }
                    elseif($lhstemp->IsTainted || $rhstemp->IsTainted ) {
                        $strtemplate =  "OneTaint\t$Slice->ClassName\t$Slice->FuncName\t$Slice->FileName\t".$Expr->getLine() . "\n";
                        //echo $strtemplate, "\n";
                        if(!in_array($strtemplate, $OneTaintInfo))
                            $OneTaintInfo[] = $strtemplate;

                        if(!$compexpr->right instanceof Node\Scalar && !$compexpr->left instanceof Node\Scalar) {
                            if(checktypearr($lhstypescopy, $rhstypescopy)) {
                                $this->LooseComp1 = true;
                                //echo "here@!!!\n";
                            }
                            if($compexpr instanceof Expr\BinaryOp\Equal or $compexpr instanceof Expr\BinaryOp\NotEqual)
                                if(($lhstemp->FromEncrypt or $rhstemp->FromEncrypt) or 
                                    ($lhstemp->FromDatabase or $rhstemp->FromDatabase)) {
                                    $this->Auth = True;
                                }
                        }
                    }
                }
                return $CallSite;
            }
            $lowfuncname = strtolower($Function[1]);
            if(strpos($lowfuncname, 'hash') !== false 
                or strpos($lowfuncname, 'decode') !== false
                or strpos($lowfuncname, 'encode') !== false 
                or strpos($lowfuncname, 'crypt') !== false) {
                if(!$lowfuncname === 'password_hash') {
                    $CallSite->FromEncrypt = true;
                    $this->FromEncrypt = true;
                    $this->FuncFromEncrypt = true;
                }
            }
            //echo $lowfuncname;
            if(strpos($lowfuncname, 'mysql_affected_rows') !== false 
                or strpos($lowfuncname, 'msql_create_db') !== false
                or strpos($lowfuncname, 'mysql_db_query') !== false 
                or strpos($lowfuncname, 'msql_drop_db') !== false
                or strpos($lowfuncname, 'mysql_fetch_array') !== false 
                or strpos($lowfuncname, 'mysql_fetch_assoc') !== false
                or strpos($lowfuncname, 'mysql_fetch_row') !== false 
                or strpos($lowfuncname, 'mysql_query') !== false
                or strpos($lowfuncname, 'mysql_result') !== false 
                or strpos($lowfuncname, 'mysql_fetch_object')!== false
                or strpos($lowfuncname, 'mysqli_query') !== false
                or strpos($lowfuncname, 'mysqli_result') !== false
                or strpos($lowfuncname, 'mysqli_fetch_object') !== false
                or strpos($lowfuncname, 'mysqli_fetch_row') !== false
                or strpos($lowfuncname, 'mysqli_fetch_field') !== false
                or strpos($lowfuncname, 'mysqli_fetch_fields') !== false
                or strpos($lowfuncname, 'mysqli_fetch_accoc') !== false
                or strpos($lowfuncname, 'mysqli_fetch_array') !== false
                or strpos($lowfuncname, 'mysqli_fetch_all') !== false
                or $lowfuncname === 'query'
            ){
                $CallSite->FromDatabase = true;
                $this->FromDatabase = true;
                $this->FuncFromDatabase = true;
                if($SQLiTaint) {
                    $this->SQLi = true;
                    $strtemplate = "SQLi\t$Slice->ClassName\t$Slice->FuncName\t$lowfuncname\t$Slice->FileName\t".$Expr->getLine() . "\n";
                    if(!in_array($strtemplate, $SQLiInfo))
                        $SQLiInfo[] = $strtemplate;
                }
                $CallSite->DataAccess = true;
                $this->DataAccess = true;
                $this->FuncDataAccess = true;
                $strtemplate = "DataAccess\t$Slice->ClassName\t$Slice->FuncName\t$lowfuncname\t$Slice->FileName\t".$Expr->getLine()."\n";
                //echo $strtemplate, "\n";
                if(!in_array($strtemplate, $DataAccessInfo))
                    $DataAccessInfo[] = $strtemplate;
            }
            if(strpos($lowfuncname, 'fopen') !== false 
                or strpos($lowfuncname, 'fread') !== false
                or strpos($lowfuncname, 'fwrite') !== false
                or strpos($lowfuncname, 'readfile') !== false
                or strpos($lowfuncname, 'feof') !== false
                or strpos($lowfuncname, 'fileupload') !== false
            ){
                $CallSite->DataAccess = true;
                $this->DataAccess = true;
                $this->FuncDataAccess = true;
                $strtemplate = "DataAccess\t$Slice->ClassName\t$Slice->FuncName\t$lowfuncname\t$Slice->FileName\t".$Expr->getLine(). "\n";
                //echo $strtemplate, "\n";
                if(!in_array($strtemplate, $DataAccessInfo))
                    $DataAccessInfo[] = $strtemplate;
            }
            if(strpos($lowfuncname, 'mysql_real_escape_string') !== false 
                or strpos($lowfuncname, 'htmlentities') !== false 
                or strpos($lowfuncname, 'htmlspecialchars') !== false  
                or strpos($lowfuncname, 'strap_tags') !== false 
                or strpos($lowfuncname, 'sanitize') !== false
            ) {
                $CallSite->Sanitized = true;
            }

            if(strpos($lowfuncname, 'count') !== false 
                or strpos($lowfuncname, 'strlen') !== false
                or strpos($lowfuncname, 'strpos') !== false
                or strpos($lowfuncname, 'preg_match_all') !== false
                or strpos($lowfuncname, 'intval') !== false
                or strpos($lowfuncname, 'ord') !== false
                or strpos($lowfuncname, 'floor') !== false
                or strpos($lowfuncname, 'ceil') !== false
            ) {
                $FlagType = [Variable::Int_];
            }
            elseif(strpos($lowfuncname, 'is_array') !== false
                or strpos($lowfuncname, 'array_key_exists') !== false
                or strpos($lowfuncname, 'in_array') !== false
                or strpos($lowfuncname, 'in_object') !== false
                or strpos($lowfuncname, 'in_numeric') !== false
                or strpos($lowfuncname, 'in_null') !== false
                or strpos($lowfuncname, 'in_int') !== false
                or strpos($lowfuncname, 'in_file') !== false
                or strpos($lowfuncname, 'defined') !== false
                or strpos($lowfuncname, 'is_dir') !== false
                or strpos($lowfuncname, 'preg_match') !== false
                or strpos($lowfuncname, 'file_exists') !== false
                or strpos($lowfuncname, 'is_string') !== false
                or strpos($lowfuncname, 'is_callable') !== false
                or strpos($lowfuncname, 'func_exists') !== false 
                or strpos($lowfuncname, 'class_exists') !== false 
                or strpos($lowfuncname, 'method_exists') !== false
            ) {
                $FlagType = [Variable::Bool_];
            }
            elseif(strpos($lowfuncname, 'substr') !== false 
                or strpos($lowfuncname, 'dirname') !== false 
                or strpos($lowfuncname, 'sub_replace') !== false
                or strpos($lowfuncname, 'preg_replace') !== false
                or strpos($lowfuncname, 'get_class') !== false 
                or strpos($lowfuncname, 'json_encode') !== false 
                or strpos($lowfuncname, 'trim') !== false
                or strpos($lowfuncname, 'ltrim') !== false
                or strpos($lowfuncname, 'strtolower') !== false
                or strpos($lowfuncname, 'strtoupper') !== false
                or strpos($lowfuncname, 'md5') !== false
                or strpos($lowfuncname, 'hash') !== false
                or strpos($lowfuncname, 'sha1') !== false
                or strpos($lowfuncname, 'sha256') !== false
                or strpos($lowfuncname, 'basename') !== false
                or strpos($lowfuncname, 'chr') !== false
                or strpos($lowfuncname, 'htmlspecialchars') !== false
                or strpos($lowfuncname, 'htmlentities') !== false
                or strpos($lowfuncname, 'mysql_affected_rows') !== false 
                or strpos($lowfuncname, 'msql_create_db') !== false
                or strpos($lowfuncname, 'mysql_db_query') !== false 
                or strpos($lowfuncname, 'msql_drop_db') !== false
                or strpos($lowfuncname, 'mysql_fetch_array') !== false 
                or strpos($lowfuncname, 'mysql_fetch_assoc') !== false
                or strpos($lowfuncname, 'mysql_fetch_row') !== false 
                or strpos($lowfuncname, 'mysql_query') !== false
                or strpos($lowfuncname, 'mysql_result') !== false 
                or strpos($lowfuncname, 'mysql_fetch_object')!== false
                or strpos($lowfuncname, 'mysqli_query') !== false
                or strpos($lowfuncname, 'mysqli_result') !== false
                or strpos($lowfuncname, 'mysqli_fetch_object') !== false
                or strpos($lowfuncname, 'mysqli_fetch_row') !== false
                or strpos($lowfuncname, 'mysqli_fetch_field') !== false
                or strpos($lowfuncname, 'mysqli_fetch_fields') !== false
                or strpos($lowfuncname, 'mysqli_fetch_accoc') !== false
                or strpos($lowfuncname, 'mysqli_fetch_array') !== false
                or strpos($lowfuncname, 'mysqli_fetch_all') !== false
                or $lowfuncname === 'query'
            ) {
                $FlagType = [Variable::String_];
            }
            elseif(strpos($lowfuncname, 'explode') !== false 
                or strpos($lowfuncname, 'array_merge') !== false
                or strpos($lowfuncname, 'serisize') !== false
                or strpos($lowfuncname, 'strtr') !== false
            ) {
                $FlagType = [Variable::Array_];
            }
            elseif(strpos($lowfuncname, 'round') !== false 
                or strpos($lowfuncname, 'mktime') !== false
            ){
                $FlagType = [Variable::Double_];
            }
            if(isset($FlagType)) {
                $CallSite->Types = $FlagType;
            }
            else
                $CallSite->Types = [Variable::Unknown_];
            $exprstring = printexpr($Expr);
            if(encrypt($exprstring)) {
                $CallSite->FromEncrypt = true;
                $CallSite->IsTainted = true;
            }
            if($IsTainted)
                $CallSite->IsTainted = true;
            return $CallSite;
        }

        elseif($Expr instanceof Expr\Variable){
            $Variable = Evaluate($Expr, $Slice);
            if(!$Variable instanceof Variable) {
                $Variable = new Variable();
                $Variable->Value = $Expr;
            }
            $exprstring = printexpr($Expr);
            if(encrypt($exprstring)) {
                $Variable->FromEncrypt = true;
                $Variable->IsTainted = true;
            }
            return $Variable;
        }
        elseif($Expr instanceof Expr\ArrayDimFetch) {
            $Variable= Evaluate($Expr, $Slice);
            if(!$Variable instanceof Variable) {
                $Variable = new Variable();
                $Variable->Value = $Expr;
            }
            $exprstring = printexpr($Expr);
            if(encrypt($exprstring)) {
                $Variable->FromEncrypt = true;
                $Variable->IsTainted = true;
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
            if(encrypt($exprstring)) {
                $Variable->FromEncrypt = true;
                $Variable->IsTainted = true;
            }
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
            if(encrypt($exprstring)) {
                $Variable->FromEncrypt = true;
                $Variable->IsTainted = true;
            }
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
                if($temp->FromEncrypt) {
                    $ret->FromEncrypt = true;
                    $ret->IsTainted = true;
                }
                if($temp->FromDatabase) {
                    $ret->FromDatabase = true;
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
            //echo "coming return_\n";
            foreach($ret->Types as $t)
                if(!in_array($t, $this->ReturnTypes))
                    $this->ReturnTypes[] = $t;
            if($ret->IsTainted) {
                $this->ReturnTainted = true;
                echo" return tainted\n";
            }
            if($ret->FromEncrypt) {
                $this->ReturnFromEncrypt = true;
            }
            if($ret->FromDatabase) {
                $this->ReturnFromDatabase = true;
                //echo "returnfromdatabase\n";
            }
            if($ret->DataAccess)
                $this->ReturnDataAccess = true;
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
        elseif($Expr instanceof Stmt\Print_) {
            $Variable = $this->ExecutionPerExpr($Expr->expr, $Slice);
            if($Variable->IsTainted) {
                $this->XSS = true;
            }
        }

        elseif($Expr instanceof Stmt\Echo_) {
            foreach($Expr->exprs as $e) {
                $Variable = $this->ExecutionPerExpr($e, $Slice);
                if($Variable->IsTainted) {
                    $this->XSS = true;
                    $strtemplate = "XSS\t$Slice->ClassName\t$Slice->FuncName\t$Slice->FileName\t".$Expr->getLine(). "\n";
                    //echo $strtemplate, "\n";
                    if(!in_array($strtemplate, $XSSInfo))
                        $XSSInfo[] = $strtemplate;
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
        $FromEncrypt = false;
        if(encrypt(printexpr($Var)))
            $FromEncrypt = true;
        if(is_string($Var->name)) {
            //  TODO $this
            if($Var->name == 'this') {
                return $Slice->TargetObject;
            }
            elseif(!isset($Slice->Variables[$Var->name])) {
                $pos = count($Slice->VariableValues);
                $Slice->Variables[$Var->name] = $pos;
                if($arrkey){
                    $Slice->VariableValues[$pos] = new Variable();
                }
                else {
                    $Slice->VariableValues[$pos] = new Variable();
                }
            }
            else{
                $pos = $Slice->Variables[$Var->name];
            }
            if($FromEncrypt)
                $Slice->VariableValues[$pos]->FromEncrypt = $FromEncrypt;
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
                $item = new ArrayItem($key, NULL);// append
                $Array->Items[count($Array->Items)] = $item;
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
    return $printer->prettyPrintExpr($expr);
}

function encrypt($exprstring){
    if(strpos($exprstring, 'passwd') !== false or
        strpos($exprstring, 'password') !== false or
        strpos($exprstring, 'uname') !== false or
        strpos($exprstring, 'username') !== false or
        strpos($exprstring, 'login') !== false or
        strpos($exprstring, "encrypt")!== false or
        strpos($exprstring, "authen")!== false  or
        strpos($exprstring, "verify")!== false 
    ) {
        return true;
    }
    return false;
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
function isloosecomp($Expr){
    if($Expr instanceof Expr\BinaryOp\Equal or $Expr instanceof Expr\BinaryOp\NotEqual or
        $Expr instanceof Expr\BinaryOp\GreaterOrEqual or $Expr instanceof Expr\BinaryOp\Greater or
        $Expr instanceof Expr\BinaryOp\SmallerOrEqual or $Expr instanceof Expr\BinaryOp\Smaller)
        return True;
    return False;
}

function defaultvar($Expr, $taint = false) {
    $ret = new Variable();
    $exprstring = printexpr($Expr);
    if(encrypt($exprstring)) {
        $ret->Types = [Variable::String_];
        $ret->IsTainted = true;
    }
    if($taint)
        $ret->IsTainted = true;
    $ret->Value = $Expr;
    return $ret;
}
