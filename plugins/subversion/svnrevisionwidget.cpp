/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svnrevisionwidget.h"
#include <QRadioButton>
#include <knuminput.h>
#include <kdatetimewidget.h>
#include <kcombobox.h>

SvnRevisionWidget::SvnRevisionWidget( QWidget *parent )
    : QWidget(parent)
{
    setupUi( this );

    QList<SvnRevision::Keyword> keylist;
    keylist << SvnRevision::Head << SvnRevision::Base << SvnRevision::Working;
    keylist << SvnRevision::Prev << SvnRevision::Committed;
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
        SvnRevision::Keyword key = (SvnRevision::Keyword)(data.toInt());
        rev.setKey( key );
    }
    else if( revdateRadio->isChecked() ){
        rev.setDate( datetimeWidget->dateTime() );
    }

    return rev;
}

void SvnRevisionWidget::installKeys( const QList<SvnRevision::Keyword> &keylist )
{
    kindCombo->clear();
    foreach( const SvnRevision::Keyword &_key, keylist )
    {
        switch( _key ){
            case SvnRevision::Head:
                kindCombo->addItem( "Head", QVariant( (int)SvnRevision::Head ) );
            break;
            case SvnRevision::Base:
                kindCombo->addItem( "Base", QVariant( (int)SvnRevision::Base ) );
            break;
            case SvnRevision::Working:
                kindCombo->addItem( "Working", QVariant( (int)SvnRevision::Working ) );
            break;
            case SvnRevision::Prev:
                kindCombo->addItem( "Prev", QVariant( (int)SvnRevision::Prev ) );
            break;
            case SvnRevision::Committed:
                kindCombo->addItem( "Committed", QVariant( (int)SvnRevision::Committed ) );
            break;
	    default:
            break;
        }
    }
}

void SvnRevisionWidget::setNumber( long revnum )
{
    numInput->setValue( revnum );
}

void SvnRevisionWidget::setKey( SvnRevision::Keyword keyword )
{
    int idx = kindCombo->findData( QVariant((int)keyword) );
    if( idx == -1 ) return;
    kindCombo->setCurrentIndex( idx );
}

// only enables specified type. disable all other type
void SvnRevisionWidget::enableType( SvnRevision::Type type )
{
    switch( type ){
        case SvnRevision::Number:
            revnumRadio->setChecked(true);
        break;
        case SvnRevision::Kind:
            revkindRadio->setChecked(true);
        break;
        case SvnRevision::Date:
            revdateRadio->setChecked(true);
        break;
    }
}

