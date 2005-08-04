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

#include "classtoolwidget.h"

#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kpopupmenu.h>
#include "classstore.h"
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3PtrList>


ClassToolWidget::ClassToolWidget(ClassViewPart *part, QWidget *parent)
    : ClassTreeBase(part, parent, "class tool widget")
{}


ClassToolWidget::~ClassToolWidget()
{}


KPopupMenu *ClassToolWidget::createPopup()
{
    KPopupMenu *popup = contextItem? contextItem->createPopup() : 0;
    if (!popup) {
        popup = new KPopupMenu(this);
	popup->insertTitle(i18n("Class Tool"));
    }

    return popup;
}


void ClassToolWidget::insertClassAndClasses(ParsedClass *parsedClass, Q3ValueList<ParsedClass*> classList)
{
    ClassTreeItem *root = new ClassTreeClassItem(this, 0, parsedClass);
    
    ClassTreeItem *lastItem = 0;

    Q3ValueList<ParsedClass*>::ConstIterator it;
    for (it = classList.begin(); it != classList.end(); ++it) {
        lastItem = new ClassTreeClassItem(root, lastItem, *it);
        lastItem->setExpandable(false);
    }
    
    if (!root->firstChild())
        root->setExpandable(false);
    else
        root->setOpen(true);
}


void ClassToolWidget::insertClassAndClasses(ParsedClass *parsedClass, const Q3PtrList<ParsedParent> &parentList)
{
    ClassTreeItem *root = new ClassTreeClassItem(this, 0, parsedClass);
    
    ClassTreeItem *lastItem = 0;

    Q3PtrListIterator<ParsedParent> it(parentList);
    for (; it.current(); ++it) {
        ParsedClass *parentClass = m_part->classStore()->getClassByName((*it)->name());
        lastItem = new ClassTreeClassItem(root, lastItem, parentClass);
        lastItem->setExpandable(false);
    }
    
    if (!root->firstChild())
        root->setExpandable(false);
    else
        root->setOpen(true);
}


void ClassToolWidget::addClassAndAttributes(ParsedClass *parsedClass, PIAccess filter, ClassTreeItem **lastItem)
{
    *lastItem = new ClassTreeClassItem(this, *lastItem, parsedClass);

    ClassTreeItem *ilastItem = 0;
    
    Q3ValueList<ParsedAttribute*> attrList = parsedClass->getSortedAttributeList();
    Q3ValueList<ParsedAttribute*>::ConstIterator it;
    for (it = attrList.begin(); it != attrList.end(); ++it) {
        if (filter == (PIAccess)-1 || filter == (*it)->access())
            ilastItem = new ClassTreeAttrItem(*lastItem, ilastItem, *it);
    }

    if (!(*lastItem)->firstChild())
        (*lastItem)->setExpandable(false);
    else
        (*lastItem)->setOpen(true);
}


void ClassToolWidget::addClassAndMethods(ParsedClass *parsedClass, PIAccess filter, ClassTreeItem **lastItem)
{
    *lastItem = new ClassTreeClassItem(this, *lastItem, parsedClass);
    
    ClassTreeItem *ilastItem = 0;
    
    Q3ValueList<ParsedMethod*> methodList = parsedClass->getSortedMethodList();
    Q3ValueList<ParsedMethod*>::ConstIterator it;
    for (it = methodList.begin(); it != methodList.end(); ++it) {
        if (filter == (PIAccess)-1 || filter == (*it)->access())
            ilastItem = new ClassTreeMethodItem(*lastItem, ilastItem, *it);
    }
    
    if (!(*lastItem)->firstChild())
        (*lastItem)->setExpandable(false);
    else
        (*lastItem)->setOpen(true);
}


void ClassToolWidget::insertAllClassMethods(ParsedClass *parsedClass, PIAccess filter)
{
    ClassTreeItem *lastItem = 0;
    
    // First treat all parents.
    for ( ParsedParent *pParent = parsedClass->parents.first();
          pParent != 0;
          pParent = parsedClass->parents.next() )
        {
            ParsedClass *parentClass = m_part->classStore()->getClassByName(pParent->name());
            if (parentClass)
                addClassAndMethods(parentClass, filter, &lastItem);
        }
    
    // Add the current class
    addClassAndMethods(parsedClass, filter, &lastItem);
}


void ClassToolWidget::insertAllClassAttributes(ParsedClass *parsedClass, PIAccess filter)
{
    ClassTreeItem *lastItem = 0;
    // First treat all parents.
    for ( ParsedParent *pParent = parsedClass->parents.first();
          pParent != 0;
          pParent = parsedClass->parents.next() )
        {
            ParsedClass *parentClass = m_part->classStore()->getClassByName(pParent->name());
            if (parentClass)
                addClassAndAttributes(parentClass, filter, &lastItem);
        }
    
    // Add the current class
    addClassAndAttributes(parsedClass, filter, &lastItem);
}

#include "classtoolwidget.moc"
