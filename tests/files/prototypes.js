/**
 * "type" : { "toString" : "function void (int, int)" }
 */
function Class(a, b) {
    /**
     * "useCount" : 4
     */
    this.a = a;

    /**
     * "useCount" : 4
     */
    this.b = b;
}

Class.prototype.a = 2;

/**
 * "useCount" : 2
 */
Class.prototype.print = /* */ function() {
    /**
     * "toString" : "Class o"
     */
    var o = this;

    console.log(o.a);
    this.b += o.a;
}

/**
 * "toString" : "Class o"
 */
var o = new Class(2, 3);
o.a += 5;
o.print();

var object = {
    /**
     * "useCount" : 2
     */
    a: false,
    method: function() { this.a = true; }
};

object.log = function() { console.log(this.a); }

/**
 * "toString" : "void Subclass ()"
 */
function Subclass() {
}
Subclass.prototype = Class;
Subclass.b = 2;

var o = {};
o.__proto__ = Subclass;
o.b = 3;

var mod = {};
mod.Subclass = Subclass;

var modInstance = new mod.Subclass();

/**
 * "type" : { "toString" : "int" }
 */
var modInstanceValue = modInstance.b;
