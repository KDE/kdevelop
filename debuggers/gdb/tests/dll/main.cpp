#include <dlfcn.h> 

typedef int (*ft)(int);

int main()
{
    void* handle = dlopen("./libhelper.so", RTLD_LAZY);
    void* sym = dlsym(handle, "helper");

    ft f = (ft)sym;

    f(10);
    f(15);
    return 0;
}
