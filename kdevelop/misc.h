// Whoa, this is really a hack to avoid a nameclash with
// the StringTokenizer class in khtml. 

#define StringTokenizer KDevStringTokenizer


class StringTokenizer
{
public:
    StringTokenizer();
    ~StringTokenizer();
 
    void tokenize( const char *, const char * );
    const char* nextToken();
    bool hasMoreTokens() { return ( pos != 0 ); }
 
protected:
    char *pos;
    char *end;
    char *buffer;
    int  bufLen;
};
