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


#ifndef BASICREFACTORING_H_
#define BASICREFACTORING_H_


#include <QtCore/QObject>
#include <QSharedPointer>
#include <language/languageexport.h>
#include <language/codegen/documentchangeset.h>
#include <language/duchain/navigation/useswidget.h>


namespace KDevelop
{
class ContextMenuExtension;
class IndexedDeclaration;
class Context;
class Declaration;
class DUContext;

/**
 * A widget that show the uses that it has collected for
 * the given declaration.
 */
class KDEVPLATFORMLANGUAGE_EXPORT BasicRefactoringCollector : public UsesWidget::UsesWidgetCollector
{
public:
    BasicRefactoringCollector(const IndexedDeclaration &decl);
    QVector<IndexedTopDUContext> allUsingContexts() const;

protected:
    /// Process the uses for the given TopDUContext.
    virtual void processUses(KDevelop::ReferencedTopDUContext topContext) override;

private:
    QVector<IndexedTopDUContext> m_allUsingContexts;
};


/// The base class for Refactoring classes from Language plugins.
class KDEVPLATFORMLANGUAGE_EXPORT BasicRefactoring : public QObject
{
    Q_OBJECT

public:
    explicit BasicRefactoring(QObject *parent = NULL);

    /// Update the context menu with the "Rename" action.
    virtual void fillContextMenu(KDevelop::ContextMenuExtension &extension, KDevelop::Context *context);

    struct NameAndCollector {
        QString newName;
        QSharedPointer<BasicRefactoringCollector> collector;
    };

protected:
    /**
     * Apply the changes to the uses that can be found inside the given
     * context and its children.
     * NOTE: the DUChain must be locked.
     */
    virtual DocumentChangeSet::ChangeResult applyChanges(const QString &oldName, const QString &newName,
                                                         DocumentChangeSet &changes, DUContext *context,
                                                         int usedDeclarationIndex);

    /**
     * Apply the changes to the given declarations.
     * NOTE: the DUChain must be locked.
     */
    virtual DocumentChangeSet::ChangeResult applyChangesToDeclarations(const QString &oldName, const QString &newName,
                                                                       DocumentChangeSet &changes,
                                                                       const QList<IndexedDeclaration> &declarations);

    /**
     * Get the declaration under the current position of the cursor.
     *
     * @param allowUse Set to false if the declarations to be returned
     * cannot come from uses.
     */
    virtual IndexedDeclaration declarationUnderCursor(bool allowUse = true);

    /**
     * Start the renaming of a declaration.
     * This function retrieves the new name for declaration @p decl and if approved renames all instances of it.
     */
    virtual void startInteractiveRename(const KDevelop::IndexedDeclaration &decl);

    /**
     * Asks user to input a new name for @p declaration
     * @return new name or empty string if user changed his mind or new name contains inappropriate symbols (e.g. spaces, points, braces e.t.c) and the collector used for collecting information about @p declaration.
     * NOTE: unlock the DUChain before calling this.
     */
    virtual BasicRefactoring::NameAndCollector newNameForDeclaration(const KDevelop::DeclarationPointer& declaration);

    /**
    * Renames all declarations collected by @p collector from @p oldName to @p newName
    * @param apply - if changes should be applied immediately
    * @return all changes if @p apply is false and empty set otherwise.
    */
    DocumentChangeSet renameCollectedDeclarations(KDevelop::BasicRefactoringCollector* collector, const QString& replacementName, const QString& originalName, bool apply = true);

    /**
     * @returns true if we can show the interactive rename widget for the
     * given declaration. The default implementation just returns true.
     */
    virtual bool acceptForContextMenu(const Declaration *decl);

private slots:
    void executeRenameAction();
};

} // End of namespace KDevelop

#endif /* BASICREFACTORING_H_ */
