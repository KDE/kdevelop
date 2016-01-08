/// "isDeprecated" : "true"
__attribute__ ((deprecated)) int deprecatedfunc() { return 0; }
/// "isDeprecated" : "false"
int nodeprecatedfunc() { return 0; }
/// "isDeprecated" : "true"
__attribute__ ((deprecated)) static const int deprecatedvar = 0;
/// "isDeprecated" : "false"
static const int nodeprecatedvar = 0;
