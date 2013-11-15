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
 * "type" : { "toString" : "int" }
 */
var b = a;

/**
 * "type" : { "toString" : "double" }
 */
var d = 1.2;

/**
 * "type" : { "toString" : "array" }
 */
var array = [1, 2, 3];

/**
 * "type" : { "toString" : "mixed" }
 */
var c = function(a) {
    return a;
};

/**
 * "type" : { "toString" : "class" }
 */
var obj = {
    foo: "bar"
};

/**
 * "type" : { "toString"  : "function string ()" },
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
 * "type": { "toString" : "function mixed (mixed)" },
 * "returnType" : { "toString" : "mixed" }
 */
function testReturnMixedArg(arg)
{
    return arg;
}
