/*
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_PLUGIN_OVERRIDESPAGE_H
#define KDEVPLATFORM_PLUGIN_OVERRIDESPAGE_H

#include <QWidget>

#include "language/duchain/declaration.h"
#include "ipagefocus.h"

class QTreeWidget;
class QTreeWidgetItem;

namespace KDevelop {

/**
 * Assistant page for choosing class functions, overridden from base classes. 
 */
class OverridesPage : public QWidget, public IPageFocus
{
    Q_OBJECT

public:
    explicit OverridesPage(QWidget* parent);
    ~OverridesPage() override;

    /**
     * Default implementation populates the tree with all virtual functions in the base classes.
     * Calls @c addPotentialOverride() on each function, where more filtering can be applied.
     *
     * @param directBases Declarations of base classes from which the new class inherits directly.
     * @param allBases Declarations of all base classes from which functions can be overridden
     */
    virtual void addBaseClasses(const QList<DeclarationPointer>& directBases,
                                const QList<DeclarationPointer>& allBases);
    /**
     * Add @p childDeclaration as potential override.
     *
     * Don't call @c KDevelop::OverridesPage::addPotentialOverride() in overloaded
     * class to filter a declaration.
     *
     * @p classItem The parent class from which @p childDeclaration stems from.
     *              Should be used as parent for the override item.
     * @p childDeclaration The overridable function.
     */
    virtual void addPotentialOverride(QTreeWidgetItem* classItem,
                                      const DeclarationPointer& childDeclaration);

    /**
     * Add @p declarations as potential overrides under the category @p category.
     *
     * The DUChain must be locked for reading before calling this function
     *
     * @param category the user-visible category name
     * @param declarations a list of declarations that can be overridden or implemented in the new class
     */
    void addCustomDeclarations(const QString& category, const QList< KDevelop::DeclarationPointer >& declarations);

    QList<DeclarationPointer> selectedOverrides() const;

    void clear();

    QTreeWidget* overrideTree() const;

    QWidget* extraFunctionsContainer() const;

    void setFocusToFirstEditWidget() override;

public Q_SLOTS:
    /**
     * Selects all functions for overriding
     */
    virtual void selectAll();
    /**
     * Deselects all potential overrides
     */
    virtual void deselectAll();

private:
    struct OverridesPagePrivate* const d;
};

}

#endif // KDEVPLATFORM_PLUGIN_OVERRIDESPAGE_H
