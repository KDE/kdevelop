/*
 *  Copyright (C) 2003 Alexander Dymo (cloudtemple@mksat.net)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include "klistviewaction.h"
#include "kcomboview.h"

#include "viewcombos.h"
#include "classviewpart.h"

NamespaceItem::NamespaceItem(ClassViewPart *part, QListView *parent, QString name, NamespaceDom dom)
    :QListViewItem(parent, name), m_dom(dom), m_part(part)
{
    m_part->nsmap[m_dom->name()] = this;
}

NamespaceItem::NamespaceItem(ClassViewPart *part, QListViewItem *parent, QString name, NamespaceDom dom)
    :QListViewItem(parent, name), m_dom(dom), m_part(part)
{
    m_part->nsmap[m_dom->name()] = this;
}

NamespaceItem::~ NamespaceItem( )
{
    m_part->nsmap.remove(m_dom->name());
}

NamespaceDom NamespaceItem::dom() const
{
    return m_dom;
}

void NamespaceItem::setup()
{
    QListViewItem::setup();
    setPixmap( 0, UserIcon("CVnamespace", KIcon::DefaultState, m_part->instance()) );
}


ClassItem::ClassItem(ClassViewPart *part, QListView *parent, QString name, ClassDom dom)
    :QListViewItem(parent, name), m_dom(dom), m_part(part)
{
    m_part->clmap[m_dom] = this;
}

ClassItem::ClassItem(ClassViewPart *part, QListViewItem *parent, QString name, ClassDom dom)
    :QListViewItem(parent, name), m_dom(dom), m_part(part)
{
    m_part->clmap[m_dom] = this;
}

ClassItem::~ ClassItem( )
{
    m_part->clmap.remove(m_dom);
}

ClassDom ClassItem::dom() const
{
    return m_dom;
}

void ClassItem::setup()
{
    QListViewItem::setup();
    setPixmap( 0, UserIcon("CVclass", KIcon::DefaultState, m_part->instance()) );
}


FunctionItem::FunctionItem(ClassViewPart *part, QListView *parent, QString name, FunctionDom dom)
    :QListViewItem(parent, name), m_dom(dom), m_part(part)
{
    m_part->fnmap[m_dom] = this;
}

FunctionItem::FunctionItem(ClassViewPart *part, QListViewItem *parent, QString name, FunctionDom dom)
    :QListViewItem(parent, name), m_dom(dom), m_part(part)
{
    m_part->fnmap[m_dom] = this;
}

FunctionItem::~ FunctionItem( )
{
    m_part->fnmap.remove(m_dom);
}

FunctionDom FunctionItem::dom() const
{
    return m_dom;
}

void FunctionItem::setup()
{
    QListViewItem::setup();
    QString iconName;
    if( m_dom->access() == CodeModelItem::Private )
        iconName = "CVprivate_meth";
    else if( m_dom->access() == CodeModelItem::Protected )
        iconName = "CVprotected_meth";
    else
        iconName = "CVpublic_meth";
    setPixmap( 0, UserIcon(iconName, KIcon::DefaultState, m_part->instance()) );
}


namespace ViewCombosOp{

void refreshNamespaces(ClassViewPart *part, KComboView *view)
{
    view->clear();

    NamespaceItem *global_item = new NamespaceItem( part, view->listView(), i18n("(Global Namespace)"), part->codeModel()->globalNamespace() );
    view->addItem(global_item);
    global_item->setPixmap( 0, UserIcon("CVnamespace", KIcon::DefaultState, part->instance()) );
    NamespaceList namespaces = part->codeModel()->globalNamespace()->namespaceList();
    for (NamespaceList::const_iterator it = namespaces.begin(); it != namespaces.end(); ++it)
    {
        NamespaceItem *item = new NamespaceItem(part, view->listView(), part->languageSupport()->formatModelItem(*it), *it);
        view->addItem(item);
        item->setOpen(true);
        processNamespace(part, view, item);
    }
    view->setCurrentActiveItem(global_item);
}

void refreshClasses(ClassViewPart *part, KComboView *view, const QString &dom)
{
    view->clear();

    view->setCurrentText(EmptyClasses);
    NamespaceDom nsdom;
    if (dom == "::")
        nsdom = part->codeModel()->globalNamespace();
    else
    {
        nsdom = namespaceByName(part->codeModel()->globalNamespace(), dom);
        if (!nsdom)
            return;
    }
    ClassList classes = nsdom->classList();
    for (ClassList::const_iterator it = classes.begin(); it != classes.end(); ++it)
    {
        ClassItem *item = new ClassItem(part, view->listView(), part->languageSupport()->formatModelItem(*it), *it);
        view->addItem(item);
        item->setOpen(true);
        processClass(part, view, item);
    }
}

void refreshFunctions(ClassViewPart *part, KComboView *view, const ClassDom & dom)
{
    view->clear();

    view->setCurrentText(EmptyFunctions);
    FunctionList functions = dom->functionList();
    for (FunctionList::const_iterator it = functions.begin(); it != functions.end(); ++it)
    {
        FunctionItem *item = new FunctionItem(part, view->listView(), part->languageSupport()->formatModelItem(*it, true), *it);
        view->addItem(item);
        item->setOpen(true);
        processFunction(part, view, item);
    }
}

void refreshFunctions(ClassViewPart *part, KComboView *view, const QString & dom)
{
    view->clear();

    view->setCurrentText(EmptyFunctions);
    NamespaceDom nsdom;
    if (dom == "::")
        nsdom = part->codeModel()->globalNamespace();
    else
    {
        nsdom = namespaceByName(part->codeModel()->globalNamespace(), dom);
        if (!nsdom)
            return;
    }
    FunctionList functions = nsdom->functionList();
    for (FunctionList::const_iterator it = functions.begin(); it != functions.end(); ++it)
    {
        FunctionItem *item = new FunctionItem(part, view->listView(), part->languageSupport()->formatModelItem(*it, true), *it);
        view->addItem(item);
        item->setOpen(true);
        processFunction(part, view, item);
    }
}

void processNamespace( ClassViewPart *part, KComboView *view, NamespaceItem * item, ProcessType type )
{
    NamespaceDom baseDom = namespaceByName(part->codeModel()->globalNamespace(), item->dom()->name());
    if (!baseDom)
        return;
    NamespaceList namespaces = baseDom->namespaceList();
    for (NamespaceList::const_iterator it = namespaces.begin(); it != namespaces.end(); ++it)
    {
       kdDebug() << "   processNamespace( for " << item->dom()->name() << endl;
       if ((type == ViewCombosOp::Refresh) && (part->nsmap.contains((*it)->name())))
        {
            kdDebug() << "  ok. refresh " << (*it)->name() << endl;
            NamespaceDom nsdom = *it;
            //namespace item exists - update
            NamespaceItem *ns = part->nsmap[nsdom->name()];
            ns->setText(0, part->languageSupport()->formatModelItem(nsdom));
            if (part->m_namespaces->view()->currentItem() == ns)
            {
                //reload this and dependent combos because namespace item is currently selected
                part->m_namespaces->view()->setCurrentText(part->languageSupport()->formatModelItem(nsdom));

                //check classes
                part->updateClassesForAdd(nsdom);
                //check functions
                part->updateFunctionsForAdd(model_cast<ClassDom>(nsdom));
            }
            //refresh info about nested namespaces
            processNamespace(part, part->m_namespaces->view(), ns, ViewCombosOp::Refresh);

            continue;
        }

        kdDebug() << "  ok. add " << (*it)->name() << endl;
        NamespaceItem *newitem = new NamespaceItem(part, item, part->languageSupport()->formatModelItem(*it, true), *it);
        view->addItem(newitem);
        newitem->setOpen(true);
        processNamespace(part, view, newitem);
    }
}

void processClass( ClassViewPart *part, KComboView *view, ClassItem * item, ProcessType type )
{
    ClassList classes = item->dom()->classList();
    for (ClassList::const_iterator it = classes.begin(); it != classes.end(); ++it)
    {
        if ((type == ViewCombosOp::Refresh) && (part->clmap.contains(*it)))
        {
            ClassDom cldom = *it;
            //class item exists - update
            ClassItem *cl = part->clmap[cldom];
            cl->setText(0, part->languageSupport()->formatModelItem(cldom));
            if (part->m_classes->view()->currentItem() == cl)
            {
                //reload this and dependent combos because class item is currently selected
                part->m_classes->view()->setCurrentText(part->languageSupport()->formatModelItem(cldom));

                //check functions
                part->updateFunctionsForAdd(cldom);
            }
            //refresh info about nested classes
            processClass(part, part->m_classes->view(), cl, ViewCombosOp::Refresh);

            continue;
        }

        ClassItem *newitem = new ClassItem(part, item, part->languageSupport()->formatModelItem(*it), *it);
        view->addItem(newitem);
        newitem->setOpen(true);
        processClass(part, view, newitem);
    }
}

void processFunction( ClassViewPart* /*part*/, KComboView* /*view*/, FunctionItem* /*item*/, ProcessType /*type*/ )
{
    //@todo allow nested functions (adymo: Pascal has nested procedures and functions)
/*    FunctionList functions = item->dom()->functionList();
    for (FunctionList::const_iterator it = functions.begin(); it != functions.end(); ++it)
    {
        FunctionItem *newitem = new FunctionItem(part, item, part->languageSupport()->formatModelItem(*it), *it);
        view->addItem(newitem);
        newitem->setOpen(true);
        processFunction(part, view, newitem);
    }*/
}

}

bool ViewCombosOp::removeNamespacesItems(ClassViewPart *part, QListView *view, const NamespaceDom &dom)
{
    removeFunctionItems(part, view, model_cast<ClassDom>(dom));
    removeClassItems(part, view, model_cast<ClassDom>(dom));

    bool result = false;

    NamespaceList nl = dom->namespaceList();
    for (NamespaceList::const_iterator it = nl.begin(); it != nl.end(); ++it)
    {
        result = result || removeNamespacesItems(part, view, *it);
        NamespaceDom nsd = *it;
        if ( (part->nsmap.contains((*it)->name())) && (part->nsmap[(*it)->name()] != 0) )
            result = true;
    }
    return result;
}

void ViewCombosOp::removeClassItems( ClassViewPart * part, QListView * view, const ClassDom & dom )
{
    removeFunctionItems(part, view, dom);

    ClassList cl = dom->classList();
    for (ClassList::const_iterator it = cl.begin(); it != cl.end(); ++it)
    {
        removeClassItems(part, view, *it);
        if ( (part->clmap.contains(*it)) && (part->clmap[*it] != 0) )
            part->m_classes->view()->removeItem(part->clmap[*it]);
    }
}

void ViewCombosOp::removeFunctionItems( ClassViewPart * part, QListView * view, const ClassDom & dom )
{
    FunctionList fl = dom->functionList();
    for (FunctionList::const_iterator it = fl.begin(); it != fl.end(); ++it)
    {
        if ( (part->fnmap.contains(*it)) && (part->fnmap[*it] != 0) )
            part->m_functions->view()->removeItem(part->fnmap[*it]);
    }
}

NamespaceDom ViewCombosOp::namespaceByName( NamespaceDom dom, QString name )
{
    NamespaceDom result;

    result = dom->namespaceByName(name);
    if (!result)
    {
        NamespaceList nslist = dom->namespaceList();
        for (NamespaceList::const_iterator it = nslist.begin(); it != nslist.end(); ++it)
        {
            result = namespaceByName(*it, name);
            if (result)
                break;
        }
    }
    return result;
}

