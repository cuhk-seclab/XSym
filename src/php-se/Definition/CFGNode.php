<?php
require_once "Constants.php";

class CFGNodeAbstract{
    public $Node;
    public $Stmt;
    public $Parent;
    public $Child;
    public $Type = '';
    public $Nodeid = 0;
    public function __construct($id) {
        $this->Nodeid = $id;
    }
}

class CFGCondNode extends CFGNodeAbstract {
    /**
     * @var Expr Stmt
     */
    public $Condition = NULL;

    /**
     * constructor
     */
    public function __construct($id = -1) {
        parent::__construct($id);
        $this->Type = Cond;
    }
}

class CFGIfNode extends CFGNodeAbstract {
    /**
     * @var array
     */
    public $Conditions = [];

    /**
     * @var array
     */
    public $Bodies = [];

    /**
     * constructor
     */
    public function __construct($id = -1, $IfConditions = [], $_IfBodies = []) {
        parent::__construct($id);
        $this->Type = IfStmt;
        $this->Conditions = $IfConditions;
        $this->Bodies = $_IfBodies;
    }
}

class CFGLoopNode extends CFGNodeAbstract {
    public $Init;
    public $Loop;
    public $Conditions = array();
    public $LoopType;
    public $Body = array();

    public function __construct($id = -1, $_LoopType = '', $_Conditions = [], $_Body = NULL, $_Init = NULL, $Loop = NULL) {
        parent::__construct($id);
        $this->Type = LoopStmt;
        $this->LoopType = $_LoopType;
        $this->Conditions = $_Conditions;
        $this->Body = $_Body;
        $this->Init = $_Init;
        $this->Loop = $_Loop;
    }
}
