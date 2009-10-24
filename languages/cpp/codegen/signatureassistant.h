/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   
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

#ifndef CPP_SIGNATUREASSISTANT_H
#define CPP_SIGNATUREASSISTANT_H

#include <interfaces/iassistant.h>
#include <language/duchain/identifier.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/types/indexedtype.h>
#include <language/duchain/indexedstring.h>


namespace KDevelop {
class ParseJob;
}
namespace Cpp {
typedef QPair<KDevelop::IndexedType, QString> ParameterItem;
struct Signature
{
  QList<ParameterItem> parameters;
  QList<QString> defaultParams;
  KDevelop::IndexedType returnType;
};

class AdaptDefinitionSignatureAssistant : public KDevelop::ITextAssistant {
  Q_OBJECT
  public:
    AdaptDefinitionSignatureAssistant(KTextEditor::View* view, KTextEditor::Range inserted);
    bool isUseful();
    
  private:
    KDevelop::Identifier m_declarationName;
    
    bool m_editingDefinition; //If this is true, the user is editing on the definition side, and the declaration should be updated
    
    KDevelop::DeclarationId m_definitionId;
    KDevelop::ReferencedTopDUContext m_definitionContext;
    Signature m_oldSignature; //old signature of the _other_side
    KDevelop::IndexedString m_document;
    KDevelop::SimpleRange m_invocationRange;
    
    KDevelop::DUContext* findFunctionContext(KUrl url, KDevelop::SimpleRange position) const;
  private slots:
    void parseJobFinished(KDevelop::ParseJob*);
};

}

#endif // CPP_SIGNATUREASSISTANT_H
