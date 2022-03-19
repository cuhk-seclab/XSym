<?php
include "EnableWarning.php";
if (count($argv) <= 1){
    print("Please Input File/Directory Location\n");
    exit(1);
}
include_once __DIR__ ."/ConstructCFG.php";
include_once __DIR__ ."/Definition/Constants.php";
include_once __DIR__ ."/Definition/GlobalVariable.php";
include_once __DIR__ ."/Execution.php";
include_once __DIR__ ."/Definition/Slice.php";

/**
 * Start Analysis
 * @param string $AppPath path to the root of the app, existence should be guaranteed
 */
function MainAnalysisStart($AppPath) {
    $AbsPath = realpath($AppPath);
    $AllClasses = ConstructAppCFG($AbsPath);
    GlobalVariable::$AllClasses = $AllClasses;
    $Class = $AllClasses[MAIN_CLASS];
    $ClassName = MAIN_CLASS;
    foreach($Class->ClassMethods as $MethodName => $Method) {
        if($Method->visited == false && substr($MethodName, -4 ) == ".php") {
            $Analyzer = new Execution();
            $Slice = new Slice($ClassName, $MethodName, $Method->FileName);
            $Method->FuncVisit = true;
            //echo $MethodName, "\n";
            $pos = 0;
            foreach($Method->Params as $paramname => $value) {
                $newparam =  new Variable("argtype" . (string)$pos);
                $newparam->Value = new PhpParser\Node\Expr\Variable($paramname);
                $newparam->Sources[] = 'arg' . (string)$pos;
                $Slice->Variables[$paramname] = $pos;
                $Slice->VariableValues[] = $newparam;
                $pos ++;
            }
            $Analyzer->ExecutionPerNode($Method->Body[0], $Method->Body[1], $Slice);
            GlobalVariable::$Analyzers[$ClassName][$MethodName] = $Analyzer;
            $Analyzer->DumpConstraints($ClassName, $MethodName, array_keys($Method->Params));
        }
    }
}


$AppPath = $argv[1];
if(file_exists($AppPath)) {
    MainAnalysisStart($AppPath);
}
else{
    print("The app path does not exist!\n");
    exit(1);
}
