/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "moverepository.h"

#include <kfiledialog.h>
#include <kmessagebox.h>

#include "snippetrepository.h"

MoveRepository::MoveRepository(SnippetRepository* repo, QWidget* parent)
: KDialog(parent), Ui::AddRepository(), repo_(repo)
{
    Ui::AddRepository::setupUi(mainWidget());
    setCaption(i18n("Move Repository"));
    setButtons(KDialog::Ok | KDialog::Cancel);
    setButtonText(KDialog::Ok, i18n("Move Repository"));
    location->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
    name->setText( repo->text() );
    location->setUrl( repo->getLocation() );


    if ( repo->QStandardItem::parent() ) {
        // If it's a subrepo we don't allow to change the location.
        // Only changing the name is allowed.
        location->setEnabled( false );
    }
}

MoveRepository::~MoveRepository()
{
}

void MoveRepository::accept()
{
    if ( name->text().isEmpty() || location->url().isEmpty() ) {
        KMessageBox::error( this, i18n("Empty name or location"),
                            i18n("Please enter a name and a location") );
        return;
    }

    QString newLocation;
    if ( repo_->QStandardItem::parent() ) {
        QDir dir( location->url().path() );
        dir.cdUp();
        newLocation = dir.path() + QDir::separator() + name->text();
    } else {
        newLocation = location->url().path();
    }

    repo_->changeLocation( newLocation, name->text() );

    // Call baseclass' accept() method
    KDialog::accept();
}

#include "moverepository.moc"
