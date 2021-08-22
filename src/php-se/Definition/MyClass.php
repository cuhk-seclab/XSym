<?php
/**
 * Define a CFG class
 */
class MyClass{
    /**
     * Class name
     * @var string 
     */
    public $ClassName = "";
    public $FileName = "";
    
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

    public $Extends = '' ;

    /**
     * Methods/Functions inside the class
     * @var array
     */
    public $ClassMethods = [];

    /**
     * constructor
     * @param string $_ClassName
     */
    public function __construct($_ClassName = "") {
    	$this->ClassName = $_ClassName;
    }
}
