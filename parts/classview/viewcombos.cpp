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

#include "kcomboview.h"

#include "viewcombos.h"
#include "classviewpart.h"

NamespaceItem::NamespaceItem(ClassViewPart *part, QListView *parent, QString name, NamespaceDom dom)
    :QListViewItem(parent, name), m_dom(dom), m_part(part)
{
    m_part->nsmap[m_dom.data()] = this;
}

NamespaceItem::NamespaceItem(ClassViewPart *part, QListViewItem *parent, QString name, NamespaceDom dom)
    :QListViewItem(parent, name), m_dom(dom), m_part(part)
{
    m_part->nsmap[m_dom.data()] = this;
}

NamespaceItem::~ NamespaceItem( )
{
    m_part->nsmap.remove(m_dom.data());
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
    m_part->clmap[m_dom.data()] = this;
}

ClassItem::ClassItem(ClassViewPart *part, QListViewItem *parent, QString name, ClassDom dom)
    :QListViewItem(parent, name), m_dom(dom), m_part(part)
{
    m_part->clmap[m_dom.data()] = this;
}

ClassItem::~ ClassItem( )
{
    m_part->clmap.remove(m_dom.data());
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
    m_part->fnmap[m_dom.data()] = this;
}

FunctionItem::FunctionItem(ClassViewPart *part, QListViewItem *parent, QString name, FunctionDom dom)
    :QListViewItem(parent, name), m_dom(dom), m_part(part)
{
    m_part->fnmap[m_dom.data()] = this;
}

FunctionItem::~ FunctionItem( )
{
    m_part->fnmap.remove(m_dom.data());
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

void refreshClasses(ClassViewPart *part, KComboView *view, const NamespaceDom &dom)
{
    view->clear();

    view->setCurrentText(EmptyClasses);
    ClassList classes = dom->classList();
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

void refreshFunctions(ClassViewPart *part, KComboView *view, const NamespaceDom & dom)
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

void processNamespace( ClassViewPart *part, KComboView *view, NamespaceItem * item )
{
    NamespaceList namespaces = item->dom()->namespaceList();
    for (NamespaceList::const_iterator it = namespaces.begin(); it != namespaces.end(); ++it)
    {
        NamespaceItem *newitem = new NamespaceItem(part, item, part->languageSupport()->formatModelItem(*it, true), *it);
        view->addItem(newitem);
        newitem->setOpen(true);
        processNamespace(part, view, newitem);
    }
}

void processClass( ClassViewPart *part, KComboView *view, ClassItem * item )
{
    ClassList classes = item->dom()->classList();
    for (ClassList::const_iterator it = classes.begin(); it != classes.end(); ++it)
    {
        ClassItem *newitem = new ClassItem(part, item, part->languageSupport()->formatModelItem(*it), *it);
        view->addItem(newitem);
        newitem->setOpen(true);
        processClass(part, view, newitem);
    }
}

void processFunction( ClassViewPart* /*part*/, KComboView* /*view*/, FunctionItem* /*item*/ )
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

void ViewCombosOp::removeNamespacesItems(ClassViewPart *part, QListView *view, const FileDom &dom)
{
    NamespaceList nl = dom->namespaceList();

    QListViewItemIterator it( view );
    while ( it.current() )
    {
        NamespaceItem *item = dynamic_cast<NamespaceItem*>(it.current());
        kdDebug() << "ViewCombosOp::removeNamespacesItems - fileName in namespace " << item->dom()->name() << " is " << dom->name();
        if ( (item->dom() != part->codeModel()->globalNamespace()) && nl.contains(item->dom()))
        {
            kdDebug() << "deleting..." << endl;
            delete item;
        }
    }
//     QPtrList<QListViewItem> lst;
//     QListViewItemIterator it( view );
//     while ( it.current() ) {
//         NamespaceItem *item = dynamic_cast<NamespaceItem*>(it.current());
//         kdDebug() << "ViewCombosOp::removeNamespacesItems - fileName in namespace " << item->dom()->name() << " is " << item->dom()->file()->name();
//         if (item && (item->dom()->fileName() == fileName))
//             lst.append( it.current() );
//         ++it;
//     }
//
//     lst.setAutoDelete(true);
}

