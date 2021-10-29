/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_NAVIGATIONACTION_H
#define KDEVPLATFORM_NAVIGATIONACTION_H

#include <QUrl>

#include "../duchainpointer.h"

#include <KTextEditor/Cursor>

namespace KDevelop {
class AbstractNavigationContext;

struct NavigationAction
{
    enum Type {
        None,
        NavigateDeclaration,
        NavigateUses,
        ShowUses,
        JumpToSource, //If this is set, the action jumps to document and cursor if they are valid, else to the declaration-position of decl
        ExecuteKey, //This is used to do changes within one single navigation-context. executeKey(key) will be called in the current context,
                    //and the context has the chance to react in an arbitrary way.
        ShowDocumentation
    };

    ///When executed, this navigation-action calls the "executeKeyAction(QString) function in its navigation-context
    explicit NavigationAction(const QString& _key) : type(ExecuteKey)
        , key(_key)
    {
    }

    NavigationAction()
    {
    }

    NavigationAction(const DeclarationPointer& decl_, Type type_) :  decl(decl_)
        , type(type_)
    {
    }

    NavigationAction(const QUrl& _document, const KTextEditor::Cursor& _cursor) :  type(JumpToSource)
        , document(_document)
        , cursor(_cursor)
    {
    }

    explicit NavigationAction(AbstractNavigationContext* _targetContext) : targetContext(_targetContext)
    {
    }

    AbstractNavigationContext* targetContext = nullptr; //If this is set, this action does nothing else than jumping to that context

    DeclarationPointer decl;
    Type type = None;

    QUrl document;
    KTextEditor::Cursor cursor;
    QString key;
};
}

Q_DECLARE_TYPEINFO(KDevelop::NavigationAction, Q_MOVABLE_TYPE);

#endif
