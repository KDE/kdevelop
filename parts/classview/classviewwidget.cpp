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
#include "domutil.h"

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
    if (ls) {
        disconnect(ls, 0, this, 0);
        connect(ls, SIGNAL(updatedSourceInfo()), this, SLOT(refresh()));
    } else
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
 * is stored. This works by removing n levels below the project
 * directory, if it is in a subdirectory that deep. Otherwise a
 * null string is returned.
 * Examples:
 *   determineFolder("/proj/src/include/foo.cpp", "/proj", 2) => "src/include"
 *   determineFolder("/proj/src/bla.cpp, "/proj", 2) => null
 *   determineFolder("/proj/bar.cpp", "/proj", 2) => null
 */
QString ClassViewWidget::determineFolder(QString fileName, QString projectDir, int levels)
{
    projectDir += "/";
    if (!fileName.startsWith(projectDir))
        return QString::null;
    fileName.remove(0, projectDir.length()); // get relative path
    int pos;
    if (levels != -1) { // we're given a specific level
        pos = fileName.find('/');
        if (pos == -1)
            return QString::null;
        while (--levels > 0) {
            pos = fileName.find('/', pos+1);
            if (pos == -1)
                return QString::null;
        }
   }
   else { // get the complete relative dir
    pos = fileName.findRev('/');
    if (pos == -1)
        return QString::null;
   }
   return fileName.left(pos);
}


/**
 * Creates a hierarchy of folder items with the given dirNames.
 * The hierarchy is built up below the parent argument.
 * The output argument folders is filled with a mapping from
 * folder names to their list view items.
 */
void ClassViewWidget::buildClassFolderHierarchy(ClassTreeItem *classes,
                                                QStringList &dirNames,
                                                QMap<QString, ClassTreeItem*> *folders)
{    
    QStringList::ConstIterator sit;
    for (sit = dirNames.begin(); sit != dirNames.end(); ++sit) {
        QStringList l = QStringList::split('/', *sit);
        QStringList::ConstIterator i;
        QString path;
        for (i = l.begin(); i != l.end(); ++i) {
            const QString & dir = *i;
            ClassTreeItem *parent;
            QMap<QString, ClassTreeItem*>::iterator folder = folders->find(path);
            if (folder!=folders->end())
                parent = *folder;
            else
                parent = classes;
            if (!path.isEmpty())
                path += '/';
            path += dir;
            if (folders->find(path)==folders->end())
                folders->insert(path, new ClassTreeOrganizerItem(parent, 0, dir));
        }
    }

    // Sort folders
    setSorting(1, true); // turn sorting on
    sortFolder(classes);
    setSorting(-1);      // we don't keep the whole view sorted
}


/**
 * Creates a flat list of folder items.
 */
void ClassViewWidget::buildClassFolderFlatList(ClassTreeItem *organizerItem,
                                               QStringList &dirNames,
                                               QMap<QString, ClassTreeItem*> *folders)
{
    dirNames.sort();
    ClassTreeItem *lastItem = 0;
    
    QStringList::ConstIterator it;
    for (it = dirNames.begin(); it != dirNames.end(); ++it) {
        lastItem = new ClassTreeOrganizerItem(organizerItem, lastItem, *it);
        folders->insert(*it, lastItem);
    }
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

        QDomDocument &dom = *m_part->projectDom();
        bool foldersAsHierarchy = DomUtil::readBoolEntry(dom, "/kdevclassview/folderhierarchy");
        int depth = 0;
        if (!foldersAsHierarchy) {
            depth = DomUtil::readIntEntry(dom, "/kdevclassview/depthoffolders");
            if (depth == 0)
                depth = 2;
        }
        
        QValueList<ParsedClass*> classList = store->getSortedClassList();
        QValueList<ParsedClass*>::ConstIterator it;

                // Make a list of all directories under the project directory
        QString projectDir = m_part->project()->projectDirectory();
        QStringList dirNames;
        if (foldersAsHierarchy)
            depth = -1;  // don't consider depth if hierarchical view
        for (it = classList.begin(); it != classList.end(); ++it) {
            QString fileName = (*it)->definedInFile();
            QString dirName = determineFolder(fileName, projectDir, depth);
            if (!dirName.isNull() && !dirNames.contains(dirName))
                dirNames.append(dirName);                        
        }
        
        // Create folders
        QMap<QString, ClassTreeItem*> folders;
        if (foldersAsHierarchy)
            buildClassFolderHierarchy(lastItem, dirNames, &folders);
        else
            buildClassFolderFlatList(lastItem, dirNames, &folders);
        
        ilastItem = 0;
        // Put classes into folders (if appropriate) or directly into the organizer item
        for (it = classList.begin(); it != classList.end(); ++it) {
            QString fileName = (*it)->definedInFile();
            QString dirName = determineFolder(fileName, projectDir, depth);
            kdDebug(9003) << "inserting " << fileName << " into " << dirName << endl;
            QMap<QString, ClassTreeItem*>::ConstIterator fit = folders.find(dirName);
            if (fit == folders.end())
                ilastItem = new ClassTreeClassItem(lastItem, ilastItem, *it);
            else {
                kdDebug(9003) << "found folder" << endl;
                QListViewItem *iilastItem = (*fit)->firstChild();
                while (iilastItem && iilastItem->nextSibling())
                    iilastItem = iilastItem->nextSibling();
                new ClassTreeClassItem(*fit, static_cast<ClassTreeItem*>(iilastItem), *it);
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
    
// Sort folders recursively in a tree view
// This forces a sorting of all items
void ClassViewWidget::sortFolder(QListViewItem* item)
{
    if (item!=0) {
        item->sort();
        QListViewItem *child = item->firstChild();
        for (; child!=0; child = child->nextSibling())
            sortFolder(child);
    }
}

    
#include "classviewwidget.moc"
