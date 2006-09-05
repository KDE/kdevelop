/*
 * KDevelop C++ Code Completion Support
 *
 * Copyright (c) 2006 Hamish Rodda <rodda@kde.org>
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

#include "cppcodecompletion.h"

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/codecompletion2.h>

#include <kdevcore.h>
#include <kdevdocumentcontroller.h>

#include "cpplanguagesupport.h"
#include "cppcodecompletionmodel.h"

#include "duchain/duchain.h"
#include "duchain/topducontext.h"

using namespace KTextEditor;

CppCodeCompletion::CppCodeCompletion( CppLanguageSupport * parent )
  : QObject(parent)
  , m_model(new CppCodeCompletionModel(this))
{
  connect (KDevCore::documentController(), SIGNAL(documentLoaded(KDevDocument*)), SLOT(documentLoaded(KDevDocument*)));
}

CppCodeCompletion::~CppCodeCompletion()
{
}

void CppCodeCompletion::cursorPositionChanged()
{
  View* view = dynamic_cast<KTextEditor::View*>(sender());
  if (!view) {
    kWarning() << k_funcinfo << "Non-view caller" << endl;
    return;
  }

  CodeCompletionInterface2* cc = dynamic_cast<CodeCompletionInterface2*>(view);

  if (!cc || cc->isCompletionActive())
    return;

  KTextEditor::Cursor end = view->cursorPosition();

  if (!end.column())
    return;

  QString text = view->document()->line(end.line());

  static QRegExp findWordStart( "\\b(\\w+)$" );
  static QRegExp findWordEnd( "^(\\w*)\\b" );

  if ( findWordStart.lastIndexIn(text.left(end.column())) < 0 )
    return;

  KTextEditor::Cursor start(end.line(), findWordStart.pos(1));

  if ( findWordEnd.indexIn(text.mid(end.column())) < 0 )
    return;

  end.setColumn(end.column() + findWordEnd.cap(1).length());

  //m_startText = text.mid(start.column(), end.column() - start.column());

  KUrl url = view->document()->url();
  if (TopDUContext* top = DUChain::self()->chainForDocument(url)) {
    QReadLocker lock(top->chainLock());
    DUContext* thisContext = top->findContextAt(end);

    m_model->setContext(thisContext, end);

    cc->startCompletion(KTextEditor::Range(start, end), m_model);
  }
}

void CppCodeCompletion::viewCreated(KTextEditor::Document * document, KTextEditor::View * view)
{
  Q_UNUSED(document);
  connect(view, SIGNAL(cursorPositionChanged(KTextEditor::View*, const KTextEditor::Cursor&)), SLOT(cursorPositionChanged()));
}

void CppCodeCompletion::documentLoaded(KDevDocument* document)
{
  if (document->textDocument()) {
    foreach (KDocument::View* view, document->textDocument()->views())
      connect(view, SIGNAL(cursorPositionChanged(KTextEditor::View*, const KTextEditor::Cursor&)), SLOT(cursorPositionChanged()));

    connect(document->textDocument(), SIGNAL(viewCreated(KTextEditor::Document*, KTextEditor::View*)), SLOT(viewCreated(KTextEditor::Document*, KTextEditor::View*)));

  } else {
    kDebug() << k_funcinfo << "Non-text editor document added" << endl;
  }
}

#include "cppcodecompletion.moc"

// kate: space-indent on; indent-width 2; replace-tabs on
