/*
 * file     : cppsupportwidget.cpp
 * begin    : 2001
 * copyright: (c) by daniel engelschalt
 * email    : daniel.engelschalt@gmx.net
 * license  : gpl version >= 2
 */

#include <qstring.h>
#include <keditcl.h>

#include "cppsupportwidget.h"
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
    setText( text );
}

#include "cppsupportwidget.moc"
