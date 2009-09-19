
#include <QtCore/QString>
#include <vector>

struct B { int i; int j; static int k; };
struct C { int a[3]; };
struct D { int *ptr; };
int B::k = 11;
typedef int (*fp)(int);

int g = 10;
int g2 = 23;

void func2()
{
    int foobar = 123;
    printf("func2\n");
}

void func(QString& xs)
{
   int ac = 10;	
   std::string s;
   func2();
   g = 10;
   xs = "foo";
   
}

class Test
{
 public:
     QString n;
     int b;
};

struct S1 { int a; int b; };
struct S2 { int a; int b; int c; };

void test_type_changes(int x)
{
    S1 s = {1, 2};
    s.a++;
    {
        S2 s = {1, 2, 3};
        s.c++;
        s.a++;
    }
}

int test_main(int ac, char* av[])
{
    printf("Hello world\n");
    int i = 10;
    ++i;
    ++i;
    ++i;    
    int* p1 = 0x00000000;
    int** p1_p = &p1;
    p1 = &g;    

    B* p2 = (B*)0x12345678;
    g = 77;
    int (*p3)(int) = (fp)0x000000AE;
    B p4 = {1, 3};
    p4.i = 3;
    p2 = &p4;
    int p5[] = {5, 6, 7};
    int* p6[] = {&g, &g2};
    int p7[][2] = {{1,2}, {5,6}};
    B p8[] = {{1,2}, {3,4}};
    C p9 = {{7, 8, 9}};
    g = 77;
    const D p9_1 = {&g};
    {
        B p9_1;        
        int i = 15;
        printf("p9_1\n");
    }
    B& p10 = p4;
    int& p11 = *p1;
    int (*p12)[3] = &p5;
    int (&p13)[3] = p5;
    char p14[6] = "abc";
    wchar_t* p15 = L"test1"; 

 
    QString s = "test test test test";
    std::string ss = "test test test test";
    QString* sp = &s;
    const QString& sr = s;
    func(s);
    i = 15;
    
    std::vector<int> v;
    v.push_back(10);
    v.push_back(11);
    v.pop_back();
    
    std::vector<std::string> v2;
    v2.push_back("hi");
    v2.push_back("there");
    
    std::vector<int> v3;
    for (int i = 0; i < 20; ++i)
        v3.push_back(i);
    
    Test* test = new Test;
    Test& test2 = *test;
    test->n = "foo";
    printf("hi\n");
    test = 0;
    printf("hi2\n");
    //printf("hi %d\n", test->b);
    
    
    
    p5[1] = 14;
    return 7;
}

int main(int ac, char* av[])
{
    test_type_changes(11);
    return test_main(ac, av);
}
