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
 * "type" : { "toString" : "string" }
 */
var c = "hello";

/**
 * "type" : { "toString" : "double" }
 */
var d = 1.0;

/**
 * "type" : { "toString" : "int" }
 */
var e = a = 60;

/**
 * "type" : { "toString" : "function void ()" }
 */
var f = function(){};

/**
 * "type" : { "toString" : "array" }
 */
var array = [1, 2, 3];

/**
 * "type" : { "toString" : "RegExp" }
 */
var regexp = /.*/;

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

var o = {};
/**
 * "type" : { "toString" : "string" }
 */
var result_of_tostring = o.toString();

var o2 = new Date();
/*
 * "type" : { "toString" : "string" }
 */
var result_of_date_tostring = o2.toString();

var o3 = new RangeError();
/**
 * "type" : { "toString" : "string" }
 */
var range_error_inherits_from_error = o3.message;

/**
 * "type" : { "toString" : "bool" }
 */
var number_is_finite = a.isFinite();

/**
 * "type" : { "toString" : "bool" }
 */
var number_is_finite2 = d.isFinite();

/**
 * "type" : { "toString" : "array" }
 */
var array_concat = array.concat();

/**
 * "type" : { "toString" : "string" }
 */
var string_charat = c.charAt(0);

/**
 * "type" : { "toString" : "int" }
 */
var function_length = f.length;

/**
 * "type" : { "toString" : "bool" }
 */
var boolean_valueof = simple_compare.valueOf();

/**
 * "type" : { "toString" : "array" }
 */
var regexp_exec = regexp.exec();
