/// "useCount" : 1, "useRanges" : "[(21, 4), (21, 4)]"
static int counter = 0; /// FIXME: There should be two uses. This seems to be a problem in the JSON test suite

#define INC(i) counter += i; counter += 4;

/// "useCount" : 1, "useRanges" : "[(21, 8), (21, 9)]"
int i = 0;

#define Q_DECL_CONSTEXPR constexpr

class Cursor
{
public:
    /// "useCount" : 1, "useRanges" : "[(24, 6), (24, 12)]"
    Q_DECL_CONSTEXPR int column() const {
        return 123;
    }
};

int main()
{
    INC(i);

    Cursor c;
    c.column();
}
