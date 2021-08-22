<?php
/**
 * Class MyFunction
 * Defines the CFG representation of a single function
 * Main functions are also be in this format
 */
class MyFunction{
    /**
     * Name of the class 
     * @var string 
     */
	public $ClassName = '';

    /**
     * Name of the function
     * @var string
     */
	public $FuncName = '';
    public $FileName = '';
    public $FuncVisit = false;

    /**
     * Name/Values pairs of parameters
     * @var array
     */
	public $Params = [];

    /**
     * The start point node of the CFG
     * @var CFGNode
     */
	public $Body = null;
    public $visited = false;

    /**
     * Initialize the class
     * @param string $classname
     * @param string $functioncname
     */
	public function __construct($_ClassName, $_FuncName) {
		$this->ClassName = $_ClassName;
		$this->FuncName = $_FuncName;
	}
}
