/**
 * "type" : { "toString" : "function void (mixed, mixed)" }
 */
function Class(a, b) {
    /**
     * "useCount" : 5
     */
    this.a = a;

    /**
     * "useCount" : 2
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