/*
 * KDevelop C++ Code Completion Support
 *
 * Copyright 2006 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVCPPCODECOMPLETIONMODEL_H
#define KDEVCPPCODECOMPLETIONMODEL_H

#include <QPair>
#include <QMap>
#include <QPointer>
#include <ktexteditor/codecompletionmodel.h>
#include <ksharedptr.h>
#include <duchainpointer.h>
#include "codecompletioncontext.h"
#include "includeitem.h"

class QIcon;
class QString;

namespace KDevelop
{
class DUContext;
class Declaration;
}

namespace Cpp {
  class CodeCompletionContext;
  class NavigationWidget;
}

class CppCodeCompletionModel : public KTextEditor::CodeCompletionModel
{
  Q_OBJECT

  public:
    CppCodeCompletionModel(QObject* parent);
    virtual ~CppCodeCompletionModel();

    void setContext(KDevelop::DUContextPointer context, const KTextEditor::Cursor& position, KTextEditor::View* view);

    virtual void completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType invocationType);

    virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;

    ///getData(..) for include-file completion
    QVariant getIncludeData(const QModelIndex& index, int role) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;

  private:
    KDevelop::DUContextPointer m_context;
    KSharedPtr<Cpp::CodeCompletionContext> m_completionContext;
    typedef QPair<KDevelop::DeclarationPointer, KSharedPtr<Cpp::CodeCompletionContext> > DeclarationContextPair;


    struct CompletionItem {
      CompletionItem(KDevelop::DeclarationPointer decl = KDevelop::DeclarationPointer(), KSharedPtr<Cpp::CodeCompletionContext> context=KSharedPtr<Cpp::CodeCompletionContext>(), int _inheritanceDepth = 0, int _listOffset=0) : declaration(decl), completionContext(context), inheritanceDepth(_inheritanceDepth), listOffset(_listOffset) {
      }
      
      KDevelop::DeclarationPointer declaration;
      KSharedPtr<Cpp::CodeCompletionContext> completionContext;
      int inheritanceDepth; //Inheritance-depth: 0 for local functions(within no class), 1 for within local class, 1000+ for global items.
      int listOffset; //If it is an argument-hint, this contains the offset within the completion-context's function-list

      //If this is a completion for an include-file, this contains the file.
      Cpp::IncludeItem includeItem;
    };

    void createArgumentList(const CompletionItem& item, QString& ret, QList<QVariant>* highlighting ) const;
    
    mutable CompletionItem m_currentMatchContext;
    
    QMap<QString, QIcon> m_icons;
    mutable QMap<const CompletionItem*, QPointer<Cpp::NavigationWidget> > m_navigationWidgets;
    QList< CompletionItem > m_declarations;
};


/**
 * There may be multiple differnt parsed versions of a document available in the du-chain.
 * This function helps choosing the right one, by creating a standard parsing-environment,
 * and searching for a TopDUContext that fits in. If this fails, a random version is chosen.
 *
 * If simplified environment-matching is enabled, and a proxy-context is found, it returns
 * that proxy-contexts target-context, so the returned context may be used for completion etc.
 * without additional checking.
 *
 * @todo Move this somewhere more general
 *
 * @warning The du-chain must be locked before calling this.
* */
KDevelop::TopDUContext* getCompletionContext( const KUrl& url );

#endif

// kate: space-indent on; indent-width 2; replace-tabs on
