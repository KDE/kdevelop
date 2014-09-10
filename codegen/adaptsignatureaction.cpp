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

#include "adaptsignatureaction.h"
#include "codegenhelper.h"

#include "../duchain/duchainutils.h"
#include "../util/clangdebug.h"

#include <language/assistant/renameaction.h>
#include <language/codegen/documentchangeset.h>
#include <language/duchain/types/arraytype.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/functiondefinition.h>

#include <KLocalizedString>
#include <KMessageBox>

using namespace KDevelop;

QString makeSignatureString(const DeclarationId& definitionId, const Signature& signature,
                            TopDUContext* visibilityFrom)
{
    Q_ASSERT(visibilityFrom);
    if (!visibilityFrom) {
        return QString();
    }

    QString ret = CodegenHelper::simplifiedTypeString(signature.returnType.abstractType(),
                                                      visibilityFrom);

    ret += ' ';

    const Declaration* functionDecl = definitionId.getDeclaration(visibilityFrom);
    QualifiedIdentifier namespaceIdentifier = functionDecl->internalContext()->parentContext()->scopeIdentifier(false);
    Identifier id(IndexedString(functionDecl->qualifiedIdentifier().mid(namespaceIdentifier.count()).toString()));
    ret += functionDecl->identifier().toString();

    ret += '(';
    int pos = 0;

    foreach(const ParameterItem &item, signature.parameters)
    {
        if (pos != 0) {
            ret += ", ";
        }

        ///TODO: merge common code with helpers.cpp::createArgumentList
        AbstractType::Ptr type = item.first.abstractType();

        QString arrayAppendix;
        ArrayType::Ptr arrayType;
        while ((arrayType = type.cast<ArrayType>())) {
            type = arrayType->elementType();
            //note: we have to prepend since we iterate from outside, i.e. from right to left.
            if (arrayType->dimension()) {
                arrayAppendix.prepend(QString("[%1]").arg(arrayType->dimension()));
            } else {
                // dimensionless
                arrayAppendix.prepend("[]");
            }
        }
        ret += CodegenHelper::simplifiedTypeString(type,
                                                   visibilityFrom);

        if (!item.second.isEmpty()) {
            ret += " " + item.second;
        }
        ret += arrayAppendix;
        if (signature.defaultParams.size() > pos && !signature.defaultParams[pos].isEmpty()) {
            ret += " = " + signature.defaultParams[pos];
        }
        ++pos;
    }
    ret += ')';
    if (signature.isConst) {
        ret += " const";
    }
    return ret;
}

AdaptSignatureAction::AdaptSignatureAction(const DeclarationId& definitionId,
                                           ReferencedTopDUContext definitionContext,
                                           const Signature& oldSignature,
                                           const Signature& newSignature,
                                           bool editingDefinition, QList<RenameAction*> renameActions
                                           )
    : m_otherSideId(definitionId)
    , m_otherSideTopContext(definitionContext)
    , m_oldSignature(oldSignature)
    , m_newSignature(newSignature)
    , m_editingDefinition(editingDefinition)
    , m_renameActions(renameActions) {
}

AdaptSignatureAction::~AdaptSignatureAction()
{
    qDeleteAll(m_renameActions);
}

QString AdaptSignatureAction::description() const
{
    return m_editingDefinition ? i18n("Update declaration signature") : i18n("Update definition signature");
}

QString AdaptSignatureAction::toolTip() const
{
    DUChainReadLocker lock;
    return i18n("Update %1\nfrom: %2(%3)%4\nto: %2(%5)%6",
                m_editingDefinition ? i18n("declaration") : i18n("definition"),
                m_otherSideId.qualifiedIdentifier().toString(),
                makeSignatureString(m_otherSideId,
                                    m_oldSignature,
                                    m_otherSideTopContext.data()),
                m_oldSignature.isConst ? " const" : "",
                makeSignatureString(m_otherSideId,
                                    m_newSignature,
                                    m_otherSideTopContext.data()),
                m_newSignature.isConst ? " const" : "");
}

void AdaptSignatureAction::execute()
{
    DUChainReadLocker lock;
    IndexedString url = m_otherSideTopContext->url();
    lock.unlock();
    m_otherSideTopContext = DUChain::self()->waitForUpdate(url, TopDUContext::AllDeclarationsContextsAndUses);
    if (!m_otherSideTopContext) {
        clangDebug() << "failed to update" << url.str();
        return;
    }

    lock.lock();

    Declaration* otherSide = m_otherSideId.getDeclaration(m_otherSideTopContext.data());
    if (!otherSide) {
        clangDebug() << "could not find definition";
        return;
    }
    DUContext* functionContext = DUChainUtils::getFunctionContext(otherSide);
    if (!functionContext) {
        clangDebug() << "no function context";
        return;
    }
    if (!functionContext || functionContext->type() != DUContext::Function) {
        clangDebug() << "no correct function context";
        return;
    }

    DocumentChangeSet changes;
    KTextEditor::Range parameterRange = ClangIntegration::DUChainUtils::functionSignatureRange(otherSide);
    QString newText = makeSignatureString(m_otherSideId,
                                          m_newSignature,
                                          m_otherSideTopContext.data());
    if (!m_editingDefinition) {
        // append a newline after the method signature in case the method definition follows
        newText += "\n";
    }

    DocumentChange changeParameters(functionContext->url(), parameterRange, QString(), newText);
    changeParameters.m_ignoreOldText = true;
    changes.addChange(changeParameters);
    changes.setReplacementPolicy(DocumentChangeSet::WarnOnFailedChange);
    DocumentChangeSet::ChangeResult result = changes.applyAllChanges();
    if (!result) {
        KMessageBox::error(0, i18n("Failed to apply changes: %1", result.m_failureReason));
    }
    emit executed(this);

    foreach(RenameAction * renAct, m_renameActions) {
        renAct->execute();
    }
}

#include "moc_adaptsignatureaction.cpp"
