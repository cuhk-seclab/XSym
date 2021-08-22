<?php
/**
 * Define a Slice/environment of execution
 */
class Slice {
    /**
     * All variables: variable name - value index pairs
     * Simulate the address mapping
     * @var array 
     */
    public $Variables = [];
    /**
     * Address space for variable value fetch
     * @param array idx => Variable pairs
     */
    public $TargetObject = NULL;
    public $VariableValues = []; 
    public $VariableTypes = [];
    public $ClassName;
    public $FuncName;
    public $FileName;
    public $StaticClasses = [];
    public $Value;
    public $CallStack = [];
    public $IsReturnTainted = false;
    public function pushcall($Class='', $Func='') {
        $key = $Class . '+' . $Func;
        if(in_array($key, $this->CallStack))
            return false;
        array_push($this->CallStack, $key);
        return true;
    }

    public function popcall() {
        array_pop($this->CallStack);
    }

    public function incallstack($Class = '', $Func = ''){
        $key = $Class . '+' . $Func;
        if(in_array($key, $this->CallStack))
            return true;
        return false;
    }

    /**
     * Constraint to current state
     * Each condition is stored separated as an item
     * @var array
     */
    public $Constraints = [];

    /**
     * Padding $Count for either number of instructions
     * Or deepth/layers of call stacks.
     * @var number
     */
    public $Count;

    public function __construct($ClassName = "", $FuncName = "", $File){
        $this->ClassName = $ClassName;
        $this->FuncName = $FuncName;
        $this->FileName = $File;
    }
}
