/*
 * KDevelop Code Completion Support
 *
 * Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
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

#include "codecompletionworker.h"

#include <kdebug.h>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <klocale.h>

#include "ducontext.h"
#include "codecompletion.h"

using namespace KTextEditor;
using namespace KDevelop;

CodeCompletionWorker::CodeCompletionWorker(QObject* parent)
  : QThread(parent)
  , m_mutex(new QMutex())
  , m_abort(false)
{
}

CodeCompletionWorker::~CodeCompletionWorker()
{
  delete m_mutex;
}

void CodeCompletionWorker::computeCompletions(KDevelop::DUContextPointer context, const KTextEditor::Cursor& position, KTextEditor::View* view)
{
  {
    QMutexLocker lock(m_mutex);
    m_abort = false;
  }

  //Compute the text we should complete on
  KTextEditor::Document* doc = view->document();
  if( !doc ) {
    kDebug(9007) << "No document for completion";
    return;
  }

  KTextEditor::Range range;
  QString text;
  {
    range = KTextEditor::Range(context->range().start.textCursor(), position);
    text = doc->text(range);
  }

  if( text.isEmpty() ) {
    kDebug(9007) << "no text for context";
    return;
  }

  if( position.column() == 0 ) //Seems like when the cursor is a the beginning of a line, kate does not give the \n
    text += '\n';

  if (aborting())
    return;

  computeCompletions(context, position, view, range, text);
}

void CodeCompletionWorker::run()
{
  exec();
}

void CodeCompletionWorker::abortCurrentCompletion()
{
  QMutexLocker lock(m_mutex);
  m_abort = true;
}

bool& CodeCompletionWorker::aborting()
{
  return m_abort;
}

#include "codecompletionworker.moc"
