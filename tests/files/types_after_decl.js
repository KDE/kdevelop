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
