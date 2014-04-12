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
 * "EXPECT_FAIL" : { "type" : "function expressions not yet handled" },
 * "type" : { "toString" : "function mixed (mixed)" }
 */
var c = function(a) {
  /**
   * "type" : { "toString" : "mixed" }
   */
  return a;
};

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
 * "EXPECT_FAIL": { "type" : "function expressions not yet handled", "returnType" : "function expressions not yet handled" },
 * "type": { "toString" : "function void ()" },
 * "returnType" : { "toString" : "void" }
 */
var recurse = function() {
    /**
     * "EXPECT_FAIL": { "type" : "Type recursion is not handled properly, as Declaration::abstractType has no pointer semantics (its indexed directly)" },
     * "type": { "toString" : "function void ()" }
     */
    var bla = recurse;
}
