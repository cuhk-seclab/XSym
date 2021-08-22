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
$BothTaintInfo = [];
$OneTaintInfo = [];
$FromEncryptInfo = [];
$FromDatabaseInfo = [];
$XSSInfo = [];
$SQLiInfo = []; 
$LooseCompInfo = []; 
$DataAccessInfo = []; 

$LooseComp1log = [];
$LooseComp2log = [];
$SQLilog = [];
$ACClog = [];
$XSSlog = [];
$Authlog = [];

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
        if($Method->visited == false) {
            $Analyzer = new Execution();
            $Slice = new Slice($ClassName, $MethodName, $Method->FileName);
            $Method->FuncVisit = true;
            //echo $MethodName, "\n";
            $pos = 0;
            foreach($Method->Params as $paramname => $value) {
                $newparam =  new Variable("argtype" . (string)$pos);
                $newparam->Sources[] = 'arg' . (string)$pos;
                $Slice->Variables[$paramname] = $pos;
                $Slice->VariableValues[] = $newparam;
                $pos ++;
            }
            $Analyzer->ExecutionPerNode($Method->Body[0], $Method->Body[1], $Slice);
            GlobalVariable::$Analyzers[$ClassName][$MethodName] = $Analyzer;
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
