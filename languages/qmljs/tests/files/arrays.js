/**
 * "type" : { "toString" : "<class>" }
 */
var obj = {
  foo: "bar"
};

var array = {
    /**
     * "type" : { "toString": "string" },
     * "useCount": 1
     */
    key: "foo",

    /**
     * "type": { "toString": "unsure (int, string)" }
     */
    key2: 3
};

array.key = "bar";
array.key2 = "bar";

/**
 * "type" : { "toString" : "int" }
 */
array.new_key = 3;

/**
 * "type" : { "toString" : "<class>" }
 */
array.child = {};

/**
 * "type" : { "toString" : "string" }
 */
array["child"]["key"] = "value";

/**
 * "useCount" : 0
 */
var dont_use_me;

array.dont_use_me = "fail";