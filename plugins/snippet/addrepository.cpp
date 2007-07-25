/***************************************************************************
 *   Copyright 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "addrepository.h"

#include <kfiledialog.h>

AddRepository::AddRepository(QWidget* parent)
 : QDialog(parent), Ui::AddRepositoryBase()
{
    Ui::AddRepositoryBase::setupUi(this);

    connect(toolButton, SIGNAL(clicked()),
            this, SLOT(slotOpenDir()));
}

AddRepository::~AddRepository()
{
}

void AddRepository::slotOpenDir()
{
    QString dir = KFileDialog::getExistingDirectory(KUrl("kfiledialog:///repository"), this);
    if (!dir.isEmpty())
        location->setText( dir );
}

#include "addrepository.moc"
// kate: space-indent on; indent-width 4; tab-width 4; indent-mode cstyle; replace-tabs on; auto-insert-doxygen on;
