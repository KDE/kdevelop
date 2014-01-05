/// "toString" : "int main (int, signed char**)"
int main(int argc, char** argv)
{
    /// "toString" : "short int s"
    short s;
    /// "toString" : "int a"
    int a;
    /// "toString" : "const float b"
    const float b = 0;
    /// "toString" : "volatile long long int c"
    volatile long long c;
    /// "toString" : "void* v_ptr"
    void* v_ptr;
    /// "toString" : "void* const* v_ptr2"
    void* const* v_ptr2;
    /// "toString" : "int[5] arr"
    int arr[5];
    /// "toString" : "unsigned int uint"
    unsigned int uint;
    /// "toString" : "long unsigned int ulong"
    unsigned long ulong;
    /// "toString" : "long long unsigned int ulonglong"
    unsigned long long ulonglong;
    /// "toString" : "short unsigned int ushort"
    unsigned short ushort;
    /// "toString" : "const int& a_lref"
    const int& a_lref = a;
    /// "toString" : "int&& a_rref"
    int&& a_rref = a + a;
    /// TODO: enums, enumerators, typedefs, ...
}
