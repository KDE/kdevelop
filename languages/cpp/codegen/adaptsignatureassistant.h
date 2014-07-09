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

#include "adaptsignatureaction.h"

#include <language/assistant/staticassistant.h>
#include <language/duchain/identifier.h>
#include <language/duchain/topducontext.h>

#include <KUrl>

namespace KTextEditor {
class View;
}

namespace KDevelop {
class ParseJob;
}

using namespace KDevelop;
namespace Cpp {

class AdaptSignatureAssistant : public StaticAssistant
{
  Q_OBJECT
  public:
    AdaptSignatureAssistant(ILanguageSupport* supportedLanguage);

    virtual QString title() const;
    virtual void textChanged(KTextEditor::View* view, const KTextEditor::Range& invocationRange, const QString& removedText = QString());
    virtual bool isUseful() const;

  private:
    DUContext* findFunctionContext(const KUrl& url, const KTextEditor::Range& position) const;

    ///Compare @param newSignature to m_oldSignature and put differences in oldPositions
    ///@returns whether or not there are any differences
    bool getSignatureChanges(const Signature &newSignature, QList<int> &oldPositions) const;
    ///Set default params in @param newSignature based on m_oldSignature's defaults and @param oldPositions
    void setDefaultParams(Signature &newSignature, const QList<int> &oldPositions) const;
    ///@returns RenameActions for each parameter in newSignature that has been renamed
    QList<RenameAction*> getRenameActions(const Signature &newSignature, const QList<int> &oldPositions) const;

    // If this is true, the user is editing on the definition side,
    // and the declaration should be updated
    bool m_editingDefinition = false;
    Identifier m_declarationName;
    DeclarationId m_otherSideId;
    ReferencedTopDUContext m_otherSideTopContext;
    DUContextPointer m_otherSideContext;
    //old signature of the _other_side
    Signature m_oldSignature;
    KUrl m_document;
    QWeakPointer<KTextEditor::View> m_view;

  private slots:
    void parseJobFinished(KDevelop::ParseJob*);
    void reset();
};

}

#endif // CPP_SIGNATUREASSISTANT_H
