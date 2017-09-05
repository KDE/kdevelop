
class Operator
{
public:
    /// "useRanges" : [ "[(26, 6), (26, 7)]" ]
    int operator[](int);
    /// "useRanges" : [ "[(27, 7), (27, 8)]" ]
    operator bool();
    /// "useRanges" : [ "[(28, 7), (28, 8)]" ]
    int operator<(Operator&);
    /// "useRanges" : [ "[(29, 10), (29, 11)]" ]
    operator int();
    /// "useRanges" : [ "[(30, 6), (30, 8)]" ]
    Operator* operator->();
    /// "useRanges" : [ "[(31, 7), (31, 9)]" ]
    Operator* operator>>(Operator*);
    /// "useRanges" : [ "[(32, 7), (32, 9)]" ]
    Operator* operator==(Operator);
    /// "useRanges" : [ "[(30, 8), (30, 9)]" ]
    ~Operator() {};
};


int main()
{
    Operator op, op2;
    op[1234];
    if(!op){};
    op < op2;
    int my(op);
    op->~Operator();
    op >> &op;
    op == op2;

    return my;
}
