

#include "rangenumber.h"
#include <iostream>

int passed;
int failed;
int tests;

template <class T>
void test(SingleRange<T> range, T start, T end)
{
    tests++;
    if(range.m_valid && range.m_start == start && range.m_end == end) {
        passed++;
        std::cout << tests << ". Passed." << std::endl;
    } else {
        failed++;
        std::cout << tests << ". Failed. ";
        range.toString();
        std::cout << std::endl;
    }
}

template <class T>
void testerror(SingleRange<T> range)
{
    tests++;
    if(!range.m_valid) {
        passed++;
        std::cout << tests << ". Passed." << std::endl;
    } else {
        failed++;
        std::cout << tests << ". Failed. ";
        range.toString();
        std::cout << std::endl;
    }
}


int main(void) {
    
    passed = 0;
    failed = 0;
    tests = 0;
    SingleRange<int> range(3,4);
    
    std::cout << "Testing == on ints" << std::endl;
    test(SingleRange<int>(5,6) == SingleRange<int>(3,4) , false, false);
    test(SingleRange<int>(5,6) == SingleRange<int>(3,5) , false, true);
    test(SingleRange<int>(5,6) == SingleRange<int>(5,6) , false, true);
    test(SingleRange<int>(3,5) == SingleRange<int>(5,7) , false, true);
    test(SingleRange<int>(3,5) == SingleRange<int>(6,7) , false, false);
    test(SingleRange<int>(3,3) == SingleRange<int>(3,3) , true, true);
 
    std::cout << "Testing < on ints" << std::endl;
    test(SingleRange<int>(5,6) < SingleRange<int>(3,4) , false, false);
    test(SingleRange<int>(5,6) < SingleRange<int>(3,5) , false, false);
    test(SingleRange<int>(5,6) < SingleRange<int>(-1,6) , false, true);
    test(SingleRange<int>(0,5) < SingleRange<int>(5,7) , false, true);
    test(SingleRange<int>(-3,5) < SingleRange<int>(6,7) , true, true);
    test(SingleRange<int>(3,3) < SingleRange<int>(-3,3) , false, false);
    test(SingleRange<int>(0,0) < SingleRange<int>(0,0) , false, false);

    std::cout << "Testing > on ints" << std::endl;
    test(SingleRange<int>(5,6) > SingleRange<int>(3,4) , true, true);
    test(SingleRange<int>(5,6) > SingleRange<int>(3,5) , false, true);
    test(SingleRange<int>(5,6) > SingleRange<int>(-1,6) , false, true);
    test(SingleRange<int>(0,5) > SingleRange<int>(5,7) , false, false);
    test(SingleRange<int>(-3,5) > SingleRange<int>(6,7) , false, false);
    test(SingleRange<int>(3,3) > SingleRange<int>(-3,3) , false, true);
    test(SingleRange<int>(0,0) > SingleRange<int>(0,0) , false, false);
  
    std::cout << "Testing <= on ints" << std::endl;
    test(SingleRange<int>(5,6) <= SingleRange<int>(3,4) , false, false);
    test(SingleRange<int>(5,6) <= SingleRange<int>(3,5) , false, true);
    test(SingleRange<int>(5,6) <= SingleRange<int>(-1,6) , false, true);
    test(SingleRange<int>(0,5) <= SingleRange<int>(5,7) , true, true);
    test(SingleRange<int>(-3,5) <= SingleRange<int>(6,7) , true, true);
    test(SingleRange<int>(3,3) <= SingleRange<int>(-3,3) , false, true);
    test(SingleRange<int>(0,0) <= SingleRange<int>(0,0) , true, true);

    std::cout << "Testing >= on ints" << std::endl;
    test(SingleRange<int>(5,6) >= SingleRange<int>(3,4) , true, true);
    test(SingleRange<int>(5,6) >= SingleRange<int>(3,5) , true, true);
    test(SingleRange<int>(5,6) >= SingleRange<int>(-1,6) , false, true);
    test(SingleRange<int>(0,5) >= SingleRange<int>(5,7) , false, true);
    test(SingleRange<int>(-3,5) >= SingleRange<int>(6,7) , false, false);
    test(SingleRange<int>(3,3) >= SingleRange<int>(-3,3) , true, true);
    test(SingleRange<int>(0,0) >= SingleRange<int>(0,0) , true, true);

    std::cout << "Testing != on ints" << std::endl;
    test(SingleRange<int>(5,6) != SingleRange<int>(3,4) , true, true);
    test(SingleRange<int>(5,6) != SingleRange<int>(3,5) , false, true);
    test(SingleRange<int>(5,6) != SingleRange<int>(5,6) , false, true);
    test(SingleRange<int>(3,5) != SingleRange<int>(5,7) , false, true);
    test(SingleRange<int>(3,5) != SingleRange<int>(6,7) , true, true);
    test(SingleRange<int>(3,3) != SingleRange<int>(3,3) , false, false);

    std::cout << "Testing ! on ints" << std::endl;
    test(!SingleRange<int>(0,0), true, true);
    test(!SingleRange<int>(0,1), false, true);
    test(!SingleRange<int>(-1,0), false, true);
    test(!SingleRange<int>(-1,1), false, true);
    test(!SingleRange<int>(1,1), false, false);
   
    std::cout << "Testing || on ints" << std::endl;
    test(SingleRange<int>(5,6) || SingleRange<int>(3,4) , true, true);
    test(SingleRange<int>(5,6) || SingleRange<int>(3,5) , true, true);
    test(SingleRange<int>(5,6) || SingleRange<int>(0,6) , true, true);
    test(SingleRange<int>(0,5) || SingleRange<int>(5,7) , true, true);
    test(SingleRange<int>(-3,5) || SingleRange<int>(6,7) , true, true);
    test(SingleRange<int>(3,3) || SingleRange<int>(-3,3) , true, true);
    test(SingleRange<int>(0,0) || SingleRange<int>(0,0) , false, false);
    test(SingleRange<int>(0,0) || SingleRange<int>(0,1) , false, true);
    test(SingleRange<int>(0,0) || SingleRange<int>(-1,1) , false, true);
    test(SingleRange<int>(0,0) || SingleRange<int>(3,5) , true, true);
    test(SingleRange<int>(-3,4) || SingleRange<int>(-10,5) , false, true);
    test(SingleRange<int>(-3,-1) || SingleRange<int>(-2,-3) , true, true);

    std::cout << "Testing && on ints" << std::endl;
    test(SingleRange<int>(5,6) && SingleRange<int>(3,4) , true, true);
    test(SingleRange<int>(5,6) && SingleRange<int>(3,5) , true, true);
    test(SingleRange<int>(5,6) && SingleRange<int>(0,6) , false, true);
    test(SingleRange<int>(0,5) && SingleRange<int>(5,7) , false, true);
    test(SingleRange<int>(-3,5) && SingleRange<int>(6,7) , false, true);
    test(SingleRange<int>(3,3) && SingleRange<int>(-3,3) , false, true);
    test(SingleRange<int>(0,0) && SingleRange<int>(0,0) , false, false);
    test(SingleRange<int>(0,0) && SingleRange<int>(0,1) , false, false);
    test(SingleRange<int>(0,0) && SingleRange<int>(-1,1) , false, false);
    test(SingleRange<int>(0,0) && SingleRange<int>(3,5) , false, false);
    test(SingleRange<int>(-3,4) && SingleRange<int>(-10,5) , false, true);
    test(SingleRange<int>(-3,-1) && SingleRange<int>(-2,-3) , true, true);

    std::cout << "Testing + on ints" << std::endl;
    test(SingleRange<int>(1,4) + SingleRange<int>(2,3), 3, 7);
    test(SingleRange<int>(1,4) + SingleRange<int>(-3,-2), -2, 2);
    test(SingleRange<int>(1,4) + SingleRange<int>(-3,2), -2, 6);
    test(SingleRange<int>(-1,4) + SingleRange<int>(-3,2), -4, 6);

    std::cout << "Testing - on ints" << std::endl;
    test(SingleRange<int>(1,4) - SingleRange<int>(2,3), -2, 2);
    test(SingleRange<int>(1,4) - SingleRange<int>(-3,-2), 3, 7);
    test(SingleRange<int>(1,4) - SingleRange<int>(-3,2), -1, 7);
    test(SingleRange<int>(-1,4) - SingleRange<int>(-3,2), -3, 7);

    std::cout << "Testing * on ints" << std::endl;
    test(SingleRange<int>(1,4) * SingleRange<int>(2,3), 2, 12);
    test(SingleRange<int>(1,4) * SingleRange<int>(-3,-2), -12, -2);
    test(SingleRange<int>(1,4) * SingleRange<int>(-3,2), -12, 8);
    test(SingleRange<int>(-1,4) * SingleRange<int>(-3,2), -12, 8);

    std::cout << "Testing / on ints" << std::endl;
    test(SingleRange<int>(4,8) / SingleRange<int>(1,2), 2, 8);
    test(SingleRange<int>(4,8) / SingleRange<int>(-2,-1), -8, -2);
    testerror(SingleRange<int>(1,4) / SingleRange<int>(-3,2) );
    test(SingleRange<int>(-2,4) / SingleRange<int>(2,1), -2,4 );
    test(SingleRange<int>(-4,-1) / SingleRange<int>(2,1), -4, 0 /* -1/2  as int */);
    test(SingleRange<int>(-4,-2) / SingleRange<int>(-1,-2), 1, 4);

    std::cout << "Testing %% on ints" << std::endl;
    test(SingleRange<int>(4,4) % SingleRange<int>(3,3), 1, 1);
    test(SingleRange<int>(2,2) % SingleRange<int>(3,3), 2, 2);
    test(SingleRange<int>(4,7) % SingleRange<int>(10,10), 4, 7);
    test(SingleRange<int>(14,17) % SingleRange<int>(10,10), 4, 7);
    test(SingleRange<int>(8,10) % SingleRange<int>(10,10), 0, 9);
   
    std::cout << "Testing +=, -= etc on ints" << std::endl;
    { SingleRange<int> x(4,5); x+= SingleRange<int>(2,2); test(x, 6, 7); }
    { SingleRange<int> x(4,5); x-= SingleRange<int>(2,2); test(x, 2, 3); }
    { SingleRange<int> x(4,5); x*= SingleRange<int>(2,2); test(x, 8, 10); }
    { SingleRange<int> x(4,8); x/= SingleRange<int>(2,2); test(x, 2, 4); }
    { SingleRange<int> x(4,8); x%= SingleRange<int>(10,20); test(x, 4, 8); } 

    std::cout << "Testing 'doif' (?:) on ints" << std::endl;
    test(SingleRange<int>(1,1).doif(SingleRange<int>(3,5), SingleRange<int>(1,4)), 3,5);
    test(SingleRange<int>(0,1).doif(SingleRange<int>(3,5), SingleRange<int>(1,4)), 1,5);
    test(SingleRange<int>(0,0).doif(SingleRange<int>(3,5), SingleRange<int>(1,4)), 1,4);
    
    


    std::cout << std::endl;
    std::cout << "Tests passed: " << passed << std::endl;
    std::cout << "Tests failed: " << failed << std::endl;
    
    
    return 0;
}
// vim:ts=4:et
