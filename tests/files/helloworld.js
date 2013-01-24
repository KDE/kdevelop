/**
 * "type" : { "toString"  : "function void ()" },
 * "returnType" : { "toString"  : "void" }
 */
function helloWorld()
{
  alert("Hello World!");
}

/**
 * "EXPECT_FAIL" : { "type" : { "toString" : "int" } }
 */
var a = 5;

/**
 * "EXPECT_FAIL" : {
 *   "type" : { "toString" : "function String ()" },
 *   "returnType" : { "toString" : "String" }
 * }
 */
function testVariables()
{
 /**
  * "EXPECT_FAIL" : { "type" : { "toString" : "String" } }
  */
  var b = "some text";
}
