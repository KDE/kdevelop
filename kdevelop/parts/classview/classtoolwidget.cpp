/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kpopupmenu.h>
#include "classstore.h"
#include "classtoolwidget.h"


ClassToolWidget::ClassToolWidget(ClassView *view, QWidget *parent)
    : ClassTreeBase(view, parent, "class tool widget")
{}


ClassToolWidget::~ClassToolWidget()
{}


KPopupMenu *ClassToolWidget::createPopup()
{
    KPopupMenu *popup = contextItem->createPopup();
    if (!popup) {
        popup = new KPopupMenu();
        popup->insertTitle(i18n("Class view"), -1, 0);
    }

    return popup;
}


void ClassToolWidget::insertClassAndClasses(ParsedClass *parsedClass, QList<ParsedClass> *classList)
{
    ClassTreeItem *root = new ClassTreeClassItem(this, 0, parsedClass);
    
    ClassTreeItem *lastItem = 0;
    
    for ( ParsedClass *pClass = classList->first();
          pClass != 0;
          pClass = classList->next() )
        {
            lastItem = new ClassTreeClassItem(root, lastItem, pClass);
            lastItem->setExpandable(false);
        }
    
    if (!root->firstChild())
        root->setExpandable(false);
    else
        root->setOpen(true);
}


void ClassToolWidget::insertClassAndClasses(ParsedClass *parsedClass, QList<ParsedParent> *parentList)
{
    ClassTreeItem *root = new ClassTreeClassItem(this, 0, parsedClass);
    
    ClassTreeItem *lastItem = 0;
    
    for( ParsedParent *pParent = parentList->first();
         pParent != 0;
         pParent = parentList->next() )
        {
            ParsedClass *parentClass = m_store->getClassByName(pParent->name);
            lastItem = new ClassTreeClassItem(root, lastItem, parentClass);
            lastItem->setExpandable(false);
        }
    
    if (!root->firstChild())
        root->setExpandable(false);
    else
        root->setOpen(true);
}


void ClassToolWidget::addClassAndAttributes(ParsedClass *parsedClass, PIExport filter, ClassTreeItem **lastItem)
{
    *lastItem = new ClassTreeClassItem(this, *lastItem, parsedClass);
    
    ClassTreeItem *ilastItem = 0;
    
    QList<ParsedAttribute> *attrList = parsedClass->getSortedAttributeList();
    for ( ParsedAttribute *pAttr = attrList->first();
          pAttr != 0;
          pAttr = attrList->next() )
        {
            if (filter == (PIExport)-1 || filter == pAttr->exportScope) 
                ilastItem = new ClassTreeAttrItem(*lastItem, ilastItem, pAttr);
        }
    delete attrList;
    
    if (!(*lastItem)->firstChild())
        (*lastItem)->setExpandable(false);
    else
        (*lastItem)->setOpen(true);
}


void ClassToolWidget::addClassAndMethods(ParsedClass *parsedClass, PIExport filter, ClassTreeItem **lastItem)
{
    *lastItem = new ClassTreeClassItem(this, *lastItem, parsedClass);
    
    ClassTreeItem *ilastItem = 0;
    
    QList<ParsedMethod> *methodList = parsedClass->getSortedMethodList();
    for ( ParsedMethod *pMethod = methodList->first();
          pMethod != 0;
          pMethod = methodList->next() )
        {
            if (filter == (PIExport)-1 || filter == pMethod->exportScope) 
                ilastItem = new ClassTreeMethodItem(*lastItem, ilastItem, pMethod);
        }
    delete methodList;
    
    if (!(*lastItem)->firstChild())
        (*lastItem)->setExpandable(false);
    else
        (*lastItem)->setOpen(true);
}


void ClassToolWidget::insertAllClassMethods(ParsedClass *parsedClass, PIExport filter)
{
    ClassTreeItem *lastItem = 0;
    
    // First treat all parents.
    for ( ParsedParent *pParent = parsedClass->parents.first();
          pParent != 0;
          pParent = parsedClass->parents.next() )
        {
            ParsedClass *parentClass = m_store->getClassByName(pParent->name);
            if (parentClass)
                addClassAndMethods(parentClass, filter, &lastItem);
        }
    
    // Add the current class
    addClassAndMethods(parsedClass, filter, &lastItem);
}


void ClassToolWidget::insertAllClassAttributes(ParsedClass *parsedClass, PIExport filter)
{
    ClassTreeItem *lastItem;
    
    // First treat all parents.
    for ( ParsedParent *pParent = parsedClass->parents.first();
          pParent != 0;
          pParent = parsedClass->parents.next() )
        {
            ParsedClass *parentClass = m_store->getClassByName(pParent->name);
            if (parentClass)
                addClassAndAttributes(parentClass, filter, &lastItem);
        }
    
    // Add the current class
    addClassAndAttributes(parsedClass, filter, &lastItem);
}
