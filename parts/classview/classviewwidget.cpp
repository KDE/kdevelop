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
#include <kdebug.h>

#include "kdevlanguagesupport.h"
#include "kdevproject.h"

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
        popup = new KPopupMenu(i18n("Class View"), this);
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


/**
 * Determines the folder where a class defined in file fileName
 * is stored. If it is in $(top_srcdir) a null string is returned.
 * Examples:
 *   determineFolder("/proj/src/include/foo.cpp", "/proj") => "src/include"
 *   determineFolder("/proj/bar.cpp", "/proj") => null
 */

static QString determineFolder(QString fileName, QString projectDir)
{
    kdDebug() << "determineFolder "  << fileName << ", " << projectDir << endl;
    projectDir += "/";
    // filename must be a path all right, consider assert'ing this
    if (!fileName.startsWith(projectDir))
        return QString::null;
    fileName.remove(0, projectDir.length()); // get relative path
    int pos = fileName.findRev('/');
    if (pos == -1)
        return QString::null;
    else
        return fileName.left(pos);
}


void ClassViewWidget::buildTreeByCategory(bool fromScratch)
{
    TreeState oldTreeState;
    if (!fromScratch)
        oldTreeState = treeState();
    
    clear();

    ClassStore *store = m_part->classStore();
    ParsedScopeContainer *globalScope = store->globalScope();
    
    ClassTreeItem *ilastItem, *lastItem = 0;

    KDevLanguageSupport::Features features = m_part->languageSupport()->features();

    if (features & KDevLanguageSupport::Classes) {
        // Add classes
        lastItem = new ClassTreeOrganizerItem(this, lastItem, i18n("Classes"));
        ilastItem = 0;
        QValueList<ParsedClass*> classList = store->getSortedClassList();
        QValueList<ParsedClass*>::ConstIterator it;

        // Make a list of all directories under the project directory
        QString projectDir = m_part->project()->projectDirectory();
        QStringList dirNames;
        for (it = classList.begin(); it != classList.end(); ++it) {
            QString fileName = (*it)->definedInFile();
            QString dirName = determineFolder(fileName, projectDir);
            if (!dirName.isNull() && !dirNames.contains(dirName))  // TODO: O(n^2), not good style
                dirNames.append(dirName);
        }

        // Create folders
        dirNames.sort();
        QMap<QString, ClassTreeItem*> folders;
        
        QStringList::ConstIterator sit;
        for (sit = dirNames.begin(); sit != dirNames.end(); ++sit) {
            ilastItem = new ClassTreeOrganizerItem(lastItem, ilastItem, *sit);
            folders.insert(*sit, ilastItem);
        }

        // Put classes into folders (if appropriate) or directly into the organizer item
        for (it = classList.begin(); it != classList.end(); ++it) {
            QString fileName = (*it)->definedInFile();
            QString dirName = determineFolder(fileName, projectDir);
            QMap<QString, ClassTreeItem*>::ConstIterator fit = folders.find(dirName);
            if (fit == folders.end())
                ilastItem = new ClassTreeClassItem(lastItem, ilastItem, *it);
            else {
                QListViewItem *iilastItem = (*fit)->firstChild();
                while (iilastItem)
                    iilastItem = iilastItem->nextSibling();
                (void) new ClassTreeClassItem(*fit, static_cast<ClassTreeItem*>(iilastItem), *it);
            }
        }
        if (fromScratch)
            lastItem->setOpen(true);
    }
    
    if (features & KDevLanguageSupport::Structs) {
        // Add structs
        lastItem = new ClassTreeOrganizerItem(this, lastItem, i18n("Structs"));
        ilastItem = 0;
        QValueList<ParsedStruct*> structList = store->getSortedStructList();
        QValueList<ParsedStruct*>::ConstIterator it;
        for (it = structList.begin(); it != structList.end(); ++it)
            ilastItem = new ClassTreeStructItem(lastItem, ilastItem, *it);
        if (fromScratch)
            lastItem->setOpen(true);
    }
                                 
    if (features & KDevLanguageSupport::Functions) {
        // Add functions
        lastItem = new ClassTreeOrganizerItem(this, lastItem, i18n("Global Functions"));
        ilastItem = 0;
        QValueList<ParsedMethod*> methodList = globalScope->getSortedMethodList();
        QValueList<ParsedMethod*>::ConstIterator it;
        for (it = methodList.begin(); it != methodList.end(); ++it)
            ilastItem = new ClassTreeMethodItem(lastItem, ilastItem, *it);
        if (fromScratch)
            lastItem->setOpen(true);
    }

    if (features & KDevLanguageSupport::Variables) {
        // Add attributes
        lastItem = new ClassTreeOrganizerItem(this, lastItem, i18n("Global Variables"));
        ilastItem = 0;
        QValueList<ParsedAttribute*> attrList = globalScope->getSortedAttributeList();
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
        QValueList<ParsedScopeContainer*> scopeList = store->getSortedScopeList();
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

    // Global namespace
    lastItem = new ClassTreeScopeItem(this, lastItem, m_part->classStore()->globalScope());
    if (fromScratch)
        lastItem->setOpen(true);

    // Namespaces just below the global one
    QValueList<ParsedScopeContainer*> scopeList = m_part->classStore()->globalScope()->getSortedScopeList();
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
