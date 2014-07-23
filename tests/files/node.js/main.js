var m = require("module");
var m2 = require("./module2.js");
var not_found = require("this_module_doesn_exist"); // Don't loop forever in BackgroundParser

/**
 * "type" : {"toString" : "bool" }
 */
var a = m.bar(4);

/**
 * "type" : { "toString" : "string" }
 */
var b = m2.greetings;
