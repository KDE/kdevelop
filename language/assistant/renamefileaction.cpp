/*
  Copyright 2012 Milian Wolff <mail@milianw.de>
  Copyright 2014 Kevin Funk <kfunk@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "renamefileaction.h"

#include <language/codegen/documentchangeset.h>
#include <language/codegen/basicrefactoring.h>

#include <interfaces/idocumentcontroller.h>
#include <interfaces/icore.h>

#include <KLocalizedString>
#include <KMessageBox>

using namespace KDevelop;

struct RenameFileAction::Private
{
    KDevelop::BasicRefactoring* m_refactoring;
    QUrl m_file;
    QString m_newName;
};

RenameFileAction::RenameFileAction(BasicRefactoring* refactoring, const QUrl& file, const QString& newName)
    : d(new Private)
{
    d->m_refactoring = refactoring;
    d->m_file = file;
    d->m_newName = newName;
}

RenameFileAction::~RenameFileAction()
{
}

QString RenameFileAction::description() const
{
    return i18n("Rename file from \"%1\" to \"%2\".",
                d->m_file.fileName(), d->m_refactoring->newFileName(d->m_file, d->m_newName));
}

void RenameFileAction::execute()
{
    // save document to prevent unwanted dialogs
    IDocument* doc = ICore::self()->documentController()->documentForUrl(d->m_file);
    if (!doc) {
        qWarning() << "could find no document for url:" << d->m_file;
        return;
    }

    if (!ICore::self()->documentController()->saveSomeDocuments(QList<IDocument*>() << doc, IDocument::Silent)) {
        return;
    }

    // rename document
    DocumentChangeSet changes;
    DocumentChangeSet::ChangeResult result = d->m_refactoring->addRenameFileChanges(d->m_file, d->m_newName, &changes);
    if (result) {
        result = changes.applyAllChanges();
    }
    if(!result) {
        KMessageBox::error(0, i18n("Failed to apply changes: %1", result.m_failureReason));
    }
    emit executed(this);
}
