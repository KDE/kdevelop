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
