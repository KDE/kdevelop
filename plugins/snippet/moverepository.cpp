/***************************************************************************
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
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
: KDialog(parent), Ui::MoveRepositoryBase(), repo_(repo)
{
    Ui::MoveRepositoryBase::setupUi(this);

    connect(toolButton, SIGNAL(clicked()),
        this, SLOT(slotOpenDir()));

    name->setText( repo->text() );
    location->setText( repo->getLocation() );

    if ( repo->QStandardItem::parent() ) {
        // If it's a subrepo we don't allow to change the location.
        // Only changing the name is allowed.
        location->setEnabled( false );
    }
}

MoveRepository::~MoveRepository()
{
}

void MoveRepository::slotOpenDir()
{
    QString dir = KFileDialog::getExistingDirectory(KUrl("kfiledialog:///repository"), this);
    if (!dir.isEmpty())
        location->setText( dir );
}

void MoveRepository::accept()
{
    if ( name->text().isEmpty() || location->text().isEmpty() ) {
        KMessageBox::error( this, i18n("Empty name"), i18n("Please enter a name and a location") );
        return;
    }

    QString newLocation;
    if ( repo_->QStandardItem::parent() ) {
        QDir dir( location->text() );
        dir.cdUp();
        newLocation = dir.path() + QDir::separator() + name->text();
    } else {
        newLocation = location->text();
    }

    repo_->changeLocation( newLocation, name->text() );

    // Call baseclass' accept() method
    QDialog::accept();
}

#include "moverepository.moc"
// kate: space-indent on; indent-width 4; tab-width 4; indent-mode cstyle; replace-tabs on; auto-insert-doxygen on;
