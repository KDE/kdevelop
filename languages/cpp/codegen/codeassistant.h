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

#ifndef CPP_CODEASSISTANT_H
#define CPP_CODEASSISTANT_H

#include <interfaces/iassistant.h>
#include <QObject>
#include <ktexteditor/cursor.h>
#include <language/duchain/declarationid.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/indexedstring.h>
#include <language/duchain/types/indexedtype.h>

namespace KTextEditor {
class Document;
class Range;
class View;
}
namespace KDevelop {
class IDocument;
class ParseJob;
class DUContext;
}
namespace Cpp {

typedef QPair<KDevelop::IndexedType, KDevelop::IndexedString> SignatureItem;

class CreateDeclarationAction : public KDevelop::IAssistantAction {
  public:
    virtual QString description() const;
    virtual void execute();
};


class AdaptDefinitionSignatureAssistant : public KDevelop::ITextAssistant {
  Q_OBJECT
  public:
    AdaptDefinitionSignatureAssistant(KTextEditor::View* view, KTextEditor::Range inserted);
    bool isUseful();
    
  private:
    KDevelop::Identifier m_declarationName;
    KDevelop::DeclarationId m_definitionId;
    KDevelop::ReferencedTopDUContext m_definitionContext;
    QList<SignatureItem> m_oldSignature;
    KDevelop::IndexedString m_document;
    
    KDevelop::DUContext* findFunctionContext(KUrl url, KDevelop::SimpleCursor position) const;
  private slots:
    void parseJobFinished(KDevelop::ParseJob*);
};

class StaticCodeAssistant : public QObject {
  Q_OBJECT
  public:
    StaticCodeAssistant();
  private slots:
    void assistantHide();
    void documentLoaded(KDevelop::IDocument*);
    void textInserted(KTextEditor::Document*,KTextEditor::Range);
  private:
    KSharedPtr<KDevelop::IAssistant> m_activeAssistant;
};

}

#endif // CPP_CODEASSISTANT_H
