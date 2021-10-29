/*
    SPDX-FileCopyrightText: 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CLANGREFACTORING_H
#define CLANGREFACTORING_H

#include "clangprivateexport.h"

#include <language/codegen/basicrefactoring.h>

class TestRefactoring;

namespace KDevelop
{
class Declaration;
}

class KDEVCLANGPRIVATE_EXPORT ClangRefactoring : public KDevelop::BasicRefactoring
{
    Q_OBJECT

public:
    explicit ClangRefactoring(QObject* parent = nullptr);

    void fillContextMenu(KDevelop::ContextMenuExtension& extension, KDevelop::Context* context, QWidget* parent) override;

    QString moveIntoSource(const KDevelop::IndexedDeclaration& iDecl);

public Q_SLOTS:
    void executeMoveIntoSourceAction();

protected:
    KDevelop::DocumentChangeSet::ChangeResult applyChangesToDeclarations(const QString& oldName, const QString& newName, KDevelop::DocumentChangeSet& changes, const QList<KDevelop::IndexedDeclaration>& declarations) override;

private:
    friend TestRefactoring;

    bool validCandidateToMoveIntoSource(KDevelop::Declaration* decl);
};

#endif
