/**
 * "type" : { "toString" : "unsure (string, int)" }
 */
var hello;

hello = "world";
hello = 42;

/**
 * "type" : { "toString" : "function void (unsure (int, string), int)" }
 */
function type_still_unknown(a, b) {
    /**
     * "type" : { "toString" : "unsure (int, string)" }
     */
    a = 3;

    /**
     * "type" : { "toString" : "int" }
     */
    b;
}

type_still_unknown("don't crash here, one argument is missing");
type_still_unknown(3.14, false, "don't use these types, there are too many arguments");
type_still_unknown("now I know my type", 3);

/**
 * "toString" : { "toString" : "function void (int, mixed)" }
 */
var func1 = /* */ function(a, b) {};
func1(1, unknown_declaration);

/**
 * "type" : { "toString" : "function void (int, string)" }
 */
var func2 = func1;
func2(1, "now I know my type");

/**
 * "type" : { "toString" : "function bool ()" }
 */
var two_funcs;

two_funcs = function() { return "hello"; }
two_funcs = function() { return true; }
