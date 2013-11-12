/**
 * "type" : { "toString"  : "function void ()" },
 * "returnType" : { "toString"  : "void" }
 */
function helloWorld()
{
  alert("Hello World!");
}

/**
 * "type" : { "toString" : "int" }
 */
var a = 5;

/**
 * "type" : { "toString" : "double" }
 */
var d = 1.2;

/**
 * "EXPECT_FAIL" : { "returnType" : "return type is not properly deduced" },
 * "returnType" : { "toString" : "string" }
 */
function testVariables()
{
 /**
  * "type" : { "toString" : "string" }
  */
  var b = "some text";
  return b;
}

/**
 * "EXPECT_FAIL" : { "type" : "neither arg type nor ret type is properly deduced" },
 * "type": { "toString" : "function mixed (mixed)" }
 */
function testReturnMixedArg(arg)
{
    return arg;
}
