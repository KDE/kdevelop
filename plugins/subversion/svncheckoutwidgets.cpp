/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svncheckoutwidgets.h"
#include "svnrevision.h"

#include <kurl.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kdatetimewidget.h>
#include <knuminput.h>
#include <kcombobox.h>
#include <QRadioButton>
#include <QCheckBox>

SvnCheckoutDialog::SvnCheckoutDialog( const KUrl &destDir, QWidget *parent )
    : KDialog( parent )
{
    ui.setupUi( mainWidget() );
    setButtons( KDialog::Ok | KDialog::Cancel );
    setWindowTitle( "Subversion Checkout" );

    ui.revDate->setDateTime( QDateTime::currentDateTime() );
    ui.pathRequester->setUrl( destDir );
    ui.pathRequester->setMode( KFile::File | KFile::Directory );
}

SvnCheckoutDialog::~SvnCheckoutDialog()
{
}

KUrl SvnCheckoutDialog::reposUrl()
{
    return KUrl(ui.repositoryEdit->text());
}

KUrl SvnCheckoutDialog::destPath()
{
    return ui.pathRequester->url();
}

SvnRevision SvnCheckoutDialog::revision()
{
    SvnRevision ret;

    if( ui.revnumRadio->isChecked() ){
        ret.setNumber( ui.revNum->value() );
    }
    else if( ui.revkeyRadio->isChecked() ){
        ret.setKey( SvnRevision::Head );
    }
    else if( ui.revdateRadio->isChecked() ){
        ret.setDate( ui.revDate->dateTime() );
    }

    return ret;
}

bool SvnCheckoutDialog::recurse()
{
    if( ui.nonrecurseChk->isChecked() ){
        return false;
    }
    return true;
}

bool SvnCheckoutDialog::ignoreExternals()
{
    if( ui.ignoreExternalChk->isChecked() ){
        return true;
    }
    return false;
}

#include "svncheckoutwidgets.moc"

