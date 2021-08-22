<?php
require_once __DIR__ . "/Definition/Constants.php";
/**
 * recursively find all paths in the directory specified in $Path
 * @param string $path to the directory, existence should be guaranteed
 * @return array $filelist containing all files
 */
function FindAllFiles($Path) {
    if(!is_dir($Path))
        return [$Path];
    $Filelist = [];
    $Handle = opendir($Path);
    while($Entry = readdir($Handle)) {
        if($Entry == '.' or $Entry == '..')
            continue;
        if(!is_dir($Path . "/" . $Entry)) {
            if (substr($Entry, -4) == '.php')
                $Filelist[] = $Path . "/" .$Entry;
        }
        else {
            $Files = FindAllFiles($Path . "/" . $Entry);
            foreach($Files as $File)
                $Filelist[] = $File;
        }
    }
    return $Filelist;
}

/**
 * return a copy of input expr
 * @param Expr $expr
 * @return Expr $copy
 */
function CloneObject($expr){ 
     return DeepCopy\deep_copy($expr); 
}
/**
 * validate array index, e.g. convert from string to decimal
 * @param Int|Float|Strings|Bool $value
 * @return 
 */
function ValidatePHPArrayIndex($value) {
    if(get_class($value) == object or is_array($value) == true)
        return INVALID_KEY;
    if(is_float($value) or is_bool($value))
        return intval($value);
    if(is_null($value))
        return strval($value);
    if(is_string($value)) {
        if(is_numeric($value)) {
            if($value[0] != "0" && $value[0] != "+" && strpos($value, ".") === false && strpos($value, "e") === false)
                return intval($value);
        }
        return $value;
    }
    return INVALID_KEY;
}

function CopyObject($src, $dst){
    foreach($src as $key =>$value) {
        $dst->{$key} = $value;
    }
}

