<HTML>
<HEAD>
<TITLE>Database test #4</TITLE>
</HEAD>
<BODY>
<H1>ODBC Test 4 - Cursors</H1>
<em>The following test requires your ODBC driver to support positioned updates</em><p>
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
Updating table "php3_test"
<?
		odbc_autocommit($conn, 0);
  		if(($result = odbc_do($conn,"select * from php3_test where b>1002 for update"))){
			$cursor = odbc_cursor($result);
        		if(($upd = odbc_prepare($conn,"update php3_test set a=?,b=? where current of $cursor"))){
				while (odbc_fetch_row($result)) {
					$param[0] = odbc_result($result,1) . "(*)";
					$param[1] = odbc_result($result,2) + 2000;
					odbc_execute($upd,$param);
  				}
  				odbc_commit($conn);
			}
		}
		if($result && $upd){
?>
 - OK<p>
<H3>The table "php3_test" should now contain the following values:</H3>
<table><tr><th>A</th><th>B</th><th>C</th><th>D</th></tr>
<tr><td>test-1</td><td>1001</td><td>100.01</td><td>php3 - values 1</td></tr>
<tr><td>test-2</td><td>1002</td><td>200.02</td><td>php3 - values 2</td></tr>
<tr><td>test-3(*)</td><td>3003</td><td>300.03</td><td>php3 - values 3</td></tr>
<tr><td>test-4(*)</td><td>3004</td><td>400.04</td><td>php3 - values 4</td></tr>
<tr><td>test-5(*)</td><td>3005</td><td>500.05</td><td>php3 - values 5</td></tr>
<tr><td colspan=4><small><em><strong>Note:</strong> If you reload this testpage,<br>the three last rows will contain different<br>values in columns A and B</em></small></td></tr>
</table>

<H3>Actual contents of table "php3_test":</H3>
<?
			$res = odbc_exec($conn,"select * from php3_test");
			odbc_result_all($res);
		}else{
			echo "Your driver obviously doesn't support positioned updates<p>";
		}
?>
<p><HR width="50%"><p>
<A HREF="odbc-t5.php3<? echo "?dbuser=",$dbuser,"&dsn=",$dsn,"&dbpwd=",$dbpwd; ?>">Proceed to next test</A>
<?
	}
} else {
?>
<form action=odbc-t4.php3 method=post>
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
