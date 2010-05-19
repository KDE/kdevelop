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
#include "createclass.h"

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
    OverridesPagePrivate(ClassGenerator* g)
        : generator(g), overrides(0)
    {
    }
    ClassGenerator* generator;
    Ui::OverridesDialog* overrides;
    QMultiHash<Identifier, DeclarationPointer> overriddenFunctions;
    QMap<QTreeWidgetItem *, DeclarationPointer> declarationMap;
    QList<DeclarationPointer> chosenOverrides;
};

OverridesPage::OverridesPage(ClassGenerator* generator, QWizard* parent)
    : QWizardPage(parent)
    , d(new OverridesPagePrivate(generator))
{
    setTitle(i18n("Override Methods"));
    setSubTitle( i18n("Select any methods you would like to override in the new class.") );

    d->overrides = new Ui::OverridesDialog;
    d->overrides->setupUi(this);

    connect(d->overrides->selectAllPushButton, SIGNAL(pressed()), this, SLOT(selectAll()));
    connect(d->overrides->deselectAllPushButton, SIGNAL(pressed()), this, SLOT(deselectAll()));
}

OverridesPage::~OverridesPage()
{
    delete d;
}


void OverridesPage::initializePage()
{
    QWizardPage::initializePage();

    d->overriddenFunctions.clear();
    overrideTree()->clear();
    d->chosenOverrides.clear();
    d->declarationMap.clear();
    
    ClassGenerator * generator = dynamic_cast<CreateClassWizard *>(wizard())->generator();
    generator->clearDeclarations();

    
    foreach (const DeclarationPointer override, generator->declarations()) {
        d->chosenOverrides.append(override);
    }

    //Add All the virtual overridable classes to the treewidget
    populateOverrideTree(dynamic_cast<CreateClassWizard *>(wizard())->generator()->inheritanceList());

    overrideTree()->expandAll();
    overrideTree()->header()->resizeSections(QHeaderView::ResizeToContents);
}

bool OverridesPage::validatePage()
{
    ClassGenerator * gen = dynamic_cast<CreateClassWizard *>(wizard())->generator();
    gen->clearDeclarations();

    for (int i = 0; i < d->overrides->overridesTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = d->overrides->overridesTree->topLevelItem(i);
        for (int j = 0; j < item->childCount(); ++j) {
            QTreeWidgetItem* child = item->child(j);
            if (child->checkState(0) == Qt::Checked)
                gen->addDeclaration(d->declarationMap[child]);//TODO add overrides to the generator
        }
    }

    return true;
}

void OverridesPage::cleanupPage()
{
    kDebug();

    validatePage();
}

void OverridesPage::populateOverrideTree(const QList<DeclarationPointer> & baseList)
{
    KDevelop::DUChainReadLocker lock(DUChain::lock());
    
    foreach(const DeclarationPointer baseClass, baseList)
    {
        DUContext* context = baseClass->internalContext();

        QTreeWidgetItem* classItem = new QTreeWidgetItem(overrideTree(), QStringList() << baseClass->qualifiedIdentifier().toString());
        classItem->setIcon(0, DUChainUtils::iconForDeclaration(baseClass.data()));

        //For this internal context get all the function declarations inside the class
        foreach (Declaration * childDeclaration, context->localDeclarations())
        {
            AbstractFunctionDeclaration * func;
            if (childDeclaration->type<FunctionType>() &&
                (func = dynamic_cast<AbstractFunctionDeclaration*>(childDeclaration)) &&
                func->isVirtual())
            {
                // Its a virtual function, add it to the list
                addPotentialOverride(classItem, DeclarationPointer(childDeclaration));
            }
        }
    }
}

void OverridesPage::addPotentialOverride(QTreeWidgetItem* classItem, DeclarationPointer childDeclaration)
{
    if (d->overriddenFunctions.contains(childDeclaration->identifier())) {
        foreach (DeclarationPointer decl, d->overriddenFunctions.values(childDeclaration->identifier()))
            if (decl->indexedType() == childDeclaration->indexedType())
                // This signature is already shown somewhere else
                return;
    }

    d->overriddenFunctions.insert(childDeclaration->identifier(), childDeclaration);

    QString accessModifier;
    if (ClassMemberDeclaration* member = dynamic_cast<ClassMemberDeclaration*>(childDeclaration.data())) {
        switch (member->accessPolicy()) {
            case Declaration::DefaultAccess:
            case Declaration::Public:
                accessModifier = i18n("Public");
                break;

            case Declaration::Protected:
                accessModifier = i18n("Protected");
                break;

            case Declaration::Private:
                accessModifier = i18n("Private");
                return;
        }
    }

    QTreeWidgetItem* overrideItem = new QTreeWidgetItem(classItem, QStringList() << childDeclaration->toString());
    overrideItem->setFlags( Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable) );
    overrideItem->setCheckState( 0, d->chosenOverrides.contains(childDeclaration) ? Qt::Checked : Qt::Unchecked );
    overrideItem->setIcon(0, DUChainUtils::iconForDeclaration(childDeclaration.data()));
    overrideItem->setData(0, Qt::UserRole, QVariant::fromValue(IndexedDeclaration(childDeclaration.data())));
    overrideItem->setText(1, accessModifier);

    if (ClassFunctionDeclaration* function = dynamic_cast<ClassFunctionDeclaration*>(childDeclaration.data())) {
        overrideItem->setCheckState( 2, function->isSignal() ? Qt::Checked : Qt::Unchecked );
        overrideItem->setCheckState( 3, function->isSlot() ? Qt::Checked : Qt::Unchecked );
    }
    
    ClassFunctionDeclaration* classFunction = dynamic_cast<ClassFunctionDeclaration*>(childDeclaration.data());
    if(classFunction && classFunction->isAbstract()) {
        overrideItem->setIcon(0, KIcon("flag-red"));
        overrideItem->setCheckState(0, Qt::Checked);
        overrideItem->setText(0, overrideItem->text(0) + " = 0");///@todo this is C++ specific
        classItem->removeChild(overrideItem);
        classItem->insertChild(0, overrideItem);
    }
    
    d->declarationMap[overrideItem] = childDeclaration;
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

ClassGenerator* OverridesPage::generator() const
{
    return d->generator;
}


#include "overridespage.moc"
