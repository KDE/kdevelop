// "new this.foo" declares the foo class, that gets assigned to itself, which confuses StructureType::declaration()
function() {
	this.foo = new this.foo();
	this.foo.bar();
}
