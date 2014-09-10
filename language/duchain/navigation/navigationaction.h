/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_NAVIGATIONACTION_H
#define KDEVPLATFORM_NAVIGATIONACTION_H

#include <QUrl>

#include <ktexteditor/cursor.h>

#include "../duchainpointer.h"

namespace KDevelop {

class AbstractNavigationContext;

struct NavigationAction {
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
  NavigationAction(QString _key) : targetContext(0), type(ExecuteKey), key(_key) {
  }

  NavigationAction() : targetContext(0), type(None) {
  }

  NavigationAction( DeclarationPointer decl_, Type type_ ) : targetContext(0), decl(decl_), type(type_) {
  }

  NavigationAction( const QUrl& _document, const KTextEditor::Cursor& _cursor) : targetContext(0), document(_document), cursor(_cursor) {
    type = JumpToSource;
  }

  NavigationAction(AbstractNavigationContext* _targetContext) : targetContext(_targetContext) {
  }

  AbstractNavigationContext* targetContext; //If this is set, this action does nothing else than jumping to that context

  DeclarationPointer decl;
  Type type;

  QUrl document;
  KTextEditor::Cursor cursor;
  QString key;
};

}

#endif
