#ifndef simpleparser_h
#define simpleparser_h

#include <qstring.h>
#include <qvaluelist.h>

class CParsedAttribute;

class SimpleVariable{
public:
    SimpleVariable()
        : scope( 0 ), name( QString::null ), type( QString::null ){}
    SimpleVariable( const SimpleVariable& source )
        : scope( source.scope ), name( source.name ), type( source.type ) {}
    ~SimpleVariable(){}

    SimpleVariable& operator = ( const SimpleVariable& source ){
        scope = source.scope;
        name = source.name;
        type = source.type;
        return *this;
    }

    int scope;
    QString name;
    QString type;
};

class SimpleParser{
public:
    static QValueList<SimpleVariable> localVariables( QString contents );
    static QValueList<SimpleVariable> parseFile( const QString& filename );
};

#endif
