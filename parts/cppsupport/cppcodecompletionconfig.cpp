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
{
}

void CppCodeCompletionConfig::init( )
{
    m_includeGlobalFunctions = DomUtil::readIntEntry( *m_dom, defaultPath + "/includeGlobalFunctions", false );
    m_includeTypes = DomUtil::readIntEntry( *m_dom, defaultPath + "/includeTypes", false );
    m_includeEnums = DomUtil::readIntEntry( *m_dom, defaultPath + "/includeEnums", false );
    m_includeTypedefs = DomUtil::readIntEntry( *m_dom, defaultPath + "/includeTypedefs", false );
    m_automaticCodeCompletion = DomUtil::readIntEntry( *m_dom, defaultPath + "/automaticCodeCompletion", true );
    m_automaticArgumentsHint = DomUtil::readIntEntry( *m_dom, defaultPath + "/automaticArgumentsHint", true );
    m_codeCompletionDelay  = DomUtil::readIntEntry( *m_dom, defaultPath + "/codeCompletionDelay", 250 );
    m_argumentsHintDelay = DomUtil::readIntEntry( *m_dom, defaultPath + "/argumentsHintDelay", 400 );
}

void CppCodeCompletionConfig::store( )
{
    DomUtil::readIntEntry( *m_dom, defaultPath + "/includeGlobalFunctions", m_includeGlobalFunctions );
    DomUtil::readIntEntry( *m_dom, defaultPath + "/includeTypes", m_includeTypes );
    DomUtil::readIntEntry( *m_dom, defaultPath + "/includeEnums", m_includeEnums );
    DomUtil::readIntEntry( *m_dom, defaultPath + "/includeTypedefs", m_includeTypedefs );
    DomUtil::readIntEntry( *m_dom, defaultPath + "/automaticCodeCompletion", m_automaticCodeCompletion );
    DomUtil::readIntEntry( *m_dom, defaultPath + "/automaticArgumentsHint", m_automaticArgumentsHint );
    DomUtil::readIntEntry( *m_dom, defaultPath + "/codeCompletionDelay", m_codeCompletionDelay );
    DomUtil::readIntEntry( *m_dom, defaultPath + "/argumentsHintDelay", m_argumentsHintDelay );
    
    emit stored();
}

void CppCodeCompletionConfig::setIncludeTypes( bool b )
{
    m_includeTypes = b;
}

void CppCodeCompletionConfig::setIncludeEnums( bool b )
{
    m_includeEnums = b;
}

void CppCodeCompletionConfig::setIncludeTypedefs( bool b )
{
    m_includeTypedefs = b;
}

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

void CppCodeCompletionConfig::setIncludeGlobalFunctions( bool b )
{
    m_includeGlobalFunctions = b;
}

