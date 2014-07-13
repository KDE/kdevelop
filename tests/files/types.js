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
var a = 50;

/**
 * "type" : { "toString" : "int" }
 */
var b = a;

/**
 * "type" : { "toString" : "double" }
 */
var d = 1.0;

/**
 * "type" : { "toString" : "int" }
 */
var e = a = 60;

/**
 * "type" : { "toString" : "array" }
 */
var array = [1, 2, 3];

/**
 * "type" : { "toString" : "bool" }
 */
var simple_compare = (a == 5.89);

/**
 * "type" : { "toString": "bool" }
 */
var nested_conditions = (simple_compare && b < 3);

/**
 * "type" : { "toString" : "int" }
 */
var simple_shift = (a << 2);

/**
 * "type" : { "toString" : "double" }
 */
var result_of_sin = Math.sin(0);

/**
 * "type" : { "toString" : "RegExp" }
 */
var regexp = new RegExp(".*");
