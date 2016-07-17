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
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/functiondefinition.h>

#include <KLocalizedString>
#include <KMessageBox>

using namespace KDevelop;

AdaptSignatureAction::AdaptSignatureAction(const DeclarationId& definitionId,
                                           ReferencedTopDUContext definitionContext,
                                           const Signature& oldSignature,
                                           const Signature& newSignature,
                                           bool editingDefinition,
                                           const QList<RenameAction*>& renameActions)
    : m_otherSideId(definitionId)
    , m_otherSideTopContext(definitionContext)
    , m_oldSignature(oldSignature)
    , m_newSignature(newSignature)
    , m_editingDefinition(editingDefinition)
    , m_renameActions(renameActions)
{
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
    auto declaration = m_otherSideId.getDeclaration(m_otherSideTopContext.data());
    if (!declaration) {
        return {};
    }
    return i18n("Update %1 signature\nfrom: %2\nto: %3",
                m_editingDefinition ? i18n("declaration") : i18n("definition"),
                CodegenHelper::makeSignatureString(declaration, m_oldSignature, m_editingDefinition),
                CodegenHelper::makeSignatureString(declaration, m_newSignature, !m_editingDefinition));
}

void AdaptSignatureAction::execute()
{
    Q_ASSERT(!DUChain::lock()->currentThreadHasReadLock() && !DUChain::lock()->currentThreadHasWriteLock());
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
    QString newText = CodegenHelper::makeSignatureString(otherSide, m_newSignature, !m_editingDefinition);
    if (!m_editingDefinition) {
        // append a newline after the method signature in case the method definition follows
        newText += QLatin1Char('\n');
    }

    DocumentChange changeParameters(functionContext->url(), parameterRange, QString(), newText);
    lock.unlock();
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
