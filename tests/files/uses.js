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
