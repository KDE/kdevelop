/***************************************************************************
 *   Copyright 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svn_revisionwidget.h"
#include <QRadioButton>
#include <knuminput.h>
#include <kdatetimewidget.h>
#include <kcombobox.h>

SvnRevisionWidget::SvnRevisionWidget( QWidget *parent )
    : QWidget(parent)
{
    setupUi( this );

    QList<SvnRevision::RevKeyword> keylist;
    keylist << SvnRevision::HEAD << SvnRevision::BASE << SvnRevision::WORKING;
    keylist << SvnRevision::PREV << SvnRevision::COMMITTED;
    installKeys( keylist );

    datetimeWidget->setDateTime( QDateTime::currentDateTime() );
}

SvnRevisionWidget::~SvnRevisionWidget()
{}

SvnRevision SvnRevisionWidget::revision()
{
    SvnRevision rev;

    if( revnumRadio->isChecked() ){
        rev.setNumber( numInput->value() );
    }
    else if( revkindRadio->isChecked() ){
        QVariant data = kindCombo->itemData( kindCombo->currentIndex() );
        SvnRevision::RevKeyword key = (SvnRevision::RevKeyword)(data.toInt());
        rev.setKey( key );
    }
    else if( revdateRadio->isChecked() ){
        rev.setDate( datetimeWidget->dateTime() );
    }

    return rev;
}

void SvnRevisionWidget::installKeys( const QList<SvnRevision::RevKeyword> &keylist )
{
    kindCombo->clear();
    foreach( SvnRevision::RevKeyword _key, keylist )
    {
        switch( _key ){
            case SvnRevision::HEAD:
                kindCombo->addItem( "HEAD", QVariant( (int)SvnRevision::HEAD ) );
            break;
            case SvnRevision::BASE:
                kindCombo->addItem( "BASE", QVariant( (int)SvnRevision::BASE ) );
            break;
            case SvnRevision::WORKING:
                kindCombo->addItem( "WORKING", QVariant( (int)SvnRevision::WORKING ) );
            break;
            case SvnRevision::PREV:
                kindCombo->addItem( "PREV", QVariant( (int)SvnRevision::PREV ) );
            break;
            case SvnRevision::COMMITTED:
                kindCombo->addItem( "COMMITTED", QVariant( (int)SvnRevision::COMMITTED ) );
            break;
        }
    }
}

void SvnRevisionWidget::setNumber( long revnum )
{
    numInput->setValue( revnum );
}

void SvnRevisionWidget::setKey( SvnRevision::RevKeyword keyword )
{
    int idx = kindCombo->findData( QVariant((int)keyword) );
    if( idx == -1 ) return;
    kindCombo->setCurrentIndex( idx );
}

// only enables specified type. disable all other type
void SvnRevisionWidget::enableType( SvnRevision::RevType type )
{
    switch( type ){
        case SvnRevision::number:
            revnumRadio->setChecked(true);
        break;
        case SvnRevision::kind:
            revkindRadio->setChecked(true);
        break;
        case SvnRevision::date:
            revdateRadio->setChecked(true);
        break;
    }
}

#include "svn_revisionwidget.moc"
