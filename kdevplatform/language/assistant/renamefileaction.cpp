/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "renamefileaction.h"

#include <debug.h>

#include <language/codegen/documentchangeset.h>
#include <language/codegen/basicrefactoring.h>

#include <interfaces/idocumentcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/icore.h>
#include <sublime/message.h>
// KF
#include <KLocalizedString>

using namespace KDevelop;

class RenameFileActionPrivate
{
public:
    KDevelop::BasicRefactoring* m_refactoring;
    QUrl m_file;
    QString m_newName;
};

RenameFileAction::RenameFileAction(BasicRefactoring* refactoring, const QUrl& file, const QString& newName)
    : d_ptr(new RenameFileActionPrivate)
{
    Q_D(RenameFileAction);

    d->m_refactoring = refactoring;
    d->m_file = file;
    d->m_newName = newName;
}

RenameFileAction::~RenameFileAction()
{
}

QString RenameFileAction::description() const
{
    Q_D(const RenameFileAction);

    return i18n("Rename file from \"%1\" to \"%2\".",
                d->m_file.fileName(), d->m_refactoring->newFileName(d->m_file, d->m_newName));
}

void RenameFileAction::execute()
{
    Q_D(RenameFileAction);

    // save document to prevent unwanted dialogs
    IDocument* doc = ICore::self()->documentController()->documentForUrl(d->m_file);
    if (!doc) {
        qCWarning(LANGUAGE) << "could find no document for url:" << d->m_file;
        return;
    }

    if (!ICore::self()->documentController()->saveSomeDocuments({doc},
                                                                IDocumentController::SaveSelectionMode::DontAskUser)) {
        return;
    }

    // rename document
    DocumentChangeSet changes;
    DocumentChangeSet::ChangeResult result = d->m_refactoring->addRenameFileChanges(d->m_file, d->m_newName, &changes);
    if (result) {
        result = changes.applyAllChanges();
    }
    if (!result) {
        auto* message = new Sublime::Message(i18n("Failed to apply changes: %1", result.m_failureReason), Sublime::Message::Error);
        ICore::self()->uiController()->postMessage(message);
    }
    emit executed(this);
}

#include "moc_renamefileaction.cpp"
