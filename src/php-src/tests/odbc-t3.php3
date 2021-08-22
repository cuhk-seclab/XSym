<HTML>
<HEAD>
<TITLE>Database test #3</TITLE>
</HEAD>
<BODY>
<H1>ODBC Test 3 - Insert records</H1>
<?
if(isset($dbuser)){
?>
Connecting to <? echo $dsn; ?> as <? echo $dbuser; ?>
<?
    $conn = odbc_connect($dsn,$dbuser,$dbpwd);
    if (!$conn){
?>
<H2>Error connecting to database! Check DSN, username and password</H2>
<?
    }else{
?>
 - OK<p>
Clearing table "php3_test"
<?
error_reporting(0);
$res=odbc_exec($conn,"delete from php3_test");
odbc_free_result($res);
error_reporting(1);
?>
 - OK<p>
Inserting into table "php3_test"
<?
	$sqlfloat="00.0";
	$sqlint = 1000;
	$stmt=odbc_prepare($conn,"insert into php3_test values(?,?,?,?)");
	for($i=1;$i<=5;$i++){
		$values[0]="test-$i";
		$values[1]=$sqlint + $i;
		$values[2]=$i . $sqlfloat . $i;
		$values[3]="php3 - values $i";
		$res = odbc_execute($stmt,$values);
	}
	$res = odbc_exec($conn,"select count(*) from php3_test");
	if($res && (odbc_result($res,1) == 5)){
?>
 - OK<p>
<H3>The table "php3_test" should now contain the following values:</H3>
<table><tr><th>A</th><th>B</th><th>C</th><th>D</th></tr>
<tr><td>test-1</td><td>1001</td><td>100.01</td><td>php3 - values 1</td></tr>
<tr><td>test-2</td><td>1002</td><td>200.02</td><td>php3 - values 2</td></tr>
<tr><td>test-3</td><td>1003</td><td>300.03</td><td>php3 - values 3</td></tr>
<tr><td>test-4</td><td>1004</td><td>400.04</td><td>php3 - values 4</td></tr>
<tr><td>test-5</td><td>1005</td><td>500.05</td><td>php3 - values 5</td></tr>
</table>

<H3>Actual contents of table "php3_test":</H3>
<?
		$res = odbc_exec($conn,"select * from php3_test");
		odbc_result_all($res);
?>
<p><HR width="50%"><p>
<A HREF="odbc-t4.php3<? echo "?dbuser=",$dbuser,"&dsn=",$dsn,"&dbpwd=",$dbpwd; ?>">Proceed to next test</A>
| <A HREF="<? echo $PHP_SELF; ?>">Change login information</A>
<?
	}
    }
} else {
?>
<form action=odbc-t3.php3 method=post>
<table border=0>
<tr><td>Database: </td><td><input type=text name=dsn></td></tr>
<tr><td>User: </td><td><input type=text name=dbuser></td></tr>
<tr><td>Password: </td><td><input type=password name=dbpwd></td></tr>
</table>
<input type=submit value=connect>

</form>
<? 
} ?>
</BODY>
</HTML>
