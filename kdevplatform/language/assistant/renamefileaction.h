/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_RENAMEFILEACTION_H
#define KDEVPLATFORM_RENAMEFILEACTION_H

#include <interfaces/iassistant.h>

#include <QUrl>

namespace KDevelop {
class BasicRefactoring;
}
class RenameFileActionPrivate;

class RenameFileAction
    : public KDevelop::IAssistantAction
{
    Q_OBJECT

public:
    RenameFileAction(KDevelop::BasicRefactoring* refactoring, const QUrl& file, const QString& newName);
    ~RenameFileAction() override;

    QString description() const override;
    void execute() override;

private:
    const QScopedPointer<class RenameFileActionPrivate> d_ptr;
    Q_DECLARE_PRIVATE(RenameFileAction)
};

#endif // KDEVPLATFORM_RENAMEFILEACTION_H
