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

#include "svn_updatewidget.h"
#include "ui_uiupdate_option_dlg.h"
#include "svn_revision.h"
#include <kurl.h>
#include <QRadioButton>
#include <QComboBox>
#include <QCheckBox>
#include <knuminput.h>
#include <kdatetimewidget.h>
#include <QDateTime>

class SvnUpdateOptionDlg::Private
{
public:
    Ui::SvnUpdateOptionWidget ui;
};

SvnUpdateOptionDlg::SvnUpdateOptionDlg( QWidget *parent )
    : KDialog( parent ), d( new Private )
{
    QWidget *widget = new QWidget( this );
    d->ui.setupUi( widget );

    setMainWidget( widget );
    setCaption( i18n("Subversion Update") );
    setButtons( KDialog::Ok | KDialog::Cancel );

    connect( d->ui.numRadio, SIGNAL(toggled(bool)), d->ui.intInput,  SLOT(setEnabled(bool)) );
    connect( d->ui.numRadio, SIGNAL(toggled(bool)), d->ui.comboBox, SLOT(setDisabled(bool)) );
    connect( d->ui.numRadio, SIGNAL(toggled(bool)), d->ui.dateTimeWidget, SLOT(setDisabled(bool)) );

    connect( d->ui.keywordRadio, SIGNAL(toggled(bool)), d->ui.comboBox,  SLOT(setEnabled(bool)) );
    connect( d->ui.keywordRadio, SIGNAL(toggled(bool)), d->ui.intInput, SLOT(setDisabled(bool)) );
    connect( d->ui.keywordRadio, SIGNAL(toggled(bool)), d->ui.dateTimeWidget, SLOT(setDisabled(bool)) );

    connect( d->ui.datetimeRadio, SIGNAL(toggled(bool)), d->ui.dateTimeWidget,  SLOT(setEnabled(bool)) );
    connect( d->ui.datetimeRadio, SIGNAL(toggled(bool)), d->ui.comboBox, SLOT(setDisabled(bool)) );
    connect( d->ui.datetimeRadio, SIGNAL(toggled(bool)), d->ui.intInput, SLOT(setDisabled(bool)) );

    QDateTime dateTime = QDateTime::currentDateTime();
    d->ui.dateTimeWidget->setDateTime( dateTime );
}

SvnUpdateOptionDlg::~SvnUpdateOptionDlg()
{
    delete d;
}

SvnRevision SvnUpdateOptionDlg::revision()
{
    SvnRevision rev;
    if( d->ui.numRadio->isChecked() ){
        rev.setNumber( d->ui.intInput->value() );
        return rev;
    }
    else if( d->ui.keywordRadio->isChecked() ){
        // note. If you add more keywords in .ui, you should update below.
        rev.setKey( SvnRevision::HEAD );
        return rev;
    }
    else if( d->ui.datetimeRadio->isChecked() ){
        rev.setDate( d->ui.dateTimeWidget->dateTime() );
        return rev;
    }
    else{
        // should not reach here
        return rev;
    }

}

bool SvnUpdateOptionDlg::recurse()
{
    return !( d->ui.nonrecurseChk->isChecked() );
}
bool SvnUpdateOptionDlg::ignoreExternal()
{
    return d->ui.ignoreexternalChk->isChecked();
}


#include "svn_updatewidget.moc"



