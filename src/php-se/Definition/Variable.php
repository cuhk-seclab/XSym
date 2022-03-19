<?php
/**
 * Define a variable used in execution
 */
class Variable {
    const Int_ = "Int_";
    const Bool_ = "Bool_";
    const Double_ = "Double_";
    const String_ = "String_";
    const Object_ = "Object_";
    const Array_ = "Array_";
    const Unknown_ = "String_";
    const CallSite = "CallSite_";
    const IsSet_ = "IsSet_";

    /**
     * Flag: Symbolic/Concrete
     * @var boolean
     */
    public $IsSymbolic = true;
    public $IsTainted = false;
    public $Expr;
    public $FromDatabase = false;
    public $DataAccess = false;
    public $FromEncrypt = false;
    public $Vars = [];
    public $LooseComp = false;

    public $Sources = []; // record the source of arguments

    /**
     * Value after applying Slice
     * @var Expr
     */
    public $Value = null;
    public $Sanitized = false;
    /**
     * The type of current expression
     */
    public $ExprType; ///// ????
    public $Types = [];
   /**
     * Class name
     * @var string 
     */
    public $ClassName = "";
    
    /**
     * Class Constants
     * @var array
     */
    public $Consts = [];
    
    /**
     * Properties of the class
     * @var array 
     */
    public $Props = [];

    /**
     * Name of the object
     * @var string
     */
    public $ObjectName = "";


    //public $ObjectType;
    public $Items = [];

    public $FuncName = "";
    public $Args = [];

    public function __construct($type = self::Unknown_) {
        if($type != NULL)
            $this->Types[] = self::Unknown_;
    }

    public function setObject($class="", $obj="") {
        $this->ClassName = $class;
        $this->ObjectName = $obj;
    }
    public function setCallSite($_class="", $_name="", $_args="") {
        $this->ClassName = $_class;
        $this->FuncName = $_name;
        $this->Args = $_args;
    }

    public function SetVariableType($type) {
        //$this->VariableType = $type;
        $this->Types[] = $type;
        //  TODO check type conversion here.
    }
    public function AddItem($key = NULL, $Value = NULL) {
        if($key === NULL) {
         //   echo "added ", count($this->Items), "\n";
            $key = new PhpParser\Node\Scalar\LNumber(count($this->Items));
            $this->Items[count($this->Items)] = new ArrayItem($key, $Value);// append
            return;
        }
        foreach($this->Items as $item) {
            if($key == $item->Key) {
                $item->Value = $Value;
                return;
            }
        }
        $key = new PhpParser\Node\Scalar\LNumber(count($this->Items));
        $this->Items[count($this->Items)] = new ArrayItem($key, $Value);// append
    }
    public function FindKey($key = NULL) {
        foreach($this->Items as $k => $item) {
            if($key instanceof PhpParser\Node\Scalar && $item->Key instanceof PhpParser\Node\Scalar) {
                if($key->value == $item->Key->value){
                   // echo "return";
                    return $item->Value;
                }
            }

            if($key == $item->Key) {
                return $item->Value;
            }
        }
        return NULL;
    }
        

}

class Operation extends Variable {
    public $Left;
    public $Right;
    public $Operator;

    public function __construct ($left, $right, $operator) {
        $this->Left = $left;
        $this->Right = $right;
        $this->Operator = $operator;
    }


    static public function BitwiseAnd($lhs, $rhs) {
        return new Operation($lhs, $rhs, '&');
    }
    static public function BitwiseOr($lhs, $rhs) {
        return new Operation($lhs, $rhs, '|');
    }
    static public function BitwiseXor($lhs, $rhs) {
        return new Operation($lhs, $rhs, '^');
    }
    static public function BooleanAnd($lhs, $rhs) {
        return new Operation($lhs, $rhs, '&&');
    }
    static public function BooleanOr($lhs, $rhs) {
        return new Operation($lhs, $rhs, '||');
    }
    static public function Coaleasce($lhs, $rhs) {
        return new Operation($lhs, $rhs, 'Unknown_');
    }
    static public function Concat($lhs, $rhs) {
        return new Operation($lhs, $rhs, '.');
    }
    static public function Div($lhs, $rhs) {
        return new Operation($lhs, $rhs, '/');
    }
    static public function Equal($lhs, $rhs) {
        return new Operation($lhs, $rhs, '==');
    }
    static public function GreaterOrEqual($lhs, $rhs) {
        return new Operation($lhs, $rhs, '>=');
    }
    static public function Greater($lhs, $rhs) {
        return new Operation($lhs, $rhs, '>');
    }
    static public function Identical($lhs, $rhs) {
        return new Operation($lhs, $rhs, '===');
    }
    static public function LogicalAnd($lhs, $rhs) {
        return new Operation($lhs, $rhs, 'and');
    }
    static public function LogicalOr($lhs, $rhs) {
        return new Operation($lhs, $rhs, 'or');
    }
    static public function LogicalXor($lhs, $rhs) {
        return new Operation($lhs, $rhs, 'xor');
    }
    static public function Minus($lhs, $rhs) {
        return new Operation($lhs, $rhs, '-');
    }
    static public function Mod($lhs, $rhs) {
        return new Operation($lhs, $rhs, '%');
    }
    static public function Mul($lhs, $rhs) {
        return new Operation($lhs, $rhs, '*');
    }
    static public function NotEqual($lhs, $rhs) {
        return new Operation($lhs, $rhs, '!=');
    }
    static public function NotIdentical($lhs, $rhs) {
        return new Operation($lhs, $rhs, '!==');
    }
    static public function Plus($lhs, $rhs) {
        return new Operation($lhs, $rhs, '+');
    }
    static public function Pow($lhs, $rhs) {
        return new Operation($lhs, $rhs, '**');
    }
    static public function ShiftLeft($lhs, $rhs) {
        return new Operation($lhs, $rhs, '<<');
    }
    static public function ShiftRight($lhs, $rhs) {
        return new Operation($lhs, $rhs, '>>');
    }
    static public function SmallerOrEqual($lhs, $rhs) {
        return new Operation($lhs, $rhs, '<=');
    }
    static public function Smaller($lhs, $rhs) {
        return new Operation($lhs, $rhs, '<');
    }
    static public function Spaceship($lhs, $rhs) {
        return new Operation($lhs, $rhs, '<=>');
    }
    static public function BitwiseNot($lhs) {
        return new Operation($lhs, NULL, '~');
    }
    static public function BooleanNot($lhs) {
        return new Operation($lhs, NULL, '!');
    }
    static public function Cast($lhs, $type) {
        return new Operation($lhs, NULL, 'Cast_' . $type);
    }

    static public function realprint($op) {
        $t = printexpr($op->Left->Value) . $op->Operator . printexpr($op->Right->Value) . "\n";
        print($t);
        return $t;
    }
}



class ArrayItem extends Variable{
    public $Key;
    public $Value;

    public function __construct($_Index = NULL, $_Value = NULL){
        $this->Key = $_Index;
        $this->Value = $_Value;
        $this->Types[] = Variable::Unknown_;
    }
}

