//
//
// C++ Implementation: $MODULE$
//
// Description:
//
//
// Author: Roberto Raggi <robertol@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "cppcodecompletionconfig.h"
#include "cppsupportpart.h"

#include <domutil.h>

#include <kdebug.h>
#include <qdom.h>

QString CppCodeCompletionConfig::defaultPath = QString::fromLatin1( "/kdevcppsupport/codecompletion" );

CppCodeCompletionConfig::CppCodeCompletionConfig( CppSupportPart * part, QDomDocument* dom )
		: QObject( part ), m_part( part ), m_dom( dom )
{
	init();
}

CppCodeCompletionConfig::~CppCodeCompletionConfig()
{}

void CppCodeCompletionConfig::init( )
{
/*	m_includeGlobalFunctions = DomUtil::readBoolEntry( *m_dom, defaultPath + "/includeGlobalFunctions", true );
	m_includeTypes = DomUtil::readBoolEntry( *m_dom, defaultPath + "/includeTypes", true );
	m_includeEnums = DomUtil::readBoolEntry( *m_dom, defaultPath + "/includeEnums", true );
	m_includeTypedefs = DomUtil::readBoolEntry( *m_dom, defaultPath + "/includeTypedefs", false );*/
	m_automaticCodeCompletion = DomUtil::readBoolEntry( *m_dom, defaultPath + "/automaticCodeCompletion", false );
	m_automaticArgumentsHint = DomUtil::readBoolEntry( *m_dom, defaultPath + "/automaticArgumentsHint", true );
	m_automaticHeaderCompletion = DomUtil::readBoolEntry( *m_dom, defaultPath + "/automaticHeaderCompletion", true );
	m_codeCompletionDelay = DomUtil::readIntEntry( *m_dom, defaultPath + "/codeCompletionDelay", 250 );
	m_argumentsHintDelay = DomUtil::readIntEntry( *m_dom, defaultPath + "/argumentsHintDelay", 400 );
	m_headerCompletionDelay = DomUtil::readIntEntry( *m_dom, defaultPath + "/headerCompletionDelay", 250 );

	m_showOnlyAccessibleItems = DomUtil::readBoolEntry( *m_dom, defaultPath + "/showOnlyAccessibleItems", false );
	m_completionBoxItemOrder = (CompletionBoxItemOrder)DomUtil::readIntEntry( *m_dom, defaultPath + "/completionBoxItemOrder", ByAccessLevel );
	m_showEvaluationContextMenu = DomUtil::readBoolEntry( *m_dom, defaultPath + "/howEvaluationContextMenu", true );
	m_showCommentWithArgumentHint = DomUtil::readBoolEntry( *m_dom, defaultPath + "/showCommentWithArgumentHint", true );
	m_statusBarTypeEvaluation = DomUtil::readBoolEntry( *m_dom, defaultPath + "/statusBarTypeEvaluation", false );
	m_namespaceAliases = DomUtil::readEntry( *m_dom, defaultPath + "/namespaceAliases", "std=_GLIBCXX_STD;__gnu_cxx=std" );
	m_processPrimaryTypes = DomUtil::readBoolEntry( *m_dom, defaultPath + "/processPrimaryTypes", true );
	m_processFunctionArguments = DomUtil::readBoolEntry( *m_dom, defaultPath + "/processFunctionArguments", false );

        m_preProcessAllHeaders = DomUtil::readBoolEntry( *m_dom, defaultPath + "/preProcessAllHeaders", false );
        m_parseMissingHeaders = DomUtil::readBoolEntry( *m_dom, defaultPath + "/parseMissingHeaders", false );
        m_resolveIncludePaths = DomUtil::readBoolEntry( *m_dom, defaultPath + "/resolveIncludePaths", true );
        m_alwaysParseInBackground = DomUtil::readBoolEntry( *m_dom, defaultPath + "/alwaysParseInBackground", true );
        m_usePermanentCaching = DomUtil::readBoolEntry( *m_dom, defaultPath + "/usePermanentCaching", true );
        m_alwaysIncludeNamespaces = DomUtil::readBoolEntry( *m_dom, defaultPath + "/alwaysIncludeNamespaces", false );
        m_includePaths = DomUtil::readEntry( *m_dom, defaultPath + "/includePaths", ".;" );

        
}

void CppCodeCompletionConfig::store( )
{
/*	DomUtil::writeBoolEntry( *m_dom, defaultPath + "/includeGlobalFunctions", m_includeGlobalFunctions );
	DomUtil::writeBoolEntry( *m_dom, defaultPath + "/includeTypes", m_includeTypes );
	DomUtil::writeBoolEntry( *m_dom, defaultPath + "/includeEnums", m_includeEnums );
	DomUtil::writeBoolEntry( *m_dom, defaultPath + "/includeTypedefs", m_includeTypedefs );*/
	DomUtil::writeBoolEntry( *m_dom, defaultPath + "/automaticCodeCompletion", m_automaticCodeCompletion );
	DomUtil::writeBoolEntry( *m_dom, defaultPath + "/automaticArgumentsHint", m_automaticArgumentsHint );
	DomUtil::writeBoolEntry( *m_dom, defaultPath + "/automaticHeaderCompletion", m_automaticHeaderCompletion );
	DomUtil::writeIntEntry( *m_dom, defaultPath + "/codeCompletionDelay", m_codeCompletionDelay );
	DomUtil::writeIntEntry( *m_dom, defaultPath + "/argumentsHintDelay", m_argumentsHintDelay );
	DomUtil::writeIntEntry( *m_dom, defaultPath + "/headerCompletionDelay", m_headerCompletionDelay );
	
	DomUtil::writeBoolEntry( *m_dom, defaultPath + "/showOnlyAccessibleItems", m_showOnlyAccessibleItems );
	DomUtil::writeIntEntry( *m_dom, defaultPath + "/completionBoxItemOrder", m_completionBoxItemOrder );
	DomUtil::writeBoolEntry( *m_dom, defaultPath + "/howEvaluationContextMenu", m_showEvaluationContextMenu );
	DomUtil::writeBoolEntry( *m_dom, defaultPath + "/showCommentWithArgumentHint", m_showCommentWithArgumentHint );
	DomUtil::writeBoolEntry( *m_dom, defaultPath + "/statusBarTypeEvaluation", m_statusBarTypeEvaluation );
	DomUtil::writeEntry( *m_dom, defaultPath + "/namespaceAliases", m_namespaceAliases );
	DomUtil::writeBoolEntry( *m_dom, defaultPath + "/processPrimaryTypes", m_processPrimaryTypes );
	DomUtil::writeBoolEntry( *m_dom, defaultPath + "/processFunctionArguments", m_processFunctionArguments );
        DomUtil::writeBoolEntry( *m_dom, defaultPath + "/preProcessAllHeaders", m_preProcessAllHeaders );
        DomUtil::writeBoolEntry( *m_dom, defaultPath + "/parseMissingHeaders", m_parseMissingHeaders );
        DomUtil::writeBoolEntry( *m_dom, defaultPath + "/resolveIncludePaths", m_resolveIncludePaths );
        DomUtil::writeBoolEntry( *m_dom, defaultPath + "/alwaysParseInBackground", m_alwaysParseInBackground );
        DomUtil::writeBoolEntry( *m_dom, defaultPath + "/usePermanentCaching", m_usePermanentCaching );
        DomUtil::writeBoolEntry( *m_dom, defaultPath + "/alwaysIncludeNamespaces", m_alwaysIncludeNamespaces );
        DomUtil::writeEntry( *m_dom, defaultPath + "/includePaths", m_includePaths );

	emit stored();
}

// void CppCodeCompletionConfig::setIncludeTypes( bool b )
// {
// 	m_includeTypes = b;
// }
// 
// void CppCodeCompletionConfig::setIncludeEnums( bool b )
// {
// 	m_includeEnums = b;
// }
// 
// void CppCodeCompletionConfig::setIncludeTypedefs( bool b )
// {
// 	m_includeTypedefs = b;
// }

void CppCodeCompletionConfig::setAutomaticCodeCompletion( bool b )
{
	m_automaticCodeCompletion = b;
}

void CppCodeCompletionConfig::setAutomaticArgumentsHint( bool b )
{
	m_automaticArgumentsHint = b;
}

void CppCodeCompletionConfig::setCodeCompletionDelay( int delay )
{
	m_codeCompletionDelay = delay;
}

void CppCodeCompletionConfig::setArgumentsHintDelay( int delay )
{
	m_argumentsHintDelay = delay;
}

// void CppCodeCompletionConfig::setIncludeGlobalFunctions( bool b )
// {
// 	m_includeGlobalFunctions = b;
// }

void CppCodeCompletionConfig::setHeaderCompletionDelay( int delay )
{
	m_headerCompletionDelay = delay;
}

void CppCodeCompletionConfig::setAutomaticHeaderCompletion( bool b )
{
	m_automaticHeaderCompletion = b;
}

bool CppCodeCompletionConfig::showNamespaceAppearances() const {
	return true;
}

#include "cppcodecompletionconfig.moc"
