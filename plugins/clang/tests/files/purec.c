typedef enum { Zero, One } Foobar;
/// "identifier" : "foo"
static const Foobar foo = 0 == 1;

#include <complex.h>

/// "type" : { "toString" : "_Complex float" }
float complex f_z = I * I;
/// "type" : { "toString" : "_Complex double" }
double complex d_z = I * I;
/// "type" : { "toString" : "_Complex long double" }
long double complex ld_z = I * I;
/// "type" : { "toString" : "const _Complex long double" }
const long double complex c_z = I * I;
/// "type" : { "toString" : "volatile _Complex float" }
volatile float complex v_z = I * I;
