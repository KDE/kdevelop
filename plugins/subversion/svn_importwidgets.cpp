/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svn_importwidgets.h"
#include "ui_svnimportdialog.h"

class SvnImportDialog::Private
{
public:
    Ui::SvnImportDialogWidget ui;
};

SvnImportDialog::SvnImportDialog( QWidget *parent )
    : KDialog( parent ), d( new Private )
{
    QWidget *widget = new QWidget( this );
    d->ui.setupUi( widget );
    
    setMainWidget( widget );
    setCaption( i18n("Subversion Import") );
    setButtons( KDialog::Ok | KDialog::Cancel );

    d->ui.srcUrlRequester->setMode( KFile::File | KFile::Directory );
    d->ui.destUrlRequester->setMode( KFile::File | KFile::Directory );
}

SvnImportDialog::~SvnImportDialog()
{
    delete d;
}

KUrl SvnImportDialog::srcLocal()
{
    return d->ui.srcUrlRequester->url();
}

KUrl SvnImportDialog::destRepository()
{
    return d->ui.destUrlRequester->url();
}

bool SvnImportDialog::nonRecurse()
{
    return d->ui.nonrecurseChk->isChecked();
}

bool SvnImportDialog::noIgnore()
{
    return d->ui.noignoreChk->isChecked();
}

#include "svn_importwidgets.moc"
