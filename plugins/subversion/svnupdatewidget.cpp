/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svnupdatewidget.h"
#include "ui_uiupdateoptiondlg.h"
#include "svnrevision.h"
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

    QList<SvnRevision::RevKeyword> keys;
    keys << SvnRevision::HEAD;
    d->ui.revisionWidget->installKeys( keys );
    d->ui.revisionWidget->enableType( SvnRevision::kind );
}

SvnUpdateOptionDlg::~SvnUpdateOptionDlg()
{
    delete d;
}

SvnRevision SvnUpdateOptionDlg::revision()
{
    return d->ui.revisionWidget->revision();
}

bool SvnUpdateOptionDlg::recurse()
{
    return !( d->ui.nonrecurseChk->isChecked() );
}
bool SvnUpdateOptionDlg::ignoreExternal()
{
    return d->ui.ignoreexternalChk->isChecked();
}


#include "svnupdatewidget.moc"



