
#include <qstring.h>

struct B { int i; int j; static int k; };
struct C { int a[3]; };
int B::k = 11;
typedef int (*fp)(int);

int g = 10;
int g2 = 23;

void func(const QString& s)
{
   g = 10;
   
}

int main()
{
    
    
    printf("Hello world\n");
    int* p1 = 0x00000000;
    int** p1_p = &p1;
    p1 = &g;

    B* p2 = (B*)0x12345678;
    int (*p3)(int) = (fp)0x000000AE;
    B p4 = {1, 3};
    int p5[] = {5, 6, 7};
    int* p6[] = {&g, &g2};
    int p7[][2] = {{1,2}, {5,6}};
    B p8[] = {{1,2}, {3,4}};
    C p9 = {{7, 8, 9}};
    B& p10 = p4;
    int& p11 = *p1;
    int (*p12)[3] = &p5;
    int (&p13)[3] = p5;
    char p14[6] = "abc";
    wchar_t* p15 = L"test1"; 

 


    QString s = "test";
    QString* sp = &s;
    const QString& sr = s;
    func(s);
    
    
    p5[1] = 14;
    return 0;
}
