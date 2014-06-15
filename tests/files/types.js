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

im_used_in_types_js = "and I'm a string";
