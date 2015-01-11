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
#include "ui_updateoptiondlg.h"
#include "svnrevision.h"
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
    d->ui.setupUi( mainWidget() );

    setCaption( i18n("Subversion Update") );
    setButtons( KDialog::Ok | KDialog::Cancel );

    QList<SvnRevision::Keyword> keys;
    keys << SvnRevision::Head;
    d->ui.revisionWidget->installKeys( keys );
    d->ui.revisionWidget->enableType( SvnRevision::Kind );
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





