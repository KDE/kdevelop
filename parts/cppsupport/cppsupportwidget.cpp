/*
 * file     : cppsupportwidget.cpp
 * begin    : 2001
 * copyright: (c) by daniel engelschalt
 * email    : daniel.engelschalt@gmx.net
 * license  : gpl version >= 2
 */

#include "cppsupportwidget.h"

#include <qstring.h>

#include <keditcl.h>
#include <klocale.h>

#include "cppsupportpart.h"

#include <iostream>
using namespace std;

CppSupportWidget::CppSupportWidget( CppSupportPart* part )
    : KEdit( 0, "CppSupportPart" )
{
    m_part = part;
    setReadOnly( true );
}

CppSupportWidget::~CppSupportWidget( )
{
}

void
CppSupportWidget::setCHText( const QString& text )
{
    clear( );
    if( text.isEmpty( ) )
	setText( i18n( "No code hinting available for this method !" ) );
    else
	setText( text );
}

#include "cppsupportwidget.moc"
