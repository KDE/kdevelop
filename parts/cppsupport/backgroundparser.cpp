
#include "backgroundparser.h"
#include "lexer.h"
#include "parser.h"
#include "driver.h"
#include "problemreporter.h"
#include <kdebug.h>

BackgroundParser::BackgroundParser( ProblemReporter* reporter,
                                    const QString& source,
                                    const QString& filename )
    : m_reporter( reporter ),
      m_source( source.unicode(), source.length() ),
      m_fileName( filename.unicode(), filename.length() )
{
}

BackgroundParser::~BackgroundParser()
{
}

void BackgroundParser::run()
{
    using namespace CppSupport;

    Driver driver;
    Lexer lexer;
    // stl
    lexer.addSkipWord( "__STL_BEGIN_NAMESPACE" );
    lexer.addSkipWord( "__STL_END_NAMESPACE" );
    lexer.addSkipWord( "__STL_NULL_TMPL_ARGS" );
    lexer.addSkipWord( "__GC_CONST" );
    lexer.addSkipWord( "__STL_TEMPLATE_NULL" );

    // qt
    lexer.addSkipWord( "Q_TEMPLATE_EXTERN" );
    lexer.addSkipWord( "Q_EXPORT" );
    lexer.addSkipWord( "QM_EXPORT_NETWORK" );
    lexer.addSkipWord( "QMAC_PASCAL" );
    lexer.addSkipWord( "QT_STATIC_CONST" );
    lexer.addSkipWord( "Q_OVERRIDE", CppSupport::SkipWordAndArguments );
    lexer.addSkipWord( "Q_ENUMS", CppSupport::SkipWordAndArguments );
    lexer.addSkipWord( "Q_PROPERTY", CppSupport::SkipWordAndArguments );
    lexer.addSkipWord( "Q_INLINE_TEMPLATES" );
    lexer.addSkipWord( "Q_TYPENAME" );

    // kde
    lexer.addSkipWord( "K_EXPORT_COMPONENT_FACTORY", CppSupport::SkipWordAndArguments );

    lexer.setSource( m_source );
    Parser parser( m_reporter, &driver,  &lexer );
    parser.setFileName( m_fileName );

    parser.parseTranslationUnit();
}



