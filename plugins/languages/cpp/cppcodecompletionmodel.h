/*
 * KDevelop C++ Code Completion Support
 *
 * Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
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
#include <language/codecompletion/codecompletionmodel.h>
#include <ksharedptr.h>
#include <language/duchain/duchainpointer.h>
#include <language/codecompletion/codecompletioncontext.h>
#include "includeitem.h"
#include "completionitem.h"
#include <ktexteditor/codecompletionmodelcontrollerinterface.h>

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

class CppCodeCompletionWorker;

namespace KDevelop {
  class CompletionTreeElement;
}

class CppCodeCompletionModel : public KDevelop::CodeCompletionModel, public KTextEditor::CodeCompletionModelControllerInterface
{
  Q_OBJECT

  public:
    Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)
    CppCodeCompletionModel(QObject* parent);
    virtual ~CppCodeCompletionModel();

  protected:
    virtual void aborted(KTextEditor::View* view);
    virtual bool shouldAbortCompletion (KTextEditor::View* view, const KTextEditor::SmartRange& range, const QString& currentCompletion);
    virtual bool shouldStartCompletion (KTextEditor::View*, const QString&, bool userInsertion, const KTextEditor::Cursor&);
    virtual KDevelop::CodeCompletionWorker* createCompletionWorker();
    virtual void completionInvokedInternal(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType invocationType, const KUrl& url);

  private:
    KSharedPtr<Cpp::CodeCompletionContext> m_completionContext;
};



#endif

