/*
 *  Copyright (C) 2003 Roberto Raggi (roberto@kdevelop.org)
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
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#include <qwhatsthis.h>
#include <qlistview.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kpopupmenu.h>

#include <kdevcore.h>
#include <kdevmainwindow.h>
#include <kdevlanguagesupport.h>
#include <qcomboview.h>
#include <kdevpartcontroller.h>

#include "classviewwidget.h"
#include "classviewpart.h"
#include "viewcombos.h"
#include "hierarchydlg.h"

#include "klistviewaction.h"

typedef KGenericFactory<ClassViewPart> ClassViewFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevclassview, ClassViewFactory( "kdevclassview" ) );

ClassViewPart::ClassViewPart(QObject *parent, const char *name, const QStringList& )
    : KDevPlugin("ClassView", "classview", parent, name ? name : "ClassViewPart" )
{
    setInstance(ClassViewFactory::instance());
    setXMLFile("kdevclassview.rc");

    setupActions();

    m_widget = new ClassViewWidget(this);

    mainWindow()->embedSelectView( m_widget, i18n("Classes"), i18n("Class browser") );

    connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(slotProjectClosed()) );
}


ClassViewPart::~ClassViewPart()
{
    mainWindow()->removeView( m_widget );
    delete (ClassViewWidget*) m_widget;
}

void ClassViewPart::slotProjectOpened( )
{
    connect( languageSupport(), SIGNAL(updatedSourceInfo()), this, SLOT(refresh()) );
}

void ClassViewPart::slotProjectCloses( )
{
    disconnect( languageSupport(), SIGNAL(updatedSourceInfo()), this, SLOT(refresh()) );
    m_namespaces->view()->clear();
    m_classes->view()->clear();
    m_functions->view()->clear();
}

void ClassViewPart::setupActions( )
{
    m_namespaces = new KListViewAction( new QComboView(true), i18n("Namespaces"), 0, 0, 0, actionCollection(), "namespaces_combo" );
    connect( m_namespaces->view(), SIGNAL(activated(QListViewItem*)), this, SLOT(selectNamespace(QListViewItem*)) );
    m_namespaces->setToolTip(i18n("Namespaces"));
    m_namespaces->setWhatsThis(i18n("<b>Namespace Selector</b>\nSelect a namespace to view classes and functions contained in it."));

    m_classes = new KListViewAction( new QComboView(true), i18n("Classes"), 0, 0, 0, actionCollection(), "classes_combo" );
    connect( m_classes->view(), SIGNAL(activated(QListViewItem*)), this, SLOT(selectClass(QListViewItem*)) );
    m_classes->setToolTip(i18n("Classes"));
    m_classes->setWhatsThis(i18n("<b>Class Selector</b>\nSelect a class to view it's members."));

    m_functions = new KListViewAction( new QComboView(true), i18n("Functions"), 0, 0, 0, actionCollection(), "functions_combo" );
    connect( m_functions->view(), SIGNAL(activated(QListViewItem*)), this, SLOT(selectFunction(QListViewItem*)) );
    m_functions->setToolTip(i18n("Functions"));
    m_functions->setWhatsThis(i18n("<b>Function Selector</b>\nSelect a function to jump to it's definition or declaration."));

    m_namespaces->view()->setMinimumWidth(150);
    m_classes->view()->setMinimumWidth(150);
    m_functions->view()->setMinimumWidth(300);

    m_popupAction  = new KToolBarPopupAction(i18n("Class Browser Actions"), "classwiz", 0,
                                           this, SLOT(switchedViewPopup()),
                                           actionCollection(), "view_popup");
    m_popupAction->setToolTip(i18n("Class Browser Actions"));
    m_popupAction->setDelayed(false);
    KPopupMenu *popup = m_popupAction->popupMenu();
    //TODO: check if language support has namespaces, classes, etc.
//    KDevLanguageSupport::Features features = languageSupport()->features();
    popup->insertItem(i18n("Go to Function Declaration"), this, SLOT(goToFunctionDeclaration()));
    popup->insertItem(i18n("Go to Function Definition"), this, SLOT(goToFunctionDefinition()));
    popup->insertItem(i18n("Go to Class Declaration"), this, SLOT(goToClassDeclaration()));

    //TODO: not applicable to c++ but can be useful for ada and pascal where namespace is contained
    //in a single compilation unit
    //FIXME: commented out till the time when it be needed
//    popup->insertItem(i18n("Go to Namespace Declaration"), this, SLOT(goToNamespaceDeclaration()));

    bool hasAddMethod = langHasFeature(KDevLanguageSupport::AddMethod);
    bool hasAddAttribute = langHasFeature(KDevLanguageSupport::AddAttribute);
    bool hasNewClass = langHasFeature(KDevLanguageSupport::NewClass);
    if (hasAddMethod || hasAddAttribute || hasNewClass)
        popup->insertSeparator();
    if (hasNewClass)
        popup->insertItem(SmallIcon("classnew"), i18n("Add Class..."), this, SLOT(selectedAddClass()));
    if (hasAddMethod)
        popup->insertItem(SmallIcon("methodnew"), i18n("Add Method..."), this, SLOT(selectedAddMethod()));
    if (hasAddAttribute)
        popup->insertItem(SmallIcon("variablenew"), i18n("Add Attribute..."), this, SLOT(selectedAddAttribute()));

    popup->insertSeparator();
    popup->insertItem(i18n("Graphical Class Hierarchy"), this, SLOT(graphicalClassView()));
}

void ClassViewPart::refresh( )
{
    ViewCombosOp::refreshNamespaces(this, m_namespaces->view());
}

void ClassViewPart::selectNamespace( QListViewItem * item )
{
    NamespaceItem *ni = dynamic_cast<NamespaceItem*>(item);
    if (!ni)
        return;
    ViewCombosOp::refreshClasses(this, m_classes->view(), ni->dom());
    ViewCombosOp::refreshFunctions(this, m_functions->view(), ni->dom());
}

void ClassViewPart::selectClass( QListViewItem * item )
{
    ClassItem *ci = dynamic_cast<ClassItem*>(item);
    if (!ci)
        return;
    ViewCombosOp::refreshFunctions(this, m_functions->view(), ci->dom());
}

void ClassViewPart::selectFunction( QListViewItem * item )
{
    FunctionItem *fi = dynamic_cast<FunctionItem*>(item);
    if (!fi)
        return;
    int startLine, startColumn;
    fi->dom()->getImplementationStartPosition( &startLine, &startColumn );
    if (startLine != 0)
        partController()->editDocument( KURL(fi->dom()->implementedInFile()), startLine );
    else
    {
        fi->dom()->getStartPosition( &startLine, &startColumn );
        partController()->editDocument( KURL(fi->dom()->fileName()), startLine );
    }
}

void ClassViewPart::switchedViewPopup( )
{
}

bool ClassViewPart::langHasFeature(KDevLanguageSupport::Features feature)
{
    bool result = false;
    if (languageSupport())
        result = (feature & languageSupport()->features());
    return result;
}

void ClassViewPart::goToFunctionDeclaration( )
{
    if ( m_functions->view()->currentItem() )
    {
        FunctionItem *fi = dynamic_cast<FunctionItem*>(m_functions->view()->currentItem());
        if (!fi)
            return;
        int startLine, startColumn;
        fi->dom()->getStartPosition( &startLine, &startColumn );
        partController()->editDocument( KURL(fi->dom()->fileName()), startLine );
    }
}

void ClassViewPart::goToFunctionDefinition( )
{
    if ( m_functions->view()->currentItem() )
    {
        FunctionItem *fi = dynamic_cast<FunctionItem*>(m_functions->view()->currentItem());
        if (!fi)
            return;
        int startLine, startColumn;
        fi->dom()->getImplementationStartPosition( &startLine, &startColumn );
        partController()->editDocument( KURL(fi->dom()->implementedInFile()), startLine );
    }
}

void ClassViewPart::goToClassDeclaration( )
{
    if ( m_classes->view()->currentItem() )
    {
        ClassItem *fi = dynamic_cast<ClassItem*>(m_classes->view()->currentItem());
        if (!fi)
            return;
        int startLine, startColumn;
        fi->dom()->getStartPosition( &startLine, &startColumn );
        partController()->editDocument( KURL(fi->dom()->fileName()), startLine );
    }
}

void ClassViewPart::goToNamespaceDeclaration( )
{
    if ( m_namespaces->view()->currentItem() )
    {
        NamespaceItem *fi = dynamic_cast<NamespaceItem*>(m_namespaces->view()->currentItem());
        if (!fi)
            return;
        int startLine, startColumn;
        fi->dom()->getStartPosition( &startLine, &startColumn );
        partController()->editDocument( KURL(fi->dom()->fileName()), startLine );
    }
}

void ClassViewPart::selectedAddClass( )
{
    if (languageSupport())
        languageSupport()->addClass();
}

void ClassViewPart::selectedAddMethod( )
{
    ClassItem *ci = dynamic_cast<ClassItem*>(m_classes->view()->currentItem());
    if (!ci)
        return;
    if (languageSupport())
        languageSupport()->addMethod(ci->dom()->name());
}

void ClassViewPart::selectedAddAttribute( )
{
    ClassItem *ci = dynamic_cast<ClassItem*>(m_classes->view()->currentItem());
    if (!ci)
        return;
    if (languageSupport())
        languageSupport()->addAttribute(ci->dom()->name());
}

void ClassViewPart::graphicalClassView( )
{
    HierarchyDialog dia(this);
    dia.exec();
}

#include "classviewpart.moc"
