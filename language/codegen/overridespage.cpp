/*
   Copyright 2008 Hamish Rodda <rodda@kde.org>

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

#include "overridespage.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>

#include "../duchain/duchain.h"
#include "../duchain/persistentsymboltable.h"
#include "../duchain/duchainlock.h"
#include "../duchain/types/alltypes.h"
#include "../duchain/classmemberdeclaration.h"
#include "../duchain/abstractfunctiondeclaration.h"
#include "../duchain/duchainutils.h"
#include "../duchain/classfunctiondeclaration.h"

#include "ui_overridevirtuals.h"

using namespace KDevelop;

class KDevelop::OverridesPagePrivate
{
public:
    OverridesPagePrivate()
        : overrides(0)
    {
    }

    Ui::OverridesDialog* overrides;
    QList<KDevelop::Declaration*> overrideSuperclasses;
    QVariantList selectedOverrides;
};

OverridesPage::OverridesPage(QWizard* parent)
    : QWizardPage(parent)
    , d(new OverridesPagePrivate)
{
    setTitle(i18n("Override Methods"));
    setSubTitle( i18n("Select any methods you would like to override in the new class.") );

    d->overrides = new Ui::OverridesDialog;
    d->overrides->setupUi(this);

    connect(d->overrides->selectAllPushButton, SIGNAL(pressed()), this, SLOT(selectAll()));
    connect(d->overrides->deselectAllPushButton, SIGNAL(pressed()), this, SLOT(deselectAll()));

    registerField("overrides", this, "overrides");
}

OverridesPage::~OverridesPage()
{
    delete d;
}

QList< QVariant > OverridesPage::selectedOverrides() const
{
    return d->selectedOverrides;
}

void OverridesPage::initializePage()
{
    QWizardPage::initializePage();

    d->overrideSuperclasses.clear();
    overrideTree()->clear();

    foreach (const QString& inherited, field("classInheritance").toStringList())
        fetchInheritance(inherited);

    overrideTree()->expandAll();
    overrideTree()->header()->resizeSections(QHeaderView::ResizeToContents);
}

bool OverridesPage::validatePage()
{
    d->selectedOverrides.clear();

    for (int i = 0; i < d->overrides->overridesTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = d->overrides->overridesTree->topLevelItem(i);
        for (int j = 0; j < item->childCount(); ++j) {
            QTreeWidgetItem* child = item->child(j);
            if (child->checkState(0) == Qt::Checked)
                d->selectedOverrides.append(child->data(0, Qt::UserRole));
        }
    }

    kDebug() << "Found" << d->selectedOverrides.count() << "requested overrides.";

    return true;
}

void OverridesPage::fetchInheritance(const QString& inheritedObject)
{
    // TODO: properly strip qualifiers
    QString identifier = inheritedObject;
    identifier = identifier.remove("public ", Qt::CaseInsensitive).remove("protected ", Qt::CaseInsensitive).remove("private ", Qt::CaseInsensitive).simplified();

    KDevelop::DUChainReadLocker lock( DUChain::lock(), 100 );
    if(!lock.locked()) {
      kDebug() << "Failed to lock du-chain in time";
      return;
    }

    PersistentSymbolTable::Declarations declarations = PersistentSymbolTable::self().getDeclarations( IndexedQualifiedIdentifier( QualifiedIdentifier(identifier)) );

    for (PersistentSymbolTable::Declarations::Iterator it = declarations.iterator(); it; ++it) {
        Declaration* decl = it->declaration();
        if (decl->isForwardDeclaration())
            continue;

        // Check if it's a class/struct/etc
        if (decl->type<StructureType>()) {
            fetchInheritanceFromClass(decl);
        }
    }
}

void OverridesPage::fetchInheritanceFromClass(KDevelop::Declaration* decl)
{
    // Prevent recursion / duplication
    if (d->overrideSuperclasses.contains(decl))
        return;

    d->overrideSuperclasses.append(decl);

    DUContext* context = decl->internalContext();
    QTreeWidgetItem* classItem = new QTreeWidgetItem(overrideTree(), QStringList() << decl->qualifiedIdentifier().toString());
    classItem->setIcon(0, DUChainUtils::iconForDeclaration(decl));

    foreach (Declaration* childDeclaration, context->localDeclarations()) {
        if (childDeclaration->type<FunctionType>()) {
            // We have a child function
            if (AbstractFunctionDeclaration* func = dynamic_cast<AbstractFunctionDeclaration*>(childDeclaration)) {
                if (func->isVirtual()) {
                    // It's virtual, add it to the list
                    addPotentialOverride(classItem, childDeclaration);
                }
            }
        }
    }


    foreach (const DUContext::Import& import, context->importedParentContexts())
        if (DUContext* parentContext = import.context(context->topContext()))
            if (parentContext->type() == DUContext::Class)
                fetchInheritanceFromClass( parentContext->owner() );
}

void OverridesPage::addPotentialOverride(QTreeWidgetItem* classItem, KDevelop::Declaration* childDeclaration)
{
    QString accessModifier;
    if (ClassMemberDeclaration* member = dynamic_cast<ClassMemberDeclaration*>(childDeclaration)) {
        switch (member->accessPolicy()) {
            case Declaration::Public:
                accessModifier = i18n("Public");
                break;

            case Declaration::Protected:
                accessModifier = i18n("Protected");
                break;

            case Declaration::Private:
                // You can't override a private virtual in a superclass
                return;
        }
    }

    QTreeWidgetItem* overrideItem = new QTreeWidgetItem(classItem, QStringList() << childDeclaration->toString());
    overrideItem->setFlags( Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable) );
    overrideItem->setCheckState( 0, Qt::Unchecked );
    overrideItem->setIcon(0, DUChainUtils::iconForDeclaration(childDeclaration));
    overrideItem->setData(0, Qt::UserRole, QVariant::fromValue(IndexedDeclaration(childDeclaration)));
    overrideItem->setText(1, accessModifier);

    if (ClassFunctionDeclaration* function = dynamic_cast<ClassFunctionDeclaration*>(childDeclaration)) {
        overrideItem->setCheckState( 2, function->isSignal() ? Qt::Checked : Qt::Unchecked );
        overrideItem->setCheckState( 3, function->isSlot() ? Qt::Checked : Qt::Unchecked );
    }
}

QTreeWidget* OverridesPage::overrideTree() const
{
    return d->overrides->overridesTree;
}

QWidget* OverridesPage::extraFunctionsContainer() const
{
    return d->overrides->extraFunctionWidget;
}

void OverridesPage::selectAll()
{
    for (int i = 0; i < d->overrides->overridesTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = d->overrides->overridesTree->topLevelItem(i);
        for (int j = 0; j < item->childCount(); ++j)
            item->child(j)->setCheckState(0, Qt::Checked);
    }
}

void OverridesPage::deselectAll()
{
    for (int i = 0; i < d->overrides->overridesTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = d->overrides->overridesTree->topLevelItem(i);
        for (int j = 0; j < item->childCount(); ++j)
            item->child(j)->setCheckState(0, Qt::Unchecked);
    }
}

#include "overridespage.moc"
