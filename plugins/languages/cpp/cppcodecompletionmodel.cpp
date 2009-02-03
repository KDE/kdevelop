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

#include "cppcodecompletionmodel.h"

#include <QIcon>
#include <QMetaType>
#include <QTextFormat>
#include <QBrush>
#include <QDir>
#include <kdebug.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <kiconloader.h>
#include <khtmlview.h>


#include "cppduchain/cppduchain.h"
#include "cppduchain/typeutils.h"

#include "cppduchain/overloadresolutionhelper.h"

#include <language/duchain/declaration.h>
#include "cpptypes.h"
#include "typeutils.h"
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/duchain.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include <language/duchain/parsingenvironment.h>
#include <language/editor/editorintegrator.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainbase.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/dumpchain.h>
#include <language/codecompletion/codecompletioncontext.h>
#include "cppduchain/navigation/navigationwidget.h"
#include "preprocessjob.h"
#include <language/duchain/duchainutils.h>
#include "cppcodecompletionworker.h"
#include "cpplanguagesupport.h"
#include <language/editor/modificationrevision.h>
#include <language/duchain/specializationstore.h>
#include "implementationhelperitem.h"

using namespace KTextEditor;
using namespace KDevelop;
using namespace TypeUtils;

CppCodeCompletionModel::CppCodeCompletionModel( QObject * parent )
  : KDevelop::CodeCompletionModel(parent)
{
}

#if KDE_IS_VERSION(4,2,61)
KTextEditor::CodeCompletionModelControllerInterface2::MatchReaction CppCodeCompletionModel::matchingItem(const QModelIndex& matched) {
  KSharedPtr<CompletionTreeElement> element = itemForIndex(matched);
  //Do not hide the completion-list if the matched item is an implementation-helper
  if(dynamic_cast<ImplementationHelperItem*>(element.data()))
    return KTextEditor::CodeCompletionModelControllerInterface2::None;
  else
    return CodeCompletionModelControllerInterface2::matchingItem(matched);
}
#endif

bool CppCodeCompletionModel::shouldStartCompletion(KTextEditor::View* view, const QString& inserted, bool userInsertion, const KTextEditor::Cursor& position) {
  kDebug() << inserted;
  QString insertedTrimmed = inserted.trimmed();
  if(insertedTrimmed.endsWith('\"'))
    return false; //Never start completion behind a string literal
  if(insertedTrimmed.endsWith( '(' ) || insertedTrimmed.endsWith(',') || insertedTrimmed.endsWith('<') || insertedTrimmed.endsWith(":") )
    return true;
  
  return CodeCompletionModelControllerInterface::shouldStartCompletion(view, inserted, userInsertion, position);
}

void CppCodeCompletionModel::aborted(KTextEditor::View* view) {
    kDebug() << "aborting";
    worker()->abortCurrentCompletion();
    
    KTextEditor::CodeCompletionModelControllerInterface::aborted(view);
}

bool CppCodeCompletionModel::shouldAbortCompletion(KTextEditor::View* view, const KTextEditor::SmartRange& range, const QString& currentCompletion) {
  bool ret = CodeCompletionModelControllerInterface::shouldAbortCompletion(view, range, currentCompletion);
  
  return ret;
}

KDevelop::CodeCompletionWorker* CppCodeCompletionModel::createCompletionWorker() {
  return new CppCodeCompletionWorker(this);
}

CppCodeCompletionModel::~CppCodeCompletionModel()
{
}

#include "cppcodecompletionmodel.moc"
