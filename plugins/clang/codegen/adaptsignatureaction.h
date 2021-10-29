/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef ADAPTSIGNATUREACTION_H
#define ADAPTSIGNATUREACTION_H

#include <interfaces/iassistant.h>
#include <language/duchain/declaration.h>
#include <language/duchain/topducontext.h>

namespace KDevelop {
class RenameAction;
}

using ParameterItem = QPair<KDevelop::IndexedType, QString>;
struct Signature
{
    Signature()
        : isConst(false)
        {}
    QVector<ParameterItem> parameters;
    QList<QString> defaultParams;
    KDevelop::IndexedType returnType;
    bool isConst;
};

class AdaptSignatureAction
    : public KDevelop::IAssistantAction
{
    Q_OBJECT

public:
    AdaptSignatureAction(const KDevelop::DeclarationId& definitionId,
                         const KDevelop::ReferencedTopDUContext& definitionContext,
                         const Signature& oldSignature,
                         const Signature& newSignature,
                         bool editingDefinition,
                         const QList<KDevelop::RenameAction*>& renameActions);
    ~AdaptSignatureAction() override;

    QString description() const override;
    QString toolTip() const override;
    void execute() override;

private:
    KDevelop::DeclarationId m_otherSideId;
    KDevelop::ReferencedTopDUContext m_otherSideTopContext;
    Signature m_oldSignature;
    Signature m_newSignature;
    bool m_editingDefinition;
    const QList<KDevelop::RenameAction*> m_renameActions;
};

#endif // ADAPTSIGNATUREACTION_H
