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