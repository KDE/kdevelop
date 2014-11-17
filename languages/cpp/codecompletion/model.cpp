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

#include "model.h"

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/codecompletioninterface.h>

#include <language/codecompletion/codecompletioncontext.h>
#include <language/backgroundparser/backgroundparser.h>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/idocumentcontroller.h>

#include "worker.h"
#include "context.h"
#include "../cppduchain/typeconversion.h"
#include "../debug.h"

using namespace KTextEditor;
using namespace KDevelop;

namespace Cpp {

CodeCompletionModel* CodeCompletionModel::s_self = nullptr;

bool useArgumentHintInAutomaticCompletion() {
  return false;
}

CodeCompletionModel::CodeCompletionModel( QObject * parent )
  : KDevelop::CodeCompletionModel(parent)
{
  setForceWaitForModel(true);
  Q_ASSERT(!s_self);
  s_self = this;

  connect(ICore::self()->languageController()->backgroundParser(), &BackgroundParser::parseJobFinished,
          this, &CodeCompletionModel::parseJobFinished);
}

CodeCompletionModel::~CodeCompletionModel()
{
  s_self = nullptr;
}

CodeCompletionModel* CodeCompletionModel::self()
{
  return s_self;
}

void CodeCompletionModel::startCompletionAfterParsing(const IndexedString& path)
{
  m_awaitDocument = path;
}

void CodeCompletionModel::parseJobFinished(ParseJob* job)
{
  if (job->document() != m_awaitDocument || ICore::self()->languageController()->backgroundParser()->isQueued(m_awaitDocument)) {
    return;
  }

  IDocument* doc = ICore::self()->documentController()->documentForUrl(m_awaitDocument.toUrl());
  m_awaitDocument = {};

  if (!doc) {
    return;
  }

  auto view = doc->activeTextView();
  if (!view || !view->hasFocus()) {
    return;
  }

  auto iface = dynamic_cast<KTextEditor::CodeCompletionInterface*>(view);
  if (iface) {
    ///@todo 1. This is a non-public interface, and 2. Completion should be started in "automatic invocation" mode
    QMetaObject::invokeMethod(view, "userInvokedCompletion");
  }
}

bool CodeCompletionModel::shouldStartCompletion(KTextEditor::View* view, const QString& inserted, bool userInsertion, const KTextEditor::Cursor& position) {
  qCDebug(CPP) << inserted;
  QString insertedTrimmed = inserted.trimmed();

  TypeConversion::startCache();

  QString lineText = view->document()->text(KTextEditor::Range(position.line(), 0, position.line(), position.column()));

  if(lineText.startsWith("#") && lineText.contains("include") && inserted.endsWith("/"))
    return true; //Directory-content completion

  if(insertedTrimmed.endsWith('\"'))
    return false; //Never start completion behind a string literal

  if(useArgumentHintInAutomaticCompletion())
    if(insertedTrimmed.endsWith( '(' ) || insertedTrimmed.endsWith(',') || insertedTrimmed.endsWith('<') || insertedTrimmed.endsWith(":") )
      return true;

  //Start automatic completion behind '::'
  if(insertedTrimmed.endsWith(":") && position.column() > 1 && lineText.right(2) == "::")
    return true;

  return KDevelop::CodeCompletionModel::shouldStartCompletion(view, inserted, userInsertion, position);
}

void CodeCompletionModel::aborted(KTextEditor::View* view) {
    qCDebug(CPP) << "aborting";
    worker()->abortCurrentCompletion();
    TypeConversion::stopCache();

    KDevelop::CodeCompletionModel::aborted(view);
}

bool isValidIncludeDirectiveCharacter(QChar character) {
  return character.isLetterOrNumber() || character == '_' || character == '-' || character == '.';
}

bool CodeCompletionModel::shouldAbortCompletion(KTextEditor::View* view, const KTextEditor::Range& range, const QString& currentCompletion)
{
  if(view->cursorPosition() < range.start() || view->cursorPosition() > range.end())
    return true; //Always abort when the completion-range has been left
  //Do not abort completions when the text has been empty already before and a newline has been entered

  QString line = view->document()->line(range.start().line()).trimmed();
  if(line.startsWith("#include")) {
    //Do custom check for include directives, since we allow more character then during usual completion
    QString text = view->document()->text(range);
    for(int a = 0; a < text.length(); ++a) {
      if(!isValidIncludeDirectiveCharacter(text[a]))
        return true;
    }
    return false;
  }

  static const QRegExp allowedText("^\\~?(\\w*)");
  return !allowedText.exactMatch(currentCompletion);
}

KDevelop::CodeCompletionWorker* CodeCompletionModel::createCompletionWorker() {
  return new CodeCompletionWorker(this);
}

Range CodeCompletionModel::updateCompletionRange(View* view, const KTextEditor::Range& range) {
  if(completionContext()) {
    Cpp::CodeCompletionContext* cppContext = dynamic_cast<Cpp::CodeCompletionContext*>(completionContext().data());
    Q_ASSERT(cppContext);
    cppContext->setFollowingText(view->document()->text(range));
    bool didReset = false;
    if(completionContext()->ungroupedElements().size()) {
      //Update the ungrouped elements, since they may have changed their text
      int row = rowCount() - completionContext()->ungroupedElements().size();

      foreach(KDevelop::CompletionTreeElementPointer item, completionContext()->ungroupedElements()) {
        KDevelop::CompletionCustomGroupNode* group = dynamic_cast<KDevelop::CompletionCustomGroupNode*>(item.data());
        if(group) {
          int subRow = 0;
          foreach(KDevelop::CompletionTreeElementPointer item, group->children) {
            if(item->asItem() && item->asItem()->dataChangedWithInput()) {
//               dataChanged(index(subRow, Name, parent), index(subRow, Name, parent));
              qCDebug(CPP) << "doing dataChanged";
              ///@todo This is very expensive, but kate doesn't listen for dataChanged(..). Find a cheaper way to achieve this.
              beginResetModel();
              endResetModel();
              didReset = true;
              break;
            }
            ++subRow;
          }
        }

        if(didReset)
          break;

        if(item->asItem() && item->asItem()->dataChangedWithInput()) {
          beginResetModel();
          endResetModel();
          didReset = true;
          break;
        }
        ++row;
      }
//       dataChanged(index(rowCount() - completionContext()->ungroupedElements().size(), 0), index(rowCount()-1, columnCount()-1 ));
    }
  }

  QString line = view->document()->line(range.start().line()).trimmed();
  if(line.startsWith("#include")) {
    //Skip over all characters that are allowed in a filename but usually not in code-completion
    KTextEditor::Range newRange = range;
    while(newRange.start().column() > 0) {
      KTextEditor::Cursor newStart = newRange.start();
      newStart.setColumn(newStart.column()-1);
      QChar character = view->document()->characterAt(newStart);
      if(isValidIncludeDirectiveCharacter(character)) {
        newRange.setStart(newStart); //Skip
      }else{
        break;
      }
    }
    qCDebug(CPP) << "new range:" << newRange;
    return newRange;
  }

  return KDevelop::CodeCompletionModel::updateCompletionRange(view, range);
}

Range CodeCompletionModel::completionRange(View* view, const KTextEditor::Cursor& position)
{
    Range range = KDevelop::CodeCompletionModel::completionRange(view, position);
    if (range.start().column() > 0) {
        KTextEditor::Range preRange(Cursor(range.start().line(), range.start().column() - 1),
                                    Cursor(range.start().line(), range.start().column()));
        const QString contents = view->document()->text(preRange);
        if ( contents == "~" ) {
            range.expandToRange(preRange);
        }
    }
    return range;
}

void CodeCompletionModel::foundDeclarations(QList<CompletionTreeElementPointer> item, KDevelop::CodeCompletionContext::Ptr completionContext)
{
  TypeConversion::startCache();
  KDevelop::CodeCompletionModel::foundDeclarations(item, completionContext);
}

}

