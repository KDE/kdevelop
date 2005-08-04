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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include "klistviewaction.h"
#include "kcomboview.h"

#include "viewcombos.h"
#include "classviewpart.h"

NamespaceItem::NamespaceItem(ClassViewPart *part, Q3ListView *parent, QString name, NamespaceDom dom)
    :Q3ListViewItem(parent, name), m_dom(dom), m_part(part)
{
}

NamespaceItem::NamespaceItem(ClassViewPart *part, Q3ListViewItem *parent, QString name, NamespaceDom dom)
    :Q3ListViewItem(parent, name), m_dom(dom), m_part(part)
{
}

NamespaceItem::~ NamespaceItem( )
{
}

NamespaceDom NamespaceItem::dom() const
{
    return m_dom;
}

void NamespaceItem::setup()
{
    Q3ListViewItem::setup();
    setPixmap( 0, UserIcon("CVnamespace", KIcon::DefaultState, m_part->instance()) );
}


ClassItem::ClassItem(ClassViewPart *part, Q3ListView *parent, QString name, ClassDom dom)
    :Q3ListViewItem(parent, name), m_dom(dom), m_part(part)
{
}

ClassItem::ClassItem(ClassViewPart *part, Q3ListViewItem *parent, QString name, ClassDom dom)
    :Q3ListViewItem(parent, name), m_dom(dom), m_part(part)
{
}

ClassItem::~ ClassItem( )
{
}

ClassDom ClassItem::dom() const
{
    return m_dom;
}

void ClassItem::setup()
{
    Q3ListViewItem::setup();
    setPixmap( 0, UserIcon("CVclass", KIcon::DefaultState, m_part->instance()) );
}


FunctionItem::FunctionItem(ClassViewPart *part, Q3ListView *parent, QString name, FunctionDom dom)
    :Q3ListViewItem(parent, name), m_dom(dom), m_part(part)
{
}

FunctionItem::FunctionItem(ClassViewPart *part, Q3ListViewItem *parent, QString name, FunctionDom dom)
    :Q3ListViewItem(parent, name), m_dom(dom), m_part(part)
{
}

FunctionItem::~ FunctionItem( )
{
}

FunctionDom FunctionItem::dom() const
{
    return m_dom;
}

void FunctionItem::setup()
{
    Q3ListViewItem::setup();
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
    }
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

