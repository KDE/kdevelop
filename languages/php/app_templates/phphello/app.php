<?php

%{PHP_TEMPLATE}

printHeader();
echo "<h1>Hello World!</h1>\n";
printFooter();

function printHeader(){
	echo <<<EOF
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
	<meta name="description" content="" />
	<meta name="author" content="%{AUTHOR}" />
	<meta name="keywords" content="" />
	<title>%{APPNAME}</title>
</head>
<body>

EOF;
}

function printFooter(){
	echo "</body>\n</html>";
}

?>
