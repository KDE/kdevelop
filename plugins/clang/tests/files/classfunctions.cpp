constexpr bool ret_true() { return true; }
constexpr bool ret_false() { return false; }

struct foo
{
    /// "isNoexcept" : false,
    /// "isExplicit" : false,
    /// "isDefinition": true
    foo() {}
    /// "isNoexcept" : true,
    /// "isDefinition": true
    ~foo() {}
    /// "isNoexcept" : true,
    /// "isDefinition": true
    void blub() throw() {}
    /// "isNoexcept" : true,
    /// "isDefinition": true
    void blah() noexcept {}
};

struct bar
{
    /// "isNoexcept" : false,
    /// "isExplicit" : true,
    /// "isDefinition": false
    explicit bar();
    /// "isNoexcept" : true,
    /// "EXPECT_FAIL": { "isNoexcept" : "libclang wrongly reports a destructor without definition as not noexcept" },
    /// "isDefinition": false
    ~bar();
    /// "isNoexcept" : false,
    /// "isDefinition": false
    void blub() noexcept(ret_false());
    /// "isNoexcept" : true,
    /// "EXPECT_FAIL": { "isNoexcept" : "Computed noexcept doesn't work, see a canThrow(...) comment in clangutils.cpp" },
    /// "isDefinition": false
    void blah() noexcept(true);
};

class des
{
public:
    /// "isNoexcept" : false,
    /// "isExplicit" : true,
    /// "isDefinition" : false
    explicit(ret_true()) des(int a);

    /// "isNoexcept" : true,
    /// "isVirtual" : true,
    /// "isDefinition": false
    virtual ~des();

    /// "isNoexcept" : true,
    /// "isAbstract" : true,
    /// "isVirtual" : true,
    /// "isExplicit" : true,
    /// "isDefinition" : false
    virtual explicit operator bool() const noexcept = 0;

    /// "isNoexcept" : false,
    /// "isExplicit" : false
    explicit(ret_false()) operator int();
};

des::~des()
{
}
