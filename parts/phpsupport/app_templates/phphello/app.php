<?php

printHeader();
echo "<H1>Hello World<H1>";
printFooter();

function printHeader(){
  echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"\n" .
    "\"http://www.w3.org/TR/REC-html40/loose.dtd\"><HTML>\n" . 
    "<HEAD>\n" .
    "<meta name=\"description\" content=\"\" >\n" .
    "<meta name=\"author\" content=\"$AUTHOR$\">\n" .
    "<meta name=\"keywords\" content=\"\" >\n" .
    "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n" .
    "<TITLE>$APPNAME$</TITLE>\n" .
    "</HEAD>\n". 
    "<BODY>\n";
}

function printFooter(){
  echo "\n</BODY></HTML>";
}

?>
