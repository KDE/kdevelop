
// Euclid's algorithm for calculating the greatest common divisor
int main()
{	
    int i, j;
    i = 157;
    j = 312;

    while( i != j )
    {
       if( i > j ) i = i-j;
       else j = j-i;
    }
    return 0;
}
