/*
    SPDX-FileCopyrightText: 2014 Miquel Sabaté <mikisabate@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// Qt
#include <QAction>
// KF
#include <KParts/MainWindow>
#include <KTextEditor/Document>
#include <KTextEditor/View>
// KDevelop
#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/contextmenuextension.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/navigation/abstractnavigationwidget.h>
#include <language/codegen/basicrefactoring.h>
#include <language/interfaces/codecontext.h>
#include <duchain/classdeclaration.h>
#include <duchain/classfunctiondeclaration.h>
#include <duchain/use.h>
#include <sublime/message.h>
#include <util/algorithm.h>

#include "progressdialogs/refactoringdialog.h"
#include <debug.h>

#include "ui_basicrefactoring.h"

namespace {
QPair<QString, QString> splitFileAtExtension(const QString& fileName)
{
    int idx = fileName.indexOf(QLatin1Char('.'));
    if (idx == -1) {
        return qMakePair(fileName, QString());
    }
    return qMakePair(fileName.left(idx), fileName.mid(idx));
}
}

using namespace KDevelop;

//BEGIN: BasicRefactoringCollector

BasicRefactoringCollector::BasicRefactoringCollector(const IndexedDeclaration& decl)
    : UsesWidgetCollector(decl)
{
    setCollectConstructors(true);
    setCollectDefinitions(true);
    setCollectOverloads(true);
}

QVector<IndexedTopDUContext> BasicRefactoringCollector::allUsingContexts() const
{
    return m_allUsingContexts;
}

void BasicRefactoringCollector::processUses(KDevelop::ReferencedTopDUContext topContext)
{
    m_allUsingContexts << IndexedTopDUContext(topContext.data());
    UsesWidgetCollector::processUses(topContext);
}

//END: BasicRefactoringCollector

//BEGIN: BasicRefactoring

BasicRefactoring::BasicRefactoring(QObject* parent)
    : QObject(parent)
{
    /* There's nothing to do here. */
}

void BasicRefactoring::fillContextMenu(ContextMenuExtension& extension, Context* context, QWidget* parent)
{
    auto* declContext = dynamic_cast<DeclarationContext*>(context);
    if (!declContext)
        return;

    DUChainReadLocker lock;
    Declaration* declaration = declContext->declaration().data();
    if (declaration && acceptForContextMenu(declaration)) {
        QFileInfo finfo(declaration->topContext()->url().str());
        if (finfo.isWritable()) {
            auto* action = new QAction(i18nc("@action", "Rename \"%1\"...",
                                            declaration->qualifiedIdentifier().toString()), parent);
            action->setData(QVariant::fromValue(IndexedDeclaration(declaration)));
            action->setIcon(QIcon::fromTheme(QStringLiteral("edit-rename")));
            connect(action, &QAction::triggered, this, &BasicRefactoring::executeRenameAction);
            extension.addAction(ContextMenuExtension::RefactorGroup, action);
        }
    }
}

bool BasicRefactoring::shouldRenameUses(KDevelop::Declaration* declaration) const
{
    // Now we know we're editing a declaration, but some declarations we don't offer a rename for
    // basically that's any declaration that wouldn't be fully renamed just by renaming its uses().
    if (declaration->internalContext() || declaration->isForwardDeclaration()) {
        //make an exception for non-class functions
        if (!declaration->isFunctionDeclaration() || dynamic_cast<ClassFunctionDeclaration*>(declaration))
            return false;
    }
    return true;
}

QString BasicRefactoring::newFileName(const QUrl& current, const QString& newName)
{
    QPair<QString, QString> nameExtensionPair = splitFileAtExtension(current.fileName());
    // if current file is lowercased, keep that
    if (nameExtensionPair.first == nameExtensionPair.first.toLower()) {
        return newName.toLower() + nameExtensionPair.second;
    } else {
        return newName + nameExtensionPair.second;
    }
}

DocumentChangeSet::ChangeResult BasicRefactoring::addRenameFileChanges(const QUrl& current,
                                                                       const QString& newName,
                                                                       DocumentChangeSet* changes)
{
    return changes->addDocumentRenameChange(
        IndexedString(current), IndexedString(newFileName(current, newName)));
}

bool BasicRefactoring::shouldRenameFile(Declaration* declaration)
{
    // only try to rename files when we renamed a class/struct
    if (!dynamic_cast<ClassDeclaration*>(declaration)) {
        return false;
    }
    const QUrl currUrl = declaration->topContext()->url().toUrl();
    const QString fileName = currUrl.fileName();
    const QPair<QString, QString> nameExtensionPair = splitFileAtExtension(fileName);
    // check whether we renamed something that is called like the document it lives in
    return nameExtensionPair.first.compare(declaration->identifier().toString(), Qt::CaseInsensitive) == 0;
}

DocumentChangeSet::ChangeResult BasicRefactoring::applyChanges(const QString& oldName, const QString& newName,
                                                               DocumentChangeSet& changes, DUContext* context,
                                                               int usedDeclarationIndex)
{
    if (usedDeclarationIndex == std::numeric_limits<int>::max())
        return DocumentChangeSet::ChangeResult::successfulResult();

    for (int a = 0; a < context->usesCount(); ++a) {
        const Use& use(context->uses()[a]);
        if (use.m_declarationIndex != usedDeclarationIndex)
            continue;
        if (use.m_range.isEmpty()) {
            qCDebug(LANGUAGE) << "found empty use";
            continue;
        }
        DocumentChangeSet::ChangeResult result =
            changes.addChange(DocumentChange(context->url(), context->transformFromLocalRevision(use.m_range), oldName,
                                             newName));
        if (!result)
            return result;
    }

    const auto childContexts = context->childContexts();
    for (DUContext* child : childContexts) {
        DocumentChangeSet::ChangeResult result = applyChanges(oldName, newName, changes, child, usedDeclarationIndex);
        if (!result)
            return result;
    }

    return DocumentChangeSet::ChangeResult::successfulResult();
}

DocumentChangeSet::ChangeResult BasicRefactoring::applyChangesToDeclarations(const QString& oldName,
                                                                             const QString& newName,
                                                                             DocumentChangeSet& changes,
                                                                             const QList<IndexedDeclaration>& declarations)
{
    for (auto& decl : declarations) {
        Declaration* declaration = decl.data();
        if (!declaration)
            continue;
        if (declaration->range().isEmpty())
            qCDebug(LANGUAGE) << "found empty declaration";

        TopDUContext* top = declaration->topContext();
        DocumentChangeSet::ChangeResult result =
            changes.addChange(DocumentChange(top->url(), declaration->rangeInCurrentRevision(), oldName, newName));
        if (!result)
            return result;
    }

    return DocumentChangeSet::ChangeResult::successfulResult();
}

KDevelop::IndexedDeclaration BasicRefactoring::declarationUnderCursor(bool allowUse)
{
    KTextEditor::View* view = ICore::self()->documentController()->activeTextDocumentView();
    if (!view)
        return KDevelop::IndexedDeclaration();
    KTextEditor::Document* doc = view->document();

    DUChainReadLocker lock;
    if (allowUse)
        return DUChainUtils::itemUnderCursor(doc->url(), KTextEditor::Cursor(view->cursorPosition())).declaration;
    else
        return DUChainUtils::declarationInLine(KTextEditor::Cursor(
                                                   view->cursorPosition()),
                                               DUChainUtils::standardContextForUrl(doc->url()));
}

void BasicRefactoring::startInteractiveRename(const KDevelop::IndexedDeclaration& decl)
{
    DUChainReadLocker lock(DUChain::lock());

    Declaration* declaration = decl.data();
    if (!declaration) {
        auto* message = new Sublime::Message(i18n("No declaration under cursor"), Sublime::Message::Error);
        ICore::self()->uiController()->postMessage(message);
        return;
    }
    QFileInfo info(declaration->topContext()->url().str());
    if (!info.isWritable()) {
        const QString messageText = i18n("Declaration is located in non-writable file %1.",
                                declaration->topContext()->url().str());
        auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
        ICore::self()->uiController()->postMessage(message);
        return;
    }

    QString originalName = declaration->identifier().identifier().str();
    lock.unlock();

    NameAndCollector nc = newNameForDeclaration(DeclarationPointer(declaration));

    if (nc.newName == originalName || nc.newName.isEmpty())
        return;

    renameCollectedDeclarations(nc.collector.data(), nc.newName, originalName);
}

bool BasicRefactoring::acceptForContextMenu(const Declaration* decl)
{
    // Default implementation. Some language plugins might override it to
    // handle some cases.
    Q_UNUSED(decl);
    return true;
}

void BasicRefactoring::executeRenameAction()
{
    auto* action = qobject_cast<QAction*>(sender());
    if (action) {
        IndexedDeclaration decl = action->data().value<IndexedDeclaration>();
        
        {
            DUChainReadLocker lock;
            
            if (!decl.isValid())
                decl = declarationUnderCursor();

            if (!decl.isValid())
                return;
        }

        startInteractiveRename(decl);
    }
}

BasicRefactoring::NameAndCollector BasicRefactoring::newNameForDeclaration(
    const KDevelop::DeclarationPointer& declaration)
{
    DUChainReadLocker lock;
    if (!declaration) {
        return {};
    }

    QSharedPointer<BasicRefactoringCollector> collector(new BasicRefactoringCollector(declaration.data()));

    Ui::RenameDialog renameDialog;
    QDialog dialog;
    renameDialog.setupUi(&dialog);

    UsesWidget uses(declaration.data(), collector);

    //So the context-links work
    auto* navigationWidget = declaration->context()->createNavigationWidget(declaration.data());
    if (navigationWidget)
        connect(&uses, &UsesWidget::navigateDeclaration, navigationWidget,
                &AbstractNavigationWidget::navigateDeclaration);

    QString declarationName = declaration->toString();
    dialog.setWindowTitle(i18nc("@title:window Renaming some declaration", "Rename \"%1\"", declarationName));
    renameDialog.edit->setText(declaration->identifier().identifier().str());
    renameDialog.edit->selectAll();

    renameDialog.tabWidget->addTab(&uses, i18nc("@title:tab", "Uses"));
    if (navigationWidget)
        renameDialog.tabWidget->addTab(navigationWidget, i18nc("@title:tab", "Declaration Info"));
    lock.unlock();

    if (dialog.exec() != QDialog::Accepted)
        return {};

    const auto text = renameDialog.edit->text().trimmed();
    RefactoringProgressDialog refactoringProgress(i18n("Renaming \"%1\" to \"%2\"", declarationName,
                                                       text), collector.data());
    if (!collector->isReady()) {
        if (refactoringProgress.exec() != QDialog::Accepted) { // krazy:exclude=crashy
            return {};
        }
    }

    //TODO: input validation
    return {
               text, collector
    };
}

DocumentChangeSet BasicRefactoring::renameCollectedDeclarations(KDevelop::BasicRefactoringCollector* collector,
                                                                const QString& replacementName,
                                                                const QString& originalName, bool apply)
{
    DocumentChangeSet changes;
    DUChainReadLocker lock;

    const auto allUsingContexts = collector->allUsingContexts();
    for (const KDevelop::IndexedTopDUContext collected : allUsingContexts) {
        QSet<int> hadIndices;
        const auto declarations = collector->declarations();
        for (const IndexedDeclaration decl : declarations) {
            uint usedDeclarationIndex = collected.data()->indexForUsedDeclaration(decl.data(), false);
            if (!Algorithm::insert(hadIndices, usedDeclarationIndex).inserted) {
                continue;
            }

            DocumentChangeSet::ChangeResult result = applyChanges(originalName, replacementName, changes,
                                                                  collected.data(), usedDeclarationIndex);
            if (!result) {
                auto* message = new Sublime::Message(i18n("Failed to apply changes: %1", result.m_failureReason), Sublime::Message::Error);
                ICore::self()->uiController()->postMessage(message);
                return {};
            }
        }
    }

    DocumentChangeSet::ChangeResult result = applyChangesToDeclarations(originalName, replacementName, changes,
                                                                        collector->declarations());
    if (!result) {
        auto* message = new Sublime::Message(i18n("Failed to apply changes: %1", result.m_failureReason), Sublime::Message::Error);
        ICore::self()->uiController()->postMessage(message);
        return {};
    }

    ///We have to ignore failed changes for now, since uses of a constructor or of operator() may be created on "(" parens
    changes.setReplacementPolicy(DocumentChangeSet::IgnoreFailedChange);

    if (!apply) {
        return changes;
    }

    result = changes.applyAllChanges();
    if (!result) {
        auto* message = new Sublime::Message(i18n("Failed to apply changes: %1", result.m_failureReason), Sublime::Message::Error);
        ICore::self()->uiController()->postMessage(message);
    }

    return {};
}

//END: BasicRefactoring

#include "moc_basicrefactoring.cpp"
