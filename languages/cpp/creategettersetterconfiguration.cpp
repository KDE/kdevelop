//
// C++ Implementation: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "creategettersetterconfiguration.h"

#include "cppsupportpart.h"

#include <domutil.h>

#include <qdom.h>
#include <kmessagebox.h>
QString CreateGetterSetterConfiguration::defaultPath = QString::fromLatin1( "/kdevcppsupport/creategettersetter" );

CreateGetterSetterConfiguration::CreateGetterSetterConfiguration( CppSupportPart * part ) : QObject(part), m_part(part), m_settings(0)
{
	init();
}


CreateGetterSetterConfiguration::~CreateGetterSetterConfiguration()
{
}

void CreateGetterSetterConfiguration::init( )
{
	m_settings = m_part->projectDom();
	if (m_settings == 0)
		return;
	
	m_prefixGet = DomUtil::readEntry( *m_settings, defaultPath + "/prefixGet", "" );
	m_prefixSet = DomUtil::readEntry( *m_settings, defaultPath + "/prefixSet", "set" );
	m_prefixVariable = QStringList::split(",", DomUtil::readEntry( *m_settings, defaultPath + "/prefixVariable", "m_,_" ));
	m_parameterName =  DomUtil::readEntry( *m_settings, defaultPath + "/parameterName", "theValue" );
	m_isInlineGet = DomUtil::readBoolEntry(*m_settings, defaultPath + "/inlineGet", true );
	m_isInlineSet = DomUtil::readBoolEntry(*m_settings, defaultPath + "/inlineSet", true );
}

void CreateGetterSetterConfiguration::store( )
{
	if (m_settings == 0)
		return;
	
	DomUtil::writeEntry( *m_settings, defaultPath + "/prefixGet", m_prefixGet );
	DomUtil::writeEntry( *m_settings, defaultPath + "/prefixSet", m_prefixSet );
	DomUtil::writeEntry( *m_settings, defaultPath + "/prefixVariable", m_prefixVariable.join(",") );
	DomUtil::writeEntry( *m_settings, defaultPath + "/parameterName", m_parameterName );
	DomUtil::writeBoolEntry(*m_settings, defaultPath + "/inlineGet", m_isInlineGet );
	DomUtil::writeBoolEntry(*m_settings, defaultPath + "/inlineSet", m_isInlineSet );
}
