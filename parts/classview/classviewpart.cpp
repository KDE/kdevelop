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
#include <qfileinfo.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <kmimetype.h>

#include <kdevcore.h>
#include <kdevmainwindow.h>
#include <kdevlanguagesupport.h>
#include <kcomboview.h>
#include <kdevpartcontroller.h>
#include <urlutil.h>

#include <codemodel.h>
#include <codemodel_utils.h>

#include "classviewwidget.h"
#include "classviewpart.h"
#include "hierarchydlg.h"

#include "klistviewaction.h"

#include <ktexteditor/document.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/view.h>
#include <ktexteditor/selectioninterface.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/clipboardinterface.h>

typedef KGenericFactory<ClassViewPart> ClassViewFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevclassview, ClassViewFactory( "kdevclassview" ) );

ClassViewPart::ClassViewPart(QObject *parent, const char *name, const QStringList& )
    : KDevPlugin("ClassView", "classview", parent, name ? name : "ClassViewPart" ), sync(false)
{
    setInstance(ClassViewFactory::instance());
    setXMLFile("kdevclassview.rc");

    setupActions();

    m_widget = new ClassViewWidget(this);
    m_widget->setIcon( SmallIcon("view_tree") );
    mainWindow()->embedSelectView( m_widget, i18n("Classes"), i18n("Class browser") );

    connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(slotProjectClosed()) );
    connect( core(), SIGNAL(languageChanged()), this, SLOT(slotProjectOpened()) );
    connect( partController(), SIGNAL(activePartChanged(KParts::Part*)),
        this, SLOT(activePartChanged(KParts::Part*)));
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

void ClassViewPart::slotProjectClosed( )
{
    disconnect( languageSupport(), SIGNAL(updatedSourceInfo()), this, SLOT(refresh()) );
    m_namespaces->view()->clear();
    m_classes->view()->clear();
    m_functions->view()->clear();
}

void ClassViewPart::setupActions( )
{
    m_followCode = new KAction(i18n("Synchronize"), "dirsynch", 0, this, SLOT(syncCombos()), actionCollection(), "sync_combos");
    m_followCode->setToolTip(i18n("Synchronize selectors"));
    m_followCode->setWhatsThis(i18n("<b>Synchronize</b>\nSynchronize namespaces, classes and functions selectors with code."));

    m_namespaces = new KListViewAction( new KComboView(true, 150, 0, "m_namespaces_combo"), i18n("Namespaces"), 0, 0, 0, actionCollection(), "namespaces_combo", true );
    connect( m_namespaces->view(), SIGNAL(activated(QListViewItem*)), this, SLOT(selectNamespace(QListViewItem*)) );
    m_namespaces->setToolTip(i18n("Namespaces"));
    m_namespaces->setWhatsThis(i18n("<b>Namespace Selector</b>\nSelect a namespace to view classes and functions contained in it."));

    m_classes = new KListViewAction( new KComboView(true, 150, 0, "m_classes_combo"), i18n("Classes"), 0, 0, 0, actionCollection(), "classes_combo", true );
    connect( m_classes->view(), SIGNAL(activated(QListViewItem*)), this, SLOT(selectClass(QListViewItem*)) );
    connect( m_classes->view(), SIGNAL(focusGranted()), this, SLOT(focusClasses()) );
    connect( m_classes->view(), SIGNAL(focusLost()), this, SLOT(unfocusClasses()) );
    m_classes->setToolTip(i18n("Classes"));
    m_classes->setWhatsThis(i18n("<b>Class Selector</b>\nSelect a class to view it's members."));

    m_functions = new KListViewAction( new KComboView(true, 300, 0, "m_functions_combo"), i18n("Functions"), 0, 0, 0, actionCollection(), "functions_combo", true );
    connect( m_functions->view(), SIGNAL(activated(QListViewItem*)), this, SLOT(selectFunction(QListViewItem*)) );
    connect( m_functions->view(), SIGNAL(focusGranted()), this, SLOT(focusFunctions()) );
    connect( m_functions->view(), SIGNAL(focusLost()), this, SLOT(unfocusFunctions()) );
    m_functions->setToolTip(i18n("Functions"));
    m_functions->setWhatsThis(i18n("<b>Function Selector</b>\nSelect a function to jump to it's definition or declaration."));

//    m_namespaces->view()->setMinimumWidth(150);
//    m_classes->view()->setMinimumWidth(150);
//    m_functions->view()->setMinimumWidth(300);

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
#if 0 /// FIXME: seems that the 'access attribute' of the 'CodeModel' is wrong!!!!!
    if (hasAddMethod)
        popup->insertItem(SmallIcon("methodnew"), i18n("Add Method..."), this, SLOT(selectedAddMethod()));
    if (hasAddAttribute)
        popup->insertItem(SmallIcon("variablenew"), i18n("Add Attribute..."), this, SLOT(selectedAddAttribute()));
#endif

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
    if (sync)
    {
        sync = false;
        return;
    }
    FunctionItem *fi = dynamic_cast<FunctionItem*>(item);
    if (!fi)
        return;
    int startLine, startColumn;
    // FIXME: ROBE
    //fi->dom()->getImplementationStartPosition( &startLine, &startColumn );
    //if (startLine != 0)
    //    partController()->editDocument( KURL(fi->dom()->implementedInFile()), startLine );
    //else
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

        FunctionDefinitionList lst;
        FileList fileList = codeModel()->fileList();
        CodeModelUtils::findFunctionDefinitions( FindOp(fi->dom()), fileList, lst );

        if( lst.isEmpty() )
            return;

        FunctionDefinitionDom fun = lst.front();
        QString path = QFileInfo( fi->dom()->fileName() ).dirPath( true );

        for( FunctionDefinitionList::Iterator it=lst.begin(); it!=lst.end(); ++it ){
        if( path == QFileInfo((*it)->fileName()).dirPath(true) )
            fun = *it;
        }

        fun->getStartPosition( &startLine, &startColumn );
        partController()->editDocument( KURL(fun->fileName()), startLine );

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
        languageSupport()->addMethod(ci->dom());
}

void ClassViewPart::selectedAddAttribute( )
{
    ClassItem *ci = dynamic_cast<ClassItem*>(m_classes->view()->currentItem());
    if (!ci)
        return;
    if (languageSupport())
        languageSupport()->addAttribute(ci->dom());
}

void ClassViewPart::graphicalClassView( )
{
    HierarchyDialog dia(this);
    dia.exec();
}

void ClassViewPart::focusClasses( )
{
    if (m_classes->view()->currentText() == EmptyClasses)
        m_classes->view()->setCurrentText("");
}

void ClassViewPart::focusFunctions( )
{
    if (m_functions->view()->currentText() == EmptyFunctions)
        m_functions->view()->setCurrentText("");
}

void ClassViewPart::unfocusClasses( )
{
    if (m_classes->view()->currentText().isEmpty())
        m_classes->view()->setCurrentText(EmptyClasses);
}

void ClassViewPart::unfocusFunctions( )
{
    if (m_functions->view()->currentText().isEmpty())
        m_functions->view()->setCurrentText(EmptyFunctions);
}

void ClassViewPart::syncCombos( )
{
    kdDebug() << "ClassViewPart::syncCombos" << endl;
    if (m_activeFileName.isEmpty())
        return;
    FileDom dom = codeModel()->fileByName(m_activeFileName);
    if (!dom.data())
        return;
//     NamespaceDom nsdom = syncNamespaces(dom);
//     ClassDom cldom = syncClasses(nsdom);
//     FunctionDom fndom = syncFunctions(cldom);

    kdDebug() << "ClassViewPart::syncCombos working on " << m_activeFileName << endl;
    unsigned int line; unsigned int column;
    m_activeViewCursor->cursorPosition(&line, &column);

    //try to sync with declarations
    bool declarationFound = false;
    CodeModelUtils::AllFunctions functions = CodeModelUtils::allFunctionsDetailed(dom);
    FunctionDom fndom;
    for (FunctionList::Iterator it = functions.functionList.begin();
        it != functions.functionList.end(); ++it)
    {
        int startLine; int startColumn;
        (*it)->getStartPosition(&startLine, &startColumn);
        int endLine; int endColumn;
        (*it)->getEndPosition(&endLine, &endColumn);

        kdDebug() << "sync with " << (*it)->name() << " startLine " << startLine <<
            " endLine " << endLine << " line " << line << endl;

        if ( (line >= startLine) && (line <= endLine) )
        {
            fndom = *it;
            break;
        }
    }

    if (!fndom.data())
        declarationFound = false;
    else
    {
        NamespaceDom nsdom = functions.relations[fndom].ns;
        if (nsdom.data())
            kdDebug() << "namespace to try " << nsdom->name() << endl;
        else
            kdDebug() << "namespace data empty" << endl;

        for (QMap<NamespaceModel*, NamespaceItem*>::const_iterator it = nsmap.begin();
            it != nsmap.end(); ++it)
        {
            kdDebug() << " in nsmap " << it.key() << " data " << it.data() << endl;
            kdDebug() << " in nsmap " << it.key()->name() << " data " << it.data()->text(0) << endl;
        }

        kdDebug() << " in nsmap is ? " << nsdom.data() << endl;
        if (nsdom)
        {
            kdDebug() << " in nsmap is ? " << nsdom->name() << endl;
            if (nsdom->name() != "::")
                return;
        }

        if (nsdom.data() && nsmap[nsdom.data()])
        {
            kdDebug() << "trying namespace " << nsdom->name() << endl;
            m_namespaces->view()->setCurrentActiveItem(nsmap[nsdom.data()]);
        }
        else
        {
            kdDebug() << "trying global namespace " << endl;
            if (m_namespaces->view()->listView()->firstChild())
            {
                kdDebug() << "firstChild exists - global ns" << endl;
                m_namespaces->view()->setCurrentActiveItem(m_namespaces->view()->listView()->firstChild());
            }
        }
        ClassDom cldom = functions.relations[fndom].klass;
        if (cldom.data() && clmap[cldom.data()])
        {
            kdDebug() << "trying class " << cldom->name() << endl;
            m_classes->view()->setCurrentActiveItem(clmap[cldom.data()]);
        }

        kdDebug() << "trying function " << fndom->name() << endl;
        sync = true;
        m_functions->view()->setCurrentItem(fnmap[fndom.data()]);
    }

    if (!declarationFound)
    {
        //try to sync with definitions

        CodeModelUtils::AllFunctionDefinitions functions = CodeModelUtils::allFunctionDefinitionsDetailed(dom);
        FunctionDefinitionDom fndom;
        for (FunctionDefinitionList::Iterator it = functions.functionList.begin();
            it != functions.functionList.end(); ++it)
        {
            int startLine; int startColumn;
            (*it)->getStartPosition(&startLine, &startColumn);
            int endLine; int endColumn;
            (*it)->getEndPosition(&endLine, &endColumn);

            kdDebug() << "sync with " << (*it)->name() << " startLine " << startLine <<
                " endLine " << endLine << " line " << line << endl;

            if ( (line >= startLine) && (line <= endLine) )
            {
                fndom = *it;
                break;
            }
        }

        if (!fndom.data())
            return;
        NamespaceDom nsdom = functions.relations[fndom].ns;
        if (nsdom.data())
            kdDebug() << "namespace to try " << nsdom->name() << endl;
        else
            kdDebug() << "namespace data empty" << endl;

        for (QMap<NamespaceModel*, NamespaceItem*>::const_iterator it = nsmap.begin();
            it != nsmap.end(); ++it)
        {
            kdDebug() << " in nsmap " << it.key() << " data " << it.data() << endl;
            kdDebug() << " in nsmap " << it.key()->name() << " data " << it.data()->text(0) << endl;
        }

        kdDebug() << " in nsmap is ? " << nsdom.data() << endl;
        if (nsdom)
        {
            kdDebug() << " in nsmap is ? " << nsdom->name() << endl;
            if (nsdom->name() != "::")
                return;
        }

        if (nsdom.data() && nsmap[nsdom.data()])
        {
            kdDebug() << "trying namespace " << nsdom->name() << endl;
            m_namespaces->view()->setCurrentActiveItem(nsmap[nsdom.data()]);
        }
        else
        {
            kdDebug() << "trying global namespace " << endl;
            if (m_namespaces->view()->listView()->firstChild())
            {
                kdDebug() << "firstChild exists - global ns" << endl;
                m_namespaces->view()->setCurrentActiveItem(m_namespaces->view()->listView()->firstChild());

                kdDebug() << "trying to find item using global namespace" << endl;
                if (codeModel()->globalNamespace()->hasFunction(fndom->name()))
                {
                    sync = true;
                    FunctionModel *mod = const_cast<FunctionModel *>( codeModel()->globalNamespace()->functionByName(fndom->name()).first().data() );
                    QListViewItem *it = fnmap[mod];
                    if (it)
                        m_functions->view()->setCurrentItem(it);
                    return;
                }
            }
        }
        ClassDom cldom = functions.relations[fndom].klass;
        if (cldom.data() && clmap[cldom.data()])
        {
            kdDebug() << "trying class " << cldom->name() << endl;
            m_classes->view()->setCurrentActiveItem(clmap[cldom.data()]);

            kdDebug() << "trying to find item using class " << cldom->name() << endl;

            if (cldom->hasFunction(fndom->name()))
            {
                sync = true;
                QListViewItem *it = fnmap[cldom->functionByName(fndom->name()).first().data()];
                if (it)
                    m_functions->view()->setCurrentItem(it);
                return;
            }
        }

        kdDebug() << "trying function definition " << fndom->name()
             << " with scope " << fndom->scope().join("::") << endl;

        NamespaceDom currNs = codeModel()->globalNamespace();
        ClassDom currClass;
        for (QStringList::const_iterator it = fndom->scope().begin();
            it != fndom->scope().end(); ++it)
        {
            if (currNs->hasNamespace(*it))
            {
                currNs = currNs->namespaceByName(*it);
                kdDebug() << "resolved namespace " << *it << endl;
            }
            else if (currNs->hasClass(*it))
            {
                currClass = currNs->classByName(*it).first();
                kdDebug() << "resolved class " << *it << endl;
            }
            else if (currClass.data() && currClass->hasClass(*it))
            {
                currClass = currClass->classByName(*it).first();
                kdDebug() << "resolved nested class " << *it << endl;
            }
        }
        if (currClass.data())
        {
            kdDebug() << "trying to find item using resolved class " << currNs->name() << endl;
            if (currClass->hasFunction(fndom->name()))
            {
                sync = true;
                QListViewItem *it = fnmap[currClass->functionByName(fndom->name()).first().data()];
                if (it)
                    m_functions->view()->setCurrentItem(it);
                return;
            }
        }
        else if (currNs.data())
        {
            kdDebug() << "trying to find item using resolved namespace " << currNs->name() << endl;
            if (currNs->hasFunction(fndom->name()))
            {
                sync = true;
                FunctionModel *mod= currNs->functionByName(fndom->name()).first().data();
                QListViewItem *it = fnmap[mod];
                if (it)
                    m_functions->view()->setCurrentItem(it);
                return;
            }
        }

        sync = true;
//        m_functions->view()->setCurrentItem(fnmap[fndom.data()]);
    }
}
//     const NamespaceList nslist = dom->namespaceList();
//     for (NamespaceList::const_iterator it = nslist->begin(); it != namespaceList.end(); ++it)
//     {
//         nsdom = syncNamespaces(*it);
//     }

void ClassViewPart::activePartChanged( KParts::Part * part)
{
    kdDebug() << "ClassViewPart::activePartChanged()" << endl;

    m_activeDocument = dynamic_cast<KTextEditor::Document*>( part );
    m_activeView = part ? dynamic_cast<KTextEditor::View*>( part->widget() ) : 0;
    m_activeEditor = dynamic_cast<KTextEditor::EditInterface*>( part );
    m_activeSelection = dynamic_cast<KTextEditor::SelectionInterface*>( part );
    m_activeViewCursor = part ? dynamic_cast<KTextEditor::ViewCursorInterface*>( m_activeView ) : 0;

    m_activeFileName = QString::null;

    if (m_activeDocument)
    {
        m_activeFileName = URLUtil::canonicalPath( m_activeDocument->url().path() );
/*        if ( languageSupport()->mimeTypes().find(
            KMimeType::findByPath(m_activeFileName)) != languageSupport()->mimeTypes().end() )
            m_activeFileName = QString::null;*/
    }
}

NamespaceDom ClassViewPart::syncNamespaces( const FileDom & dom )
{
}

ClassDom ClassViewPart::syncClasses( const NamespaceDom & dom )
{
}

FunctionDom ClassViewPart::syncFunctions( const ClassDom & dom )
{
}

#include "classviewpart.moc"
