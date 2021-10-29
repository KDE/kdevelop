/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef SIGNATUREASSISTANT_H
#define SIGNATUREASSISTANT_H

#include "adaptsignatureaction.h"
#include "clangprivateexport.h"

#include <language/assistant/staticassistant.h>
#include <language/duchain/identifier.h>
#include <language/duchain/topducontext.h>

#include <QPointer>

namespace KTextEditor {
class View;
}

class KDEVCLANGPRIVATE_EXPORT AdaptSignatureAssistant : public KDevelop::StaticAssistant
{
    Q_OBJECT

public:
    explicit AdaptSignatureAssistant(KDevelop::ILanguageSupport* supportedLanguage);

    QString title() const override;
    void textChanged(KTextEditor::Document* doc, const KTextEditor::Range& invocationRange, const QString& removedText = QString()) override;
    bool isUseful() const override;
    KTextEditor::Range displayRange() const override;

private:
    ///Compare @a newSignature to m_oldSignature and put differences in @a oldPositions
    ///@returns whether or not there are any differences
    bool getSignatureChanges(const Signature &newSignature, QList<int> &oldPositions) const;
    ///Set default params in @a newSignature based on m_oldSignature's defaults and @a oldPositions
    void setDefaultParams(Signature &newSignature, const QList<int> &oldPositions) const;
    ///@returns RenameActions for each parameter in newSignature that has been renamed
    QList<KDevelop::RenameAction*> getRenameActions(const Signature &newSignature, const QList<int> &oldPositions) const;

    // If this is true, the user is editing on the definition side,
    // and the declaration should be updated
    bool m_editingDefinition = false;
    KDevelop::Identifier m_declarationName;
    KDevelop::DeclarationId m_otherSideId;
    KDevelop::ReferencedTopDUContext m_otherSideTopContext;
    KDevelop::DUContextPointer m_otherSideContext;
    KTextEditor::Cursor m_lastEditPosition;
    //old signature of the _other_side
    Signature m_oldSignature;
    QPointer<KTextEditor::Document> m_document;
    QPointer<KTextEditor::View> m_view;

private Q_SLOTS:
    void updateReady(const KDevelop::IndexedString& document, const KDevelop::ReferencedTopDUContext& context) override;
    void reset();
};

#endif // SIGNATUREASSISTANT_H
