
void foo(int);

void set_value(int* i)
{
    *i = 10;
}

void modify(int* i)
{
    *i = 15;
}

void read(int* i)
{
    static int i2;
    i2 = *i;
}

int test_main(int* i)
{
    foo(5);
    set_value(i);
    
    modify(i);
    read(i);
    
    for(unsigned j = 0; j < 10; ++j)
	foo(j);
        
    return 0;    
}

int main()
{
    int var;
    return test_main(&var);
}
