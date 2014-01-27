/* This file is part of KDevelop
 *
 * Copyright 2014 Miquel Sabaté <mikisabate@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


// Qt
#include <QtGui/QAction>
#include <QtGui/QTabWidget>
#include <QtGui/QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPushButton>

// KDE / KDevelop
#include <KParts/MainWindow>
#include <KTextEditor/Document>
#include <kmessagebox.h>
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
#include <duchain/use.h>


namespace KDevelop
{

//BEGIN: BasicRefactoringCollector

BasicRefactoringCollector::BasicRefactoringCollector(const IndexedDeclaration &decl)
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

BasicRefactoring::BasicRefactoring(QObject *parent)
    : QObject(parent)
{
    /* There's nothing to do here. */
}

void BasicRefactoring::fillContextMenu(ContextMenuExtension &extension, Context *context)
{
    DeclarationContext *declContext = dynamic_cast<DeclarationContext *>(context);
    if (!declContext)
        return;

    DUChainReadLocker lock;
    Declaration *declaration = declContext->declaration().data();
    if (declaration && acceptForContextMenu(declaration)) {
        QFileInfo finfo(declaration->topContext()->url().str());
        if (finfo.isWritable()) {
            QAction *action = new QAction(i18n("Rename \"%1\"...", declaration->qualifiedIdentifier().toString()), 0);
            action->setData(QVariant::fromValue(IndexedDeclaration(declaration)));
            action->setIcon(KIcon("edit-rename"));
            connect(action, SIGNAL(triggered(bool)), this, SLOT(executeRenameAction()));
            extension.addAction(ContextMenuExtension::RefactorGroup, action);
        }
    }
}

DocumentChangeSet::ChangeResult BasicRefactoring::applyChanges(const QString &oldName, const QString &newName,
                                                               DocumentChangeSet &changes, DUContext *context,
                                                               int usedDeclarationIndex)
{
    if (usedDeclarationIndex == std::numeric_limits<int>::max())
        return DocumentChangeSet::ChangeResult(true);

    for (int a = 0; a < context->usesCount(); ++a) {
        const Use &use(context->uses()[a]);
        if (use.m_declarationIndex != usedDeclarationIndex)
            continue;
        if (use.m_range.isEmpty()) {
            kDebug() << "found empty use";
            continue;
        }
        DocumentChangeSet::ChangeResult result = changes.addChange(DocumentChange(context->url(), context->transformFromLocalRevision(use.m_range), oldName, newName));
        if (!result)
            return result;
    }

    foreach (DUContext *child, context->childContexts()) {
        DocumentChangeSet::ChangeResult result = applyChanges(oldName, newName, changes, child, usedDeclarationIndex);
        if (!result)
            return result;
    }
    return DocumentChangeSet::ChangeResult(true);
}

DocumentChangeSet::ChangeResult BasicRefactoring::applyChangesToDeclarations(const QString &oldName,
                                                                             const QString &newName,
                                                                             DocumentChangeSet &changes,
                                                                             const QList<IndexedDeclaration> &declarations)
{
    foreach (const IndexedDeclaration &decl, declarations) {
        Declaration *declaration = decl.data();
        if (!declaration)
            continue;
        if (declaration->range().isEmpty())
            kDebug() << "found empty declaration";

        TopDUContext *top = declaration->topContext();
        DocumentChangeSet::ChangeResult result = changes.addChange(DocumentChange(top->url(), declaration->rangeInCurrentRevision(), oldName, newName));
        if (!result)
            return result;
    }
    return DocumentChangeSet::ChangeResult(true);
}

KDevelop::IndexedDeclaration BasicRefactoring::declarationUnderCursor(bool allowUse)
{
    KDevelop::IDocument *doc = ICore::self()->documentController()->activeDocument();
    if (doc && doc->textDocument() && doc->textDocument()->activeView()) {
        DUChainReadLocker lock;
        if (allowUse)
            return DUChainUtils::itemUnderCursor(doc->url(), SimpleCursor(doc->textDocument()->activeView()->cursorPosition()));
        else
            return DUChainUtils::declarationInLine(SimpleCursor(doc->textDocument()->activeView()->cursorPosition()), DUChainUtils::standardContextForUrl(doc->url()));
    }

    return KDevelop::IndexedDeclaration();
}

void BasicRefactoring::startInteractiveRename(const KDevelop::IndexedDeclaration &decl)
{
    DUChainReadLocker lock(DUChain::lock());

    Declaration *declaration = decl.data();
    if (!declaration) {
        KMessageBox::error(ICore::self()->uiController()->activeMainWindow(), i18n("No declaration under cursor"));
        return;
    }
    QFileInfo info(declaration->topContext()->url().str());
    if (!info.isWritable()) {
        KMessageBox::error(ICore::self()->uiController()->activeMainWindow(),
                           i18n("Declaration is located in non-writeable file %1.", declaration->topContext()->url().str()));
        return;
    }

    QString originalName = declaration->identifier().identifier().str();
    QString replacementName;

    BasicRefactoringCollector *collector = new BasicRefactoringCollector(decl);
    QPointer<QDialog> dialog = new QDialog();
    QTabWidget tabWidget;
    UsesWidget uses(declaration, collector);

    //So the context-links work
    QWidget *navigationWidget = declaration->context()->createNavigationWidget(declaration);
    AbstractNavigationWidget* abstractNavigationWidget = dynamic_cast<AbstractNavigationWidget*>(navigationWidget);
    if (abstractNavigationWidget)
        connect(&uses, SIGNAL(navigateDeclaration(KDevelop::IndexedDeclaration)), abstractNavigationWidget, SLOT(navigateDeclaration(KDevelop::IndexedDeclaration)));

    QVBoxLayout verticalLayout;
    QHBoxLayout actionsLayout;
    dialog->setLayout(&verticalLayout);
    dialog->setWindowTitle(i18n("Rename %1", declaration->toString()));

    QLabel newNameLabel(i18n("New name:"));
    actionsLayout.addWidget(&newNameLabel);

    QLineEdit edit(declaration->identifier().identifier().str());
    newNameLabel.setBuddy(&edit);

    actionsLayout.addWidget(&edit);
    edit.setText(originalName);
    edit.setFocus();
    edit.selectAll();
    QPushButton goButton(i18n("Rename"));
    goButton.setToolTip(i18n("Note: All overloaded functions, overloads, forward-declarations, etc. will be renamed too"));
    actionsLayout.addWidget(&goButton);
    connect(&goButton, SIGNAL(clicked(bool)), dialog, SLOT(accept()));

    QPushButton cancelButton(i18n("Cancel"));
    actionsLayout.addWidget(&cancelButton);
    verticalLayout.addLayout(&actionsLayout);

    tabWidget.addTab(&uses, i18n("Uses"));
    if (navigationWidget)
        tabWidget.addTab(navigationWidget, i18n("Declaration Info"));

    verticalLayout.addWidget(&tabWidget);

    connect(&cancelButton, SIGNAL(clicked(bool)), dialog, SLOT(reject()));

    lock.unlock();
    dialog->resize(750, 550);
    if (dialog->exec() != QDialog::Accepted)
        return;

    replacementName = edit.text();
    if (replacementName == originalName || replacementName.isEmpty())
        return;

    DocumentChangeSet changes;
    lock.lock();
    foreach (const KDevelop::IndexedTopDUContext &collected, collector->allUsingContexts()) {
        QSet<int> hadIndices;
        foreach (const IndexedDeclaration &decl, collector->declarations()) {
            uint usedDeclarationIndex = collected.data()->indexForUsedDeclaration(decl.data(), false);
            if (hadIndices.contains(usedDeclarationIndex))
                continue;
            hadIndices.insert(usedDeclarationIndex);
            DocumentChangeSet::ChangeResult result = applyChanges(originalName, replacementName, changes, collected.data(), usedDeclarationIndex);
            if (!result) {
                KMessageBox::error(0, i18n("Applying changes failed: %1", result.m_failureReason));
                return;
            }
        }
    }

    DocumentChangeSet::ChangeResult result = applyChangesToDeclarations(originalName, replacementName, changes, collector->declarations());
    if(!result) {
        KMessageBox::error(0, i18n("Applying changes failed: %1", result.m_failureReason));
        return;
    }

    ///We have to ignore failed changes for now, since uses of a constructor or of operator() may be created on "(" parens
    changes.setReplacementPolicy(DocumentChangeSet::IgnoreFailedChange);
    result = changes.applyAllChanges();
    if(!result) {
        KMessageBox::error(0, i18n("Applying changes failed: %1", result.m_failureReason));
        return;
    }
}

bool BasicRefactoring::acceptForContextMenu(const Declaration *decl)
{
    // Default implementation. Some language plugins might override it to
    // handle some cases.
    Q_UNUSED(decl);
    return true;
}

void BasicRefactoring::executeRenameAction()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        IndexedDeclaration decl = action->data().value<IndexedDeclaration>();
        if(!decl.isValid())
            decl = declarationUnderCursor();
        startInteractiveRename(decl);
    }
}

//END: BasicRefactoring

} // End of namespace KDevelop

#include "basicrefactoring.moc"
