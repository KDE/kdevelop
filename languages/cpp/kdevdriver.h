
#ifndef __kdevdriver_h
#define __kdevdriver_h

#include "cppsupportpart.h"
#include "driver.h"

#include <kdevproject.h>
#include <kdeversion.h>

#include <cstdlib>
#include <unistd.h>
#include <qmap.h>

class KProcess;
namespace CppTools {
  class IncludePathResolver;
};

class KDevDriver: public Driver
{
public:
    ///When the driver is used in a background-thread, foreground MUST be false(else crash)
	KDevDriver( CppSupportPart* cppSupport, bool foreground = false );
    ~KDevDriver();
	CppSupportPart* cppSupport();
	void setupProject();
	//! setup the preprocessor
	//! code provided by Reginald Stadlbauer <reggie@trolltech.com>
	void setup();

	virtual void addMacro( const Macro& m )
	{
		if ( m.name() == "signals" || m.name() == "slots" )
			return ;
		Driver::addMacro( m );
	}

protected:
	void setupLexer( Lexer* lexer );

    ///Returns the complete include-path for that file. Not constant because it may add Problem-items to the driver. Must be absolute path.
    virtual QStringList getCustomIncludePath( const QString& file );

    virtual bool shouldParseIncludedFile( const ParsedFilePointer& /*file*/ );

private:
 
	CppSupportPart* m_cppSupport;
    CppTools::IncludePathResolver* m_includePathResolver;
    bool m_foreground;
    bool m_shouldParseIncludedFiles;
};

#endif
// kate: indent-mode csands; tab-width 4;


