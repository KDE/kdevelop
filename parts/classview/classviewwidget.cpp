/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classviewwidget.h"

#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kinstance.h>
#include <kpopupmenu.h>

#include "kdevlanguagesupport.h"

#include "classstore.h"
#include "classtooldlg.h"
#include "classviewpart.h"


ClassViewWidget::ClassViewWidget(ClassViewPart *part)
    : ClassTreeBase(part, 0, "class tree widget")
{
    connect( part, SIGNAL(setLanguageSupport(KDevLanguageSupport*)),
             this, SLOT(setLanguageSupport(KDevLanguageSupport*)) );
}


ClassViewWidget::~ClassViewWidget()
{}


KPopupMenu *ClassViewWidget::createPopup()
{
    KPopupMenu *popup = contextItem? contextItem->createPopup() : 0;
    if (!popup) {
        popup = new KPopupMenu();
        popup->insertTitle(i18n("Class View"), -1, 0);
    }

    popup->setCheckable(true);
    int id1 = popup->insertItem( i18n("List by Namespaces"), this, SLOT(slotTreeModeChanged()) );
    int id2 = popup->insertItem( i18n("Full Identifier Scopes"), this, SLOT(slotScopeModeChanged()) );
    KConfig *config = ClassViewFactory::instance()->config();
    config->setGroup("General");
    bool byNamespace = config->readBoolEntry("ListByNamespace", false);
    popup->setItemChecked(id1, byNamespace);
    bool identifierScopes = config->readBoolEntry("FullIdentifierScopes", false);
    popup->setItemChecked(id2, identifierScopes);

    return popup;
}


void ClassViewWidget::setLanguageSupport(KDevLanguageSupport *ls)
{
    if (ls)
        connect(ls, SIGNAL(updatedSourceInfo()), this, SLOT(refresh()));
    else
        refresh();
}


void ClassViewWidget::slotTreeModeChanged()
{
    KConfig *config = ClassViewFactory::instance()->config();
    config->setGroup("General");
    config->writeEntry("ListByNamespace", !config->readBoolEntry("ListByNamespace"));
    buildTree(true);
}


void ClassViewWidget::slotScopeModeChanged()
{
    KConfig *config = ClassViewFactory::instance()->config();
    config->setGroup("General");
    config->writeEntry("FullIdentifierScopes", !config->readBoolEntry("FullIdentifierScopes"));
    buildTree(false);
}


void ClassViewWidget::refresh()
{
    buildTree(false);
}


void ClassViewWidget::buildTree(bool fromScratch)
{
    if (!m_part->languageSupport())
        return;
    
    KConfig *config = ClassViewFactory::instance()->config();
    config->setGroup("General");
    if (config->readBoolEntry("ListByNamespace", false))
        buildTreeByNamespace(fromScratch);
    else
        buildTreeByCategory(fromScratch);
}


void ClassViewWidget::buildTreeByCategory(bool fromScratch)
{
    TreeState oldTreeState;
    if (!fromScratch)
        oldTreeState = treeState();
    
    clear();
    
    ParsedScopeContainer *scope = &m_part->classStore()->globalContainer;
    
    ClassTreeItem *ilastItem, *lastItem = 0;

    KDevLanguageSupport::Features features = m_part->languageSupport()->features();

    if (features & KDevLanguageSupport::Classes) {
        // Add classes
        lastItem = new ClassTreeOrganizerItem(this, lastItem, i18n("Classes"));
        ilastItem = 0;
        QValueList<ParsedClass*> classList = scope->getSortedClassList();
        QValueList<ParsedClass*>::ConstIterator it;
        for (it = classList.begin(); it != classList.end(); ++it)
            ilastItem = new ClassTreeClassItem(lastItem, ilastItem, *it);
        if (fromScratch)
            lastItem->setOpen(true);
    }
    
    if (features & KDevLanguageSupport::Structs) {
        // Add structs
        lastItem = new ClassTreeOrganizerItem(this, lastItem, i18n("Structs"));
        ilastItem = 0;
        QValueList<ParsedStruct*> structList = scope->getSortedStructList();
        QValueList<ParsedStruct*>::ConstIterator it;
        for (it = structList.begin(); it != structList.end(); ++it)
            ilastItem = new ClassTreeStructItem(lastItem, ilastItem, *it);
        if (fromScratch)
            lastItem->setOpen(true);
    }
                                 
    if (features & KDevLanguageSupport::Functions) {
        // Add functions
        lastItem = new ClassTreeOrganizerItem(this, lastItem, i18n("Functions"));
        ilastItem = 0;
        QValueList<ParsedMethod*> methodList = scope->getSortedMethodList();
        QValueList<ParsedMethod*>::ConstIterator it;
        for (it = methodList.begin(); it != methodList.end(); ++it)
            ilastItem = new ClassTreeMethodItem(lastItem, ilastItem, *it);
        if (fromScratch)
            lastItem->setOpen(true);
    }

    if (features & KDevLanguageSupport::Variables) {
        // Add attributes
        lastItem = new ClassTreeOrganizerItem(this, lastItem, i18n("Variables"));
        ilastItem = 0;
        QValueList<ParsedAttribute*> attrList = scope->getSortedAttributeList();
        QValueList<ParsedAttribute*>::ConstIterator it;
        for (it = attrList.begin(); it != attrList.end(); ++it)
            ilastItem = new ClassTreeAttrItem(lastItem, ilastItem, *it);
        if (fromScratch)
            lastItem->setOpen(true);
    }
    
    if (features & KDevLanguageSupport::Namespaces) {
        // Add namespaces
        lastItem = new ClassTreeOrganizerItem(this, lastItem, i18n("Namespaces"));
        ilastItem = 0;
        QValueList<ParsedScopeContainer*> scopeList = scope->getSortedScopeList();
        QValueList<ParsedScopeContainer*>::ConstIterator it;
        for (it = scopeList.begin(); it != scopeList.end(); ++it)
            ilastItem = new ClassTreeScopeItem(lastItem, ilastItem, *it);
        if (fromScratch)
            lastItem->setOpen(true);
    }

    if (!fromScratch)
        setTreeState(oldTreeState);
}


void ClassViewWidget::buildTreeByNamespace(bool fromScratch)
{
    TreeState oldTreeState;
    if (!fromScratch)
        oldTreeState = treeState();

    clear();

    ClassTreeItem *lastItem = 0;
    
    // Global namespace?
    
    QValueList<ParsedScopeContainer*> scopeList = m_part->classStore()->globalContainer.getSortedScopeList();
    QValueList<ParsedScopeContainer*>::ConstIterator it;
    for (it = scopeList.begin(); it != scopeList.end(); ++it) {
        lastItem = new ClassTreeScopeItem(this, lastItem, *it);
        if (fromScratch)
            lastItem->setOpen(true);
    }

    if (!fromScratch)
        setTreeState(oldTreeState);
}

#include "classviewwidget.moc"
