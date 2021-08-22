<?php
class CFG{
	public static function ConstructCFG($stmts) {
        global $nodeindex;
	    $start = new CFGNodeAbstract($nodeindex ++);
        Store::$idmaptonode[$nodeindex - 1] = $start; // add id map => node
	    $start->parent = null;  # if parent is null, then it reaches the beginings
	    $parent = $start;
	    $parent->type = Start;

        if(!is_null($stmts)) {
            foreach ($stmts as $stmt) {
                if ($stmt instanceof PhpParser\Node\Stmt\If_) {
                    $current = new CFGIfNode($nodeindex ++);
                    Store::$idmaptonode[$nodeindex - 1] = $current; // add id map => node
                    $if= CFG::ProcessIfStmt($stmt);//
                    $current->conds = $if[0];
                    foreach($current->conds as $cond){
                        $cond->parent = $current;
                    }
                    // should define the types of body starts and body ends
                    $current->bodystarts = $if[1];
                    $current->bodyends = $if[2];

                    $current->stmt = $stmt;
                    $current->parent = $parent;
                    $parent->child = $current;
                    $dummynode = $if[3];
                    $parent = $dummynode;

                }
                else if ($stmt instanceof PhpParser\Node\Stmt\Foreach_ || 
                    $stmt instanceof PhpParser\Node\Stmt\For_ || 
                    $stmt instanceof PhpParser\Node\Stmt\While_ || 
                    $stmt instanceof PhpParser\Node\Stmt\Do_) {
                    //print "here comes into loop\n";
                    $current = new CFGLoopNode($nodeindex ++);
                    Store::$idmaptonode[$nodeindex - 1] = $current; 
                    // add id map => node
                    // $current->type = LoopStmt;
                    $loop = CFG::ProcessLoopStmt($stmt);
                    $current->looptype = $loop[0];
                    $current->bodystart = $loop[1];
                    $current->bodyend = $loop[2];
                    $current->cond = $loop[3];
                    $current->init = $loop[4];
                    $current->loop = $loop[5];
                    $current->bodystart->parent = $current;
                    //
                    $current->stmt = $stmt;
                    $current->parent = $parent;
                    $parent->child = $current;

                    $dummynode = $loop[6];
                    $parent = $dummynode;
                } 
                else if($stmt instanceof PhpParser\Node\Stmt\Switch_){          
                    $current = new CFGIfNode($nodeindex ++ );
                    Store::$idmaptonode[$nodeindex - 1] = $current; 
                    // add id map => node
                    $switch = CFG::ProcessSwitchStmt($stmt);
                    $current->conds = $switch[0];
                    $current->bodystarts = $switch[1];
                    $current->bodyends = $switch[2];
                    foreach($current->conds as $cond) {
                        $cond->parent = $current;
                    }

                    $current->stmt = $stmt;
                    $current->parent = $parent;
                    $parent->child = $current;

                    $dummynode = $switch[3];
                    $parent = $dummynode;

                }
                else if($stmt instanceof PhpParser\Node\Stmt\TryCatch) {
                    // print "here in trycatcch\n";
                    $trycfg = CFG::ConstructCFG($stmt->stmts);
                    $current = $trycfg[0];
                    $parent->child = $current;
                    $current->parent = $parent;
                    $parent = $trycfg[1]->parent;

                    $current->stmt = $stmt;
                    $current->parent = $parent;
                    $parent->child = $current;
                    $parent = $current;
                }
                elseif(($stmt instanceof PhpParser\Node\Stmt\Nop) || ($stmt instanceof PhpParser\Node\Stmt\Function_) || ($stmt instanceof PhpParser\Node\Expr\Stmt\Class_)) {
                    //print("ConstructCFG CLASS/FUNCTION\n");
                }
                else {                    
                    $current = new CFGNodeAbstract($nodeindex ++);
                    Store::$idmaptonode[$nodeindex - 1] = $current; 
                    // add id map => node
                    $current->type = Stmt;
                    $current->node = $stmt;     

                    $current->stmt = $stmt;
                    $current->parent = $parent;
                    $parent->child = $current;
                    $parent = $current;
                }
                /**
                 * except the situations when nop/functions/classes/
                 */
                if((!$stmt instanceof PhpParser\Node\Stmt\Nop) && (! $stmt instanceof PhpParser\Node\Stmt\Function_) && (! $stmt instanceof PhpParser\Node\Expr\Stmt\Class_) && (! $stmt instanceof PhpParser\Node\Stmt\TryCatch)) {
                }       
                else{
                    #print get_class($stmt)."\n";
                    #print "should not happend\n";
                }
            }
        }
        
	    $end = new CFGNodeAbstract($nodeindex ++);
        Store::$idmaptonode[$nodeindex - 1] = $end;
        // add id map => node
	    $end->type = end;
	    $parent->child = $end;
	    $end->parent = $parent;
	    $end->child = null;
	    return array($start, $end);
    }


	// now reconstruct conditon node/ if /else if/  else and so on/
	// it should follow with several branches(at least one)
	// and transfer it into
	// and one dummy node after that
	/**
	 * return order should be like
	 * ifcond, elseifconds(array), ifstmtNodes, elseifstmtNodes, enda
	 */
	public static function ProcessIfStmt($stmtIf) {
        global $nodeindex;
	    // stmtIf has keys 'cond', 'stmts', 'elseifs', and 'else'.
	    // Array of CFG nodes representing the conditions.
	    $ifconds = array();
	    $ifcond = new CFGCondNode($nodeindex ++);
        Store::$idmaptonode[$nodeindex - 1] = $ifcond;
        // add id map => node
	    $ifcond->cond = $stmtIf->cond;
	    $bodystarts = array();
	    $bodyends = array();
        
	    $dummynode = new CFGNodeAbstract($nodeindex ++);
        Store::$idmaptonode[$nodeindex - 1] = $dummynode;

	    $body = CFG::ConstructCFG($stmtIf->stmts);
	    $body[0]->parent = $ifcond;
	    $ifconds[] = $ifcond;
	    $bodystarts[] = $body[0];
	    $bodyends[] = $body[1];
        $body[1]->child = $dummynode;

	    foreach($stmtIf->elseifs as $elseif) {
	        $cond_node = new CFGCondNode($nodeindex ++);
            Store::$idmaptonode[$nodeindex - 1] = $cond_node;
            // add id map => node
	        $cond_node->cond = $elseif->cond;
	        $body = CFG::ConstructCFG($elseif->stmts);
	        $ifconds[] = $cond_node;
	        $body[0]->parent = $cond_node;
	        $bodystarts[] = $body[0];
	        $bodyends[] = $body[1];
            $body[1]->child = $dummynode;
	    }
	     // Create and add the else body node if it exists
	    $cond_node = new CFGCondNode($nodeindex ++);
        Store::$idmaptonode[$nodeindex - 1] = $cond_node;
        // add id map => node
	    $cond_node->cond = null;
	    if ($stmtIf->else) {
	        $body = CFG::ConstructCFG($stmtIf->else->stmts);
	    }
	    else{
	         $body = CFG::ConstructCFG([]);
	    }
	    $body[0]->parent = $cond_node;
	    $body[1]->child = $dummynode;

	    $ifconds[] = $cond_node;
	    $bodystarts[] = $body[0];
	    $bodyends[] = $body[1];
	    return array($ifconds, $bodystarts, $bodyends, $dummynode);
	}


	// Constructs a node of loop.
	// 1) Creates a CFG node for the loop condition that
	// acts as the loop header.
	// 2) Creates a CFG of the body of the loop.
	// 3) Links the exit of the body CFG to the loop header CFG.
	// 4) Creates an exit dummy node.
	// 5) Links the condition node to the CFG of the body and the dummy
	// exit node.
	public function ProcessLoopStmt($stmtLoop) {
	    // Create the CFG node for the loop header.
        global $nodeindex;
	    $init = $loop = null;
	    $looptype = null;
	    $cond = null;
	    if ($stmtLoop instanceof PhpParser\Node\Stmt\Foreach_) {
	        $cond = $stmtLoop->expr;
	        $looptype = ForeachLoop;
	    }
	    else if ($stmtLoop instanceof PhpParser\Node\Stmt\For_) {
	        $init = $stmtLoop->init;
	        foreach($stmtLoop->cond as $co)
	            $cond = And_($cond, $co);
	        $loop = $stmtLoop->loop; 
	        $looptype = ForLoop;
	    }
	    else if ($stmtLoop instanceof PhpParser\Node\Stmt\While_) {
	        $cond = $stmtLoop->cond;
	        $looptype = WhileLoop;
	    }
	    else if ($stmtLoop instanceof PhpParser\Node\Stmt\Do_) {
	        $cond = $stmtLoop->cond;
	        $looptype = DoWhileLoop;
	    }
	    else {
	        print "ERROR Unrecognized loop type while construction CFG node.\n";
	    }
	    $body = CFG::ConstructCFG($stmtLoop->stmts);


	    $dummynode = new CFGNodeAbstract($nodeindex ++);
        Store::$idmaptonode[$nodeindex - 1] = $dummynode;
        $body[1]->child = $dummynode;

	    return array($looptype, $body[0], $body[1], $cond, $init, $loop, $dummynode);
	}

	public function ProcessSwitchStmt($stmtSwitch){
        global $nodeindex;
        //global $idmaptonode;

	    $conds = array();
	    $bodystarts = array();
	    $bodyends = array();
	    $elsebody = null;
	    $dummynode = new CFGNodeAbstract($nodeindex ++);
        Store::$idmaptonode[$nodeindex - 1] = $dummynode;

	    foreach($stmtSwitch->cases as $case){        
	        $body = CFG::ConstructCFG($case->stmts);
	        if($case->cond){
	            $cond = new CFGCondNode($nodeindex ++);
                Store::$idmaptonode[$nodeindex - 1] = $cond; 
                // add id map => node
	            $cond->cond = new PhpParser\Node\Expr\BinaryOp\Equal($case->cond, $stmtSwitch->cond);
	            $conds[] = $cond;
	            $bodystarts[] = $body[0];
	            $body[0]->parent = $cond;
	            $bodyends[] = $body[1];
                $body[1]->child = $dummynode;
	        }
	        else{
	            $elsebody = $body;
	        }
	    }
	    $cond = new CFGCondNode($nodeindex ++);
        Store::$idmaptonode[$nodeindex - 1] = $cond;
        // add id map => node
	    $cond->cond = null;
	    $conds[] = $cond;
	    if (!$elsebody){
	        $elsebody = CFG::ConstructCFG([]);
	    }
	    $bodystarts[] = $elsebody[0];
	    $elsebody[0]->parent = $cond;
	    $bodyends[] = $elsebody[1];
        $elsebody[1]->child = $dummynode;
	    return array($conds, $bodystarts, $bodyends, $dummynode);
	}
}
