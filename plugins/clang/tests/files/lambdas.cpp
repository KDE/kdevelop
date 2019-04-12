/// "useCount" : 1
void foo()
{
    /// "useCount": 2
    auto l1 = []() {
        foo();
    };
    /// "useCount": 1
    auto l2 = [l1] {
        l1();
    };
    /// "useCount": 0
    auto l3 = [lambda = l2]() {
        lambda();
    };
}
