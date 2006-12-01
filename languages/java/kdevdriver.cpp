
#include "kdevdriver.h"
#include "JavaLexer.hpp"
#include <unistd.h>
#include <qfileinfo.h>

KDevDriver::KDevDriver( JavaSupportPart* javaSupport )
	: m_javaSupport( javaSupport )
{
}

JavaSupportPart* KDevDriver::javaSupport()
{
	return m_javaSupport;
}

void KDevDriver::setupProject()
{
	QMap<QString, bool> map;

	{
	QStringList fileList = m_javaSupport->project()->allFiles();
	QStringList::ConstIterator it = fileList.begin();
	while( it != fileList.end() ){
		QFileInfo info( *it );
		++it;

		map.insert( info.dirPath(true), true );
	}
	}

	{
	QMap<QString, bool>::Iterator it = map.begin();
	while( it != map.end() ){
		addIncludePath( it.key() );
		++it;
	}
	}
}

void KDevDriver::setupLexer( JavaLexer* lexer )
{
	Driver::setupLexer( lexer );
}
