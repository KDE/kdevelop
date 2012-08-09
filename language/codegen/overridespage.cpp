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
#include "classgenerator.h"

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

struct KDevelop::OverridesPagePrivate
{
    OverridesPagePrivate()
        : overrides(0)
    {
    }
    Ui::OverridesDialog* overrides;
    QMultiHash<Identifier, DeclarationPointer> overriddenFunctions;
    QMap<QTreeWidgetItem *, DeclarationPointer> declarationMap;
    QList<DeclarationPointer> chosenOverrides;
};

OverridesPage::OverridesPage(QWidget* parent)
    : QWidget(parent)
    , d(new OverridesPagePrivate)
{
    d->overrides = new Ui::OverridesDialog;
    d->overrides->setupUi(this);

    connect(d->overrides->selectAllPushButton, SIGNAL(pressed()), this, SLOT(selectAll()));
    connect(d->overrides->deselectAllPushButton, SIGNAL(pressed()), this, SLOT(deselectAll()));
}

OverridesPage::~OverridesPage()
{
    delete d;
}

QList< DeclarationPointer > OverridesPage::selectedOverrides()
{
    QList<DeclarationPointer> declarations;

    for (int i = 0; i < d->overrides->overridesTree->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* item = d->overrides->overridesTree->topLevelItem(i);
        for (int j = 0; j < item->childCount(); ++j)
        {
            QTreeWidgetItem* child = item->child(j);
            if (child->checkState(0) == Qt::Checked)
            {
                kDebug() << "Adding declaration" << d->declarationMap[child]->toString();
                declarations << d->declarationMap[child];
            }
        }
    }

    kDebug() << declarations.size();
    return declarations;
}

void OverridesPage::clear()
{
    d->overriddenFunctions.clear();
    overrideTree()->clear();
    d->chosenOverrides.clear();
    d->declarationMap.clear();
}

void OverridesPage::addBaseClasses(const QList<DeclarationPointer>& directBases, const QList<DeclarationPointer>& allBases)
{
    KDevelop::DUChainReadLocker lock(DUChain::lock());

    foreach(const DeclarationPointer& baseClass, allBases) {
        DUContext* context = baseClass->internalContext();

        QTreeWidgetItem* classItem = new QTreeWidgetItem(overrideTree(), QStringList() << baseClass->qualifiedIdentifier().toString());
        classItem->setIcon(0, DUChainUtils::iconForDeclaration(baseClass.data()));

        //For this internal context get all the function declarations inside the class
        foreach (Declaration * childDeclaration, context->localDeclarations()) {
            if (AbstractFunctionDeclaration * func = dynamic_cast<AbstractFunctionDeclaration*>(childDeclaration))
            {
                if (func->isVirtual())
                {
                    // Its a virtual function, add it to the list unless it's a destructor
                    ClassFunctionDeclaration* cFunc = dynamic_cast<ClassFunctionDeclaration*>(childDeclaration);
                    if (cFunc && !cFunc->isDestructor())
                    {
                        addPotentialOverride(classItem, DeclarationPointer(childDeclaration));
                    }
                }
                else if (directBases.contains(baseClass))
                {
                    // add ctors of direct parents
                    ClassFunctionDeclaration* cFunc = dynamic_cast<ClassFunctionDeclaration*>(childDeclaration);
                    if (cFunc && cFunc->isConstructor())
                    {
                        addPotentialOverride(classItem, DeclarationPointer(childDeclaration));
                    }
                }
            }
        }
    }

    overrideTree()->expandAll();
    overrideTree()->header()->resizeSections(QHeaderView::ResizeToContents);
}

void OverridesPage::addPotentialOverride(QTreeWidgetItem* classItem, const DeclarationPointer& childDeclaration)
{
    kDebug() << childDeclaration->toString();
    if (d->overriddenFunctions.contains(childDeclaration->identifier()))
    {
        foreach (DeclarationPointer decl, d->overriddenFunctions.values(childDeclaration->identifier()))
        {
            if (decl->indexedType() == childDeclaration->indexedType())
            {
                kDebug() << "Declaration is already shown";
                return;
            }
        }
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
                kDebug() << "Declaration is private, returning";
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

void OverridesPage::addCustomDeclarations (const QString& category, const QList<DeclarationPointer>& declarations)
{
    kDebug() << category << declarations.size();
    DUChainReadLocker lock(DUChain::lock());

    QTreeWidgetItem* item = new QTreeWidgetItem(overrideTree(), QStringList() << category);
    foreach (const DeclarationPointer& declaration, declarations)
    {
        addPotentialOverride(item, declaration);
    }

    overrideTree()->expandAll();
    overrideTree()->header()->resizeSections(QHeaderView::ResizeToContents);
}


#include "overridespage.moc"
