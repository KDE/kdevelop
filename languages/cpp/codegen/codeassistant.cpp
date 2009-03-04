/*
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

#include "codeassistant.h"
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <ktexteditor/document.h>
#include <interfaces/iuicontroller.h>
#include <ktexteditor/view.h>

using namespace Cpp;
using namespace KDevelop;

StaticCodeAssistant staticCodeAssistant;

Cpp::StaticCodeAssistant::StaticCodeAssistant() {
  connect(KDevelop::ICore::self()->documentController(), SIGNAL(documentLoaded(KDevelop::IDocument*)), SLOT(documentLoaded(KDevelop::IDocument*)));
}

void Cpp::StaticCodeAssistant::documentLoaded(KDevelop::IDocument* document) {
  if(document->textDocument())
    connect(document->textDocument(), SIGNAL(textInserted(KTextEditor::Document*,KTextEditor::Range)), SLOT(textInserted(KTextEditor::Document*,KTextEditor::Range)));
}

QString Cpp::CreateDeclarationAction::description() const {
  return "create declaration";
}

void Cpp::CreateDeclarationAction::execute() {
  kDebug() << "executing action";
}

Cpp::CodeAssistant::CodeAssistant(KTextEditor::View* view) : ITextAssistant(view) {
}

QList< KDevelop::IAssistantAction::Ptr > Cpp::CodeAssistant::actions() {
  QList< KDevelop::IAssistantAction::Ptr > ret;
  ret << KDevelop::IAssistantAction::Ptr(new Cpp::CreateDeclarationAction);
  ret << KDevelop::IAssistantAction::Ptr(new Cpp::CreateDeclarationAction);
  ret << KDevelop::IAssistantAction::Ptr(new Cpp::CreateDeclarationAction);
  return ret;
}

void Cpp::StaticCodeAssistant::viewCursorPositionChanged(KTextEditor::View* view, KTextEditor::Cursor cursor) {
    bool doDisconnect = false;
    if(m_activeAssistant) {
      
    }else{
      doDisconnect = true;
    }
    
    if(doDisconnect)
      disconnect(view, SIGNAL(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)), this, SLOT(viewCursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)));      
}

void Cpp::StaticCodeAssistant::textInserted(KTextEditor::Document* document, KTextEditor::Range range) {
  QString inserted = document->text(range);
  if(range.columnWidth() == 1 && inserted == ";" && 0) {
    //Eventually pop up an assistant
    if(!document->activeView())
      return;
    
    connect(document->activeView(), SIGNAL(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)), SLOT(viewCursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)));
    
    m_activeAssistant = KSharedPtr<CodeAssistant>(new CodeAssistant(document->activeView()));
    ICore::self()->uiController()->popUpAssistant(KDevelop::IAssistant::Ptr(m_activeAssistant.data()));
  }else{
    if(inserted != "\n" && inserted != " " && m_activeAssistant)
      ICore::self()->uiController()->hideAssistant(KDevelop::IAssistant::Ptr(m_activeAssistant.data()));
  }
}



#include "codeassistant.moc"
