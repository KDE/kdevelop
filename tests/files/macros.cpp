/// "useCount" : 1, "useRanges" : "[(11, 4), (11, 4)]"
static int counter = 0; /// FIXME: There should be two uses. This seems to be a problem in the JSON test suite

#define INC(i) counter += i; counter += 4;

/// "useCount" : 1, "useRanges" : "[(11, 8), (11, 9)]"
int i = 0;

int main()
{
    /// "toString" : ""
    INC(i);
}
