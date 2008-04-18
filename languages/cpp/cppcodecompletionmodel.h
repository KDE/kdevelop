/*
 * KDevelop C++ Code Completion Support
 *
 * Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
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
class QMutex;

namespace KDevelop
{
class DUContext;
class Declaration;
}

namespace Cpp {
  class CodeCompletionContext;
  class NavigationWidget;
}

class CodeCompletionWorker;

class CompletionTreeElement;

class CppCodeCompletionModel : public KTextEditor::CodeCompletionModel2
{
  Q_OBJECT

  public:
    CppCodeCompletionModel(QObject* parent);
    virtual ~CppCodeCompletionModel();

    virtual void completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType invocationType);

    virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;

    ///getData(..) for include-file completion
    QVariant getIncludeData(const QModelIndex& index, int role) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex parent ( const QModelIndex & index ) const;
  
    void setCompletionContext(KSharedPtr<Cpp::CodeCompletionContext> completionContext);
    KSharedPtr<Cpp::CodeCompletionContext> completionContext() const;
    
    struct CompletionItem {
      CompletionItem(KDevelop::DeclarationPointer decl = KDevelop::DeclarationPointer(), KSharedPtr<Cpp::CodeCompletionContext> context=KSharedPtr<Cpp::CodeCompletionContext>(), int _inheritanceDepth = 0, int _listOffset=0) : declaration(decl), completionContext(context), inheritanceDepth(_inheritanceDepth), listOffset(_listOffset) {
      }
      
      KDevelop::DeclarationPointer declaration;
      KSharedPtr<Cpp::CodeCompletionContext> completionContext;
      int inheritanceDepth; //Inheritance-depth: 0 for local functions(within no class), 1 for within local class, 1000+ for global items.
      int listOffset; //If it is an argument-hint, this contains the offset within the completion-context's function-list
      QString alternativeText; //Text shown when declaration is zero
      //If this is a completion for an include-file, this contains the file.
      Cpp::IncludeItem includeItem;
    };

  Q_SIGNALS:
    void completionsNeeded(KDevelop::DUContextPointer context, const KTextEditor::Cursor& position, KTextEditor::View* view);

  private Q_SLOTS:
    void foundDeclarations(QList<KSharedPtr<CompletionTreeElement> > item, void* completionContext);
    
  private:
    virtual void executeCompletionItem2(KTextEditor::Document* document, const KTextEditor::Range& word, const QModelIndex& index) const;
    KSharedPtr<Cpp::CodeCompletionContext> m_completionContext;
    typedef QPair<KDevelop::DeclarationPointer, KSharedPtr<Cpp::CodeCompletionContext> > DeclarationContextPair;

    void createArgumentList(const CompletionItem& item, QString& ret, QList<QVariant>* highlighting ) const;
    
    mutable CompletionItem m_currentMatchContext;
    
    mutable QMap<const CompletionItem*, QPointer<Cpp::NavigationWidget> > m_navigationWidgets;
    QList< KSharedPtr<CompletionTreeElement> > m_completionItems;

    QMutex* m_mutex;
    CodeCompletionWorker* m_worker;
    KDevelop::TopDUContextPointer m_currentTopContext;
};



#endif

