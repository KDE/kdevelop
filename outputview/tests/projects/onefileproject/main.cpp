#include <iostream>

int main( int argc, char **argv )
{
    RingBuffer myBuffer;
    std::vector<uint64_t> testVector;

    std::srand( std::time ( 0 ) );

    fillBuffer( myBuffer, testVector );
    testWithOnlyknown( myBuffer, testVector );
    testWithSomeKnown( myBuffer, testVector);

    std::cout << "Done Testing" << std::endl;
    int i;
    if(i > 100)
    {
        /// This is of course only to test cpp check
        std::string input_string;
        char *str = (char *)malloc(strlen(input_string)+1);
        strcpy(str, input_string); /* What if malloc() fails? */
        if(input_string == "")
        {
        }
    }

    return 0;
}

