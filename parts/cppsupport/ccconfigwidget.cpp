/***************************************************************************
 *   Copyright (C) 2001 by Daniel Engelschalt                              *
 *   daniel.engelschalt@gmx.net                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include "domutil.h"
#include "ccconfigwidget.h"
#include "cppsupportpart.h"

#include <iostream.h>

CCConfigWidget::CCConfigWidget( CppSupportPart* part, QWidget* parent, const char* name )
    : CCConfigWidgetBase( parent, name )
{
    m_pPart = part;

    QDomDocument dom = *m_pPart->projectDom();
    cbEnableCC->setChecked( DomUtil::readBoolEntry( dom, "/autoprojectpart/codecompletion/enablecc" ) );

    QDomElement element = dom.documentElement( );
    QDomElement apPart  = element.namedItem( "autoprojectpart" ).toElement( );
    QDomElement ccPart  = apPart.namedItem( "codecompletion" ).toElement( );
    QDomElement chPart  = ccPart.namedItem( "codehinting" ).toElement( );
    
    if( !chPart.isNull( ) ){
	cbEnableCH->setChecked( chPart.attribute( "enablech" ).toInt( ) );
	rbSelectView->setChecked( chPart.attribute( "selectview" ).toInt( ) );
	rbOutputView->setChecked( chPart.attribute( "outputview").toInt( ) );
    }

    if( !cbEnableCH->isChecked( ) )
	bgCodeHinting->setEnabled( false );
    
    // for setting a default value
    if( rbSelectView->isChecked( ) == false )
	rbOutputView->setChecked( true );
}


CCConfigWidget::~CCConfigWidget( )
{
}

void
CCConfigWidget::accept( )
{
    QDomDocument dom = *m_pPart->projectDom();
    QDomElement  element = dom.documentElement( );

    QDomElement apPart  = element.namedItem( "autoprojectpart" ).toElement( );
    if( apPart.isNull( ) ){
	apPart = dom.createElement( "autoprojectpart" );
	element.appendChild( apPart );
    }

    QDomElement codecompletion = apPart.namedItem( "codecompletion" ).toElement( );
    if( codecompletion.isNull( ) ){
	codecompletion = dom.createElement( "codecompletion" );
	apPart.appendChild( codecompletion );
    }

    DomUtil::writeBoolEntry( dom, "/autoprojectpart/codecompletion/enablecc", cbEnableCC->isChecked( ) );

    QDomElement codehinting = codecompletion.namedItem( "codehinting" ).toElement( );
    if( codehinting.isNull( ) ){
	codehinting = dom.createElement( "codehinting" );
	codecompletion.appendChild( codehinting );
    }

    codehinting.setAttribute( "enablech"  , cbEnableCH->isChecked( ) );
    codehinting.setAttribute( "selectview", rbSelectView->isChecked( ) );
    codehinting.setAttribute( "outputview", rbOutputView->isChecked( ) );

    m_pPart->setEnableCC( cbEnableCC->isChecked( ) );
    emit enableCodeHinting( cbEnableCH->isChecked( ), rbOutputView->isChecked( ) );    
}

void
CCConfigWidget::slotEnableCH( )
{
    if( cbEnableCH->isChecked( ) )
	bgCodeHinting->setEnabled( true );
    else
	bgCodeHinting->setEnabled( false );
}

#include "ccconfigwidget.moc"
