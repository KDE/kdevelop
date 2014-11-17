/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2014 Michael Ferris <mike.ferrisb@gmail.com>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "simplerefactoring.h"

#include <QAction>
#include <QIcon>

#include <interfaces/contextmenuextension.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/interfaces/codecontext.h>

using namespace KDevelop;

SimpleRefactoring::SimpleRefactoring(QObject *parent)
    : BasicRefactoring(parent)
{
    /* There's nothing to do here. */
}

void SimpleRefactoring::fillContextMenu(ContextMenuExtension& extension, Context* context)
{
    if (DeclarationContext* declContext = dynamic_cast<DeclarationContext*>(context)){
        //Actions on declarations
        qRegisterMetaType<IndexedDeclaration>();

        DUChainReadLocker lock;

        Declaration* declaration = declContext->declaration().data();
        if (declaration) {
            QFileInfo fileInfo(declaration->topContext()->url().str());
            if (fileInfo.isWritable()) {
                QAction* action = new QAction(i18n("Rename %1", declaration->qualifiedIdentifier().toString()), this);
                action->setData(QVariant::fromValue(IndexedDeclaration(declaration)));
                action->setIcon(QIcon::fromTheme("edit-rename"));
                connect(action, SIGNAL(triggered(bool)), this, SLOT(executeRenameAction()));

                extension.addAction(ContextMenuExtension::RefactorGroup, action);
            }
        }
    }
}

#include "moc_simplerefactoring.cpp"
