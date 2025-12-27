constexpr bool ret_true() { return true; }
constexpr bool ret_false() { return false; }

struct foo
{
    /// "isExplicit" : false,
    /// "isDefinition": true
    foo() {}
    /// "isDefinition": true
    ~foo() {}
    /// "isDefinition": true
    void blub() {}
};

struct bar
{
    /// "isExplicit" : true,
    /// "isDefinition": false
    explicit bar();
    /// "isDefinition": false
    ~bar();
    /// "isDefinition": false
    void blub();
};

class des
{
public:
    /// "isExplicit" : true,
    /// "isDefinition" : false
    explicit(ret_true()) des(int a);

    /// "isVirtual" : true,
    /// "isDefinition": false
    virtual ~des();

    /// "isAbstract" : true,
    /// "isVirtual" : true,
    /// "isExplicit" : true,
    /// "isDefinition" : false
    virtual explicit operator bool() const = 0;

    /// "isExplicit" : false
    explicit(ret_false()) operator int();
};

des::~des()
{
}
