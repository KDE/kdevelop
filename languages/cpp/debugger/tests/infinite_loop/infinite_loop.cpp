
int foo()
{
    int i = 0;
    for(;;)
    {
	i = i+1;
    }
    return i;
}

int main()
{
    int r = 10;
    r += foo();    
    return r;
}