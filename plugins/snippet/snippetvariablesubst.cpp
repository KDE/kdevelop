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

#include "snippetvariablesubst.h"

#include <QHeaderView>

#include "snippetvariablemodel.h"

SnippetVariables::SnippetVariables(SnippetVariableModel* model, QWidget *parent)
    : QDialog(parent), Ui::SnippetVariableSubstBase()
{
    Ui::SnippetVariableSubstBase::setupUi(this);

    tableView->setModel( model );
    tableView->verticalHeader()->hide();
}

#include "snippetvariablesubst.moc"

// kate: space-indent on; indent-width 4; indent-mode cstyle; replace-tabs on; auto-insert-doxygen on;
