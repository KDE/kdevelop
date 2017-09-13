/**
 * "type" : { "toString" : "function void ()" }
 */
var f = function () { }

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