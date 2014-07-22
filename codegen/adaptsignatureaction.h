/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   Copyright 2014 Kevin Funk <kfunk@kde.org>

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

#ifndef ADAPTSIGNATUREACTION_H
#define ADAPTSIGNATUREACTION_H

#include <interfaces/iassistant.h>
#include <language/duchain/declaration.h>
#include <language/duchain/topducontext.h>

namespace KDevelop {
class RenameAction;
}

typedef QPair<KDevelop::IndexedType, QString> ParameterItem;
struct Signature
{
    QList<ParameterItem> parameters;
    QList<QString> defaultParams;
    KDevelop::IndexedType returnType;
    bool isConst;
};

class AdaptSignatureAction
    : public KDevelop::IAssistantAction
{
    Q_OBJECT

public:
    AdaptSignatureAction(const KDevelop::DeclarationId&   definitionId,
                         KDevelop::ReferencedTopDUContext definitionContext,
                         const Signature& oldSignature,
                         const Signature& newSignature,
                         bool editingDefinition,
                         QList<KDevelop::RenameAction*> renameActions
                         );
    virtual ~AdaptSignatureAction();

    virtual QString description() const override;
    virtual QString toolTip() const override;
    virtual void execute() override;

private:
    KDevelop::DeclarationId m_otherSideId;
    KDevelop::ReferencedTopDUContext m_otherSideTopContext;
    Signature m_oldSignature;
    Signature m_newSignature;
    bool m_editingDefinition;
    QList<KDevelop::RenameAction*> m_renameActions;
};

#endif // ADAPTSIGNATUREACTION_H
