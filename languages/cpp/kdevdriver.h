
#ifndef __kdevdriver_h
#define __kdevdriver_h

#include "cppsupportpart.h"
#include "driver.h"

#include <kdevproject.h>
#include <kdeversion.h>

#include <qprocess.h>

#include <cstdlib>
#include <unistd.h>

class KProcess;

class KDevDriver: public Driver
{
public:
	KDevDriver( CppSupportPart* cppSupport );
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
  
private:
	CppSupportPart* m_cppSupport;
};

#endif 
// kate: indent-mode csands; tab-width 4;


