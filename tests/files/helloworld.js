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

/**
 * "type" : { "toString" : "function mixed (mixed)" }
 */
var c = function(a) {
  /**
   * "type" : { "toString" : "mixed" }
   */
  return a;
};

/**
 * "type" : { "toString" : "function int (mixed, mixed)" }
 */
var function_in_nested_expression = (function(a, b) { return 2; });

/**
 * "type" : { "toString" : "string" }
 */
var this_is_not_a_function = /** */ (function() { return "this is!"; })();

/**
 * "type" : { "toString" : "<class>" }
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
 * "type" : { "toString" : "string" }
 */
var arg = "arg";

/**
 * "type" : { "toString" : "string" }
 */
var s = testVariables();

/**
 * "type": { "toString" : "function mixed (mixed)" },
 * "returnType" : { "toString" : "mixed" }
 */
function testReturnMixedArg(arg)
{
  /**
   * "type" : { "toString" : "mixed" }
   */
  return arg;
}


/**
 * "type": { "toString" : "function void ()" },
 * "returnType" : { "toString" : "void" }
 */
var recurse = function() {
    /**
     * "EXPECT_FAIL": { "type" : "Type recursion is not handled properly, as Declaration::abstractType has no pointer semantics (its indexed directly)" },
     * "type": { "toString" : "function void ()" }
     */
    var bla = recurse;
    return;
}

/**
 * "returnType" : { "toString" : "unsure (int, double)" }
 */
function two_types(a) {
    if (a > 0) {
        return 2;
    } else {
        return -0.15;
    }
}

/**
 * "returnType" : { "toString" : "bool" }
 */
function two_returns_same_type(a) {
    if (a > 0) {
        return true;
    } else {
        return false;
    }
}

/**
 * "returnType" : { "toString" : "unsure (void, int)" }
 */
function may_return_void(a) {
    if (a == 0) {
        return;
    }

    return a << 2;
}

/**
 * "returnType" : { "toString" : "double" }
 */
function ignore_mixed(a) {
    if (a) {
        return a;
    } else {
        return 3.14;
    }
}
