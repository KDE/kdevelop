/*
 * This file is part of KDevelop
 *
 * Copyright 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "clangrefactoring.h"

#include <QAction>
#include <QIcon>

#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/functiondeclaration.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/types/functiontype.h>
#include <language/interfaces/codecontext.h>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iuicontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <sublime/message.h>

#include "duchain/clanghelpers.h"
#include "duchain/documentfinderhelpers.h"
#include "duchain/duchainutils.h"
#include "sourcemanipulation.h"
#include "util/clangdebug.h"

using namespace KDevelop;

namespace {

bool isDestructor(Declaration* decl)
{
    if (auto functionDef = dynamic_cast<FunctionDefinition*>(decl)) {
        // we found a definition, e.g. "Foo::~Foo()"
        const auto functionDecl = functionDef->declaration(decl->topContext());
        if (auto classFunctionDecl = dynamic_cast<ClassFunctionDeclaration*>(functionDecl)) {
            return classFunctionDecl->isDestructor();
        }
    }
    else if (auto classFunctionDecl = dynamic_cast<ClassFunctionDeclaration*>(decl)) {
        // we found a declaration, e.g. "~Foo()"
        return classFunctionDecl->isDestructor();
    }

    return false;
}

}

ClangRefactoring::ClangRefactoring(QObject* parent)
    : BasicRefactoring(parent)
{
    qRegisterMetaType<IndexedDeclaration>();
}

void ClangRefactoring::fillContextMenu(ContextMenuExtension& extension, Context* context, QWidget* parent)
{
    auto declContext = dynamic_cast<DeclarationContext*>(context);
    if (!declContext) {
        return;
    }

    DUChainReadLocker lock;

    auto declaration = declContext->declaration().data();
    if (!declaration) {
        return;
    }

    QFileInfo fileInfo(declaration->topContext()->url().str());
    if (!fileInfo.isWritable()) {
        return;
    }

    auto action = new QAction(i18nc("@action", "Rename %1", declaration->qualifiedIdentifier().toString()), parent);
    action->setData(QVariant::fromValue(IndexedDeclaration(declaration)));
    action->setIcon(QIcon::fromTheme(QStringLiteral("edit-rename")));
    connect(action, &QAction::triggered, this, &ClangRefactoring::executeRenameAction);

    extension.addAction(ContextMenuExtension::RefactorGroup, action);

    if (!validCandidateToMoveIntoSource(declaration)) {
        return;
    }

    action = new QAction(
        i18n("Create separate definition for %1", declaration->qualifiedIdentifier().toString()), parent);
    action->setData(QVariant::fromValue(IndexedDeclaration(declaration)));
    connect(action, &QAction::triggered, this, &ClangRefactoring::executeMoveIntoSourceAction);
    extension.addAction(ContextMenuExtension::RefactorGroup, action);
}

bool ClangRefactoring::validCandidateToMoveIntoSource(Declaration* decl)
{
    if (!decl || !decl->isFunctionDeclaration() || !decl->type<FunctionType>()) {
        return false;
    }

    if (!decl->internalContext() || decl->internalContext()->type() != DUContext::Function) {
        return false;
    }

    if (!decl->isDefinition()) {
        return false;
    }

    auto childCtx = decl->internalContext()->childContexts();
    if (childCtx.isEmpty()) {
        return false;
    }

    auto ctx = childCtx.first();
    if (!ctx || ctx->type() != DUContext::Other) {
        return false;
    }

    auto functionDecl = dynamic_cast<AbstractFunctionDeclaration*>(decl);
    if (!functionDecl || functionDecl->isInline()) {
        return false;
    }

    return true;
}

QString ClangRefactoring::moveIntoSource(const IndexedDeclaration& iDecl)
{
    DUChainReadLocker lock;
    auto decl = iDecl.data();
    if (!decl) {
        return i18n("No declaration under cursor");
    }

    const auto headerUrl = decl->url();
    auto targetUrl = DocumentFinderHelpers::sourceForHeader(headerUrl.str());

    if (targetUrl.isEmpty() || targetUrl == headerUrl.str()) {
        // TODO: Create source file if it doesn't exist
        return i18n("No source file available for %1.", headerUrl.str());
    }

    lock.unlock();
    const IndexedString indexedTargetUrl(targetUrl);
    auto top
        = DUChain::self()->waitForUpdate(headerUrl, KDevelop::TopDUContext::AllDeclarationsAndContexts);
    auto targetTopContext
        = DUChain::self()->waitForUpdate(indexedTargetUrl, KDevelop::TopDUContext::AllDeclarationsAndContexts);
    lock.lock();

    if (!targetTopContext) {
        return i18n("Failed to update DUChain for %1.", targetUrl);
    }

    if (!top || !iDecl.data() || iDecl.data() != decl) {
        return i18n("Declaration lost while updating.");
    }

    clangDebug() << "moving" << decl->qualifiedIdentifier();

    if (!validCandidateToMoveIntoSource(decl)) {
        return i18n("Cannot create definition for this declaration.");
    }

    auto otherCtx = decl->internalContext()->childContexts().first();

    auto code = createCodeRepresentation(headerUrl);
    if (!code) {
        return i18n("No document for %1", headerUrl.str());
    }

    auto bodyRange = otherCtx->rangeInCurrentRevision();

    auto prefixRange(ClangIntegration::DUChainUtils::functionSignatureRange(decl));
    const auto prefixText = code->rangeText(prefixRange);
    for (int i = prefixText.length() - 1; i >= 0 && prefixText.at(i).isSpace(); --i) {
        if (bodyRange.start().column() == 0) {
            bodyRange.setStart(bodyRange.start() - KTextEditor::Cursor(1, 0));
            if (bodyRange.start().line() == prefixRange.start().line()) {
                bodyRange.setStart(KTextEditor::Cursor(bodyRange.start().line(), prefixRange.start().column() + i));
            } else {
                int lastNewline = prefixText.lastIndexOf(QLatin1Char('\n'), i - 1);
                bodyRange.setStart(KTextEditor::Cursor(bodyRange.start().line(), i - lastNewline - 1));
            }
        } else {
            bodyRange.setStart(bodyRange.start() - KTextEditor::Cursor(0, 1));
        }
    }

    const QString body = code->rangeText(bodyRange);
    SourceCodeInsertion ins(targetTopContext);
    auto parentId = decl->internalContext()->parentContext()->scopeIdentifier(false);

    ins.setSubScope(parentId);

    Identifier id(IndexedString(decl->qualifiedIdentifier().mid(parentId.count()).toString()));
    clangDebug() << "id:" << id;

    if (!ins.insertFunctionDeclaration(decl, id, body)) {
        return i18n("Insertion failed");
    }
    lock.unlock();

    auto applied = ins.changes().applyAllChanges();
    if (!applied) {
        return i18n("Applying changes failed: %1", applied.m_failureReason);
    }

    // replace function body with a semicolon
    DocumentChangeSet changeHeader;
    changeHeader.addChange(DocumentChange(headerUrl, bodyRange, body, QStringLiteral(";")));
    applied = changeHeader.applyAllChanges();
    if (!applied) {
        return i18n("Applying changes failed: %1", applied.m_failureReason);
    }

    ICore::self()->languageController()->backgroundParser()->addDocument(headerUrl);
    ICore::self()->languageController()->backgroundParser()->addDocument(indexedTargetUrl);

    return {};
}

void ClangRefactoring::executeMoveIntoSourceAction()
{
    auto action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);

    auto iDecl = action->data().value<IndexedDeclaration>();
    if (!iDecl.isValid()) {
        iDecl = declarationUnderCursor(false);
    }

    const auto error = moveIntoSource(iDecl);
    if (!error.isEmpty()) {
        auto* message = new Sublime::Message(error, Sublime::Message::Error);
        ICore::self()->uiController()->postMessage(message);
    }
}

DocumentChangeSet::ChangeResult ClangRefactoring::applyChangesToDeclarations(const QString& oldName,
                                                                             const QString& newName,
                                                                             DocumentChangeSet& changes,
                                                                             const QList<IndexedDeclaration>& declarations)
{
    for (const IndexedDeclaration decl : declarations) {
        Declaration *declaration = decl.data();
        if (!declaration)
            continue;

        if (declaration->range().isEmpty())
            clangDebug() << "found empty declaration:" << declaration->toString();

        // special handling for dtors, their name is not "Foo", but "~Foo"
        // see https://bugs.kde.org/show_bug.cgi?id=373452
        QString fixedOldName = oldName;
        QString fixedNewName = newName;

        if (isDestructor(declaration)) {
            clangDebug() << "found destructor:" << declaration->toString() << "-- making sure we replace the identifier correctly";
            fixedOldName = QLatin1Char('~') + oldName;
            fixedNewName = QLatin1Char('~') + newName;
        }

        TopDUContext *top = declaration->topContext();
        DocumentChangeSet::ChangeResult result = changes.addChange(DocumentChange(top->url(), declaration->rangeInCurrentRevision(), fixedOldName, fixedNewName));
        if (!result)
            return result;
    }

    return KDevelop::DocumentChangeSet::ChangeResult::successfulResult();
}
