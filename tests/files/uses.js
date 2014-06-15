/**
 * "useCount" : 3
 */
var global = 1;

/**
 * "useCount" : 2,
 * "internalFunctionContext" : { "declarations" : { "0" : { "useCount" : 1 } } }
 */
function func(arg) {
    func(arg, global);
}

/**
 * "useCount" : 1
 */
var i = global;
func(global, i);

/**
 * "useCount" : 0
 */
var line;
online = false;     // onFoo replaced by foo when foo is a QML signal

/**
 * "useCount" : 1
 */
var im_used_in_types_js;
