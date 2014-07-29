/// "isDeprecated" : "true"
__attribute__ ((deprecated)) int deprecatedfunc() {}
/// "isDeprecated" : "false"
int nodeprecatedfunc() {}
/// "isDeprecated" : "true"
__attribute__ ((deprecated)) static const int deprecatedvar;
/// "isDeprecated" : "false"
static const int nodeprecatedvar;
