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
#include <qlineedit.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <kmimetype.h>

#include <kdevcore.h>
#include <kdevmainwindow.h>
#include <kdevlanguagesupport.h>
#include <kcomboview.h>
#include <kdevpartcontroller.h>
#include <kdevproject.h>
#include <urlutil.h>

#include <codemodel.h>
#include <codemodel_utils.h>

#include "classviewwidget.h"
#include "classviewpart.h"
#include "hierarchydlg.h"
#include "navigator.h"

#include "klistviewaction.h"

#include <ktexteditor/document.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/view.h>
#include <ktexteditor/selectioninterface.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/clipboardinterface.h>

typedef KDevGenericFactory<ClassViewPart> ClassViewFactory;
static const KAboutData data("kdevclassview", I18N_NOOP("Class browser"), "1.0");
K_EXPORT_COMPONENT_FACTORY( libkdevclassview, ClassViewFactory( &data ) )

ClassViewPart::ClassViewPart(QObject *parent, const char *name, const QStringList& )
    : KDevPlugin("ClassView", "classview", parent, name ? name : "ClassViewPart" ), sync(false),
    m_activeDocument(0), m_activeView(0), m_activeSelection(0), m_activeEditor(0), m_activeViewCursor(0)
{
    setInstance(ClassViewFactory::instance());
    setXMLFile("kdevclassview.rc");
    
    navigator = new Navigator(this);

    setupActions();

    global_item = 0;

    m_widget = new ClassViewWidget(this);
    m_widget->setIcon( SmallIcon("view_tree") );
    m_widget->setCaption(i18n("Class Browser"));
    mainWindow()->embedSelectView( m_widget, i18n("Classes"), i18n("Class browser") );
    QWhatsThis::add(m_widget, i18n("<b>Class browser</b><p>"
            "The class browser shows all namespaces, classes and namespace and class members in a project."));

    connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(slotProjectClosed()) );
    connect( core(), SIGNAL(languageChanged()), this, SLOT(slotProjectOpened()) );
    connect( partController(), SIGNAL(activePartChanged(KParts::Part*)),
        this, SLOT(activePartChanged(KParts::Part*)));
    connect( m_widget, SIGNAL(removedNamespace(const QString&)), this, SLOT(removeNamespace(const QString& )));

    m_classes->view()->setDefaultText(EmptyClasses);
    m_functions->view()->setDefaultText(EmptyFunctions);
}


ClassViewPart::~ClassViewPart()
{
    m_namespaces->view()->clear();
    m_classes->view()->clear();
    m_functions->view()->clear();

    mainWindow()->removeView( m_widget );
    delete (ClassViewWidget*) m_widget;
}

void ClassViewPart::slotProjectOpened( )
{
    connect( languageSupport(), SIGNAL(updatedSourceInfo()), this, SLOT(refresh()) );
    connect( languageSupport(), SIGNAL(updatedSourceInfo()), navigator, SLOT(refresh()) );
    connect( languageSupport(), SIGNAL(aboutToRemoveSourceInfo(const QString& )), this, SLOT(removeFile(const QString&)));
    connect( languageSupport(), SIGNAL(addedSourceInfo(const QString& )), this, SLOT(addFile(const QString& )));
    connect( languageSupport(), SIGNAL(addedSourceInfo(const QString& )), navigator, SLOT(addFile(const QString& )));
    
//    connect( languageSupport(), SIGNAL(addedSourceInfo(const QString& )), this, SLOT(refresh()));
}

void ClassViewPart::slotProjectClosed( )
{
    disconnect( languageSupport(), SIGNAL(updatedSourceInfo()), this, SLOT(refresh()) );
    disconnect( languageSupport(), SIGNAL(updatedSourceInfo()), navigator, SLOT(refresh()) );
    m_namespaces->view()->clear();
    m_classes->view()->clear();
    m_functions->view()->clear();
}

void ClassViewPart::setupActions( )
{
    m_functionsnav = new KListViewAction( new KComboView(true, 150, 0, "m_functionsnav_combo"), i18n("Functions Navigation"), 0, 0, 0, actionCollection(), "functionsnav_combo", true );
    connect(m_functionsnav->view(), SIGNAL(activated(QListViewItem*)), navigator, SLOT(selectFunctionNav(QListViewItem*)));
    connect(m_functionsnav->view(), SIGNAL(focusGranted()), navigator, SLOT(functionNavFocused()));
    connect(m_functionsnav->view(), SIGNAL(focusLost()), navigator, SLOT(functionNavUnFocused()));
    m_functionsnav->setToolTip(i18n("Functions in file"));
    m_functionsnav->setWhatsThis(i18n("<b>Function navigator</b><p>Navigates over functions contained in the file."));
    m_functionsnav->view()->setCurrentText(NAV_NODEFINITION);
    
    m_followCode = new KAction(i18n("Synchronize"), "dirsynch", 0, this, SLOT(syncCombos()), actionCollection(), "sync_combos");
    m_followCode->setToolTip(i18n("Synchronize selectors"));
    m_followCode->setWhatsThis(i18n("<b>Synchronize</b><p>Synchronize namespaces, classes and functions selectors with the current position in code."));    
    
    m_namespaces = new KListViewAction( new KComboView(true, 150, 0, "m_namespaces_combo"), i18n("Namespaces"), 0, 0, 0, actionCollection(), "namespaces_combo", true );
    connect( m_namespaces->view(), SIGNAL(activated(QListViewItem*)), this, SLOT(selectNamespace(QListViewItem*)) );
    connect( m_namespaces->view(), SIGNAL(focusGranted()), this, SLOT(focusNamespaces()) );
    connect( m_namespaces->view(), SIGNAL(focusLost()), this, SLOT(unfocusNamespaces()) );
    m_namespaces->setToolTip(i18n("Namespaces"));
    m_namespaces->setWhatsThis(i18n("<b>Namespace selector</b><p>Select a namespace to view classes and functions contained in it."));

    m_classes = new KListViewAction( new KComboView(true, 150, 0, "m_classes_combo"), i18n("Classes"), 0, 0, 0, actionCollection(), "classes_combo", true );
    connect( m_classes->view(), SIGNAL(activated(QListViewItem*)), this, SLOT(selectClass(QListViewItem*)) );
    connect( m_classes->view(), SIGNAL(focusGranted()), this, SLOT(focusClasses()) );
    connect( m_classes->view(), SIGNAL(focusLost()), this, SLOT(unfocusClasses()) );
    m_classes->setToolTip(i18n("Classes"));
    m_classes->setWhatsThis(i18n("<b>Class selector</b><p>Select a class to view its members."));

    m_functions = new KListViewAction( new KComboView(true, 300, 0, "m_functions_combo"), i18n("Functions"), 0, 0, 0, actionCollection(), "functions_combo", true );
    connect( m_functions->view(), SIGNAL(activated(QListViewItem*)), this, SLOT(selectFunction(QListViewItem*)) );
    connect( m_functions->view(), SIGNAL(focusGranted()), this, SLOT(focusFunctions()) );
    connect( m_functions->view(), SIGNAL(focusLost()), this, SLOT(unfocusFunctions()) );
    m_functions->setToolTip(i18n("Functions"));
    m_functions->setWhatsThis(i18n("<b>Function selector</b><p>Select a function to jump to it's definition or declaration."));

//    m_namespaces->view()->setMinimumWidth(150);
//    m_classes->view()->setMinimumWidth(150);
//    m_functions->view()->setMinimumWidth(300);

    m_popupAction  = new KToolBarPopupAction(i18n("Class Browser Actions"), "classwiz", 0,
                                           this, SLOT(switchedViewPopup()),
                                           actionCollection(), "view_popup");
    m_popupAction->setToolTip(i18n("Class browser actions"));
    m_popupAction->setWhatsThis(i18n("<b>Class browser actions</b><p>A menu for commonly used class browser actions "
                                     "like switch between function declaration and definition, "
                                     "add classes, methods and attributes, "
                                     "inheritance diagram."));
    m_popupAction->setDelayed(false);
    KPopupMenu *popup = m_popupAction->popupMenu();
    //@todo check if language support has namespaces, classes, etc.
//    KDevLanguageSupport::Features features = languageSupport()->features();
    int id = popup->insertItem(i18n("Go to Function Declaration"), this, SLOT(goToFunctionDeclaration()));
    popup->setWhatsThis(id, i18n("<b>Go to function declaration</b><p>Opens a file where the function is declared and jumps to the declaration line."));
    id = popup->insertItem(i18n("Go to Function Definition"), this, SLOT(goToFunctionDefinition()));
    popup->setWhatsThis(id, i18n("<b>Go to function definition</b><p>Opens a file where the function is defined (implemented) and jumps to the definition line."));
    id = popup->insertItem(i18n("Go to Class Declaration"), this, SLOT(goToClassDeclaration()));
    popup->setWhatsThis(id, i18n("<b>Go to class declaration</b><p>Opens a file where the class is declared and jumps to the declaration line."));

    //@todo not applicable to c++ but can be useful for ada and pascal where namespace is contained
    //in a single compilation unit
    /// \FIXME commented out till the time when it be needed
//    popup->insertItem(i18n("Go to Namespace Declaration"), this, SLOT(goToNamespaceDeclaration()));

    bool hasAddMethod = langHasFeature(KDevLanguageSupport::AddMethod);
    bool hasAddAttribute = langHasFeature(KDevLanguageSupport::AddAttribute);
    bool hasNewClass = langHasFeature(KDevLanguageSupport::NewClass);
    if (hasAddMethod || hasAddAttribute || hasNewClass)
        popup->insertSeparator();
    if (hasNewClass)
    {
        id = popup->insertItem(SmallIcon("classnew"), i18n("New Class..."), this, SLOT(selectedAddClass()));
        popup->setWhatsThis(id, i18n("<b>New class</b><p>Calls the <b>New Class</b> wizard."));
    }
#if 0 /// \FIXME seems that the 'access attribute' of the 'CodeModel' is wrong!!!!!
    if (hasAddMethod)
    {
        id = popup->insertItem(SmallIcon("methodnew"), i18n("Add Method..."), this, SLOT(selectedAddMethod()));
        popup->setWhatsThis(id, i18n("<b>Add method</b><p>Calls the <b>New Method</b> wizard."));
    }
    if (hasAddAttribute)
    {
        id = popup->insertItem(SmallIcon("variablenew"), i18n("Add Attribute..."), this, SLOT(selectedAddAttribute()));
        popup->setWhatsThis(id, i18n("<b>Add attribute</b><p>Calls the <b>New Attribute</b> wizard."));
    }
#endif

    if (langHasFeature(KDevLanguageSupport::Classes))
    {
        KAction *ac = new KAction(i18n("Class Inheritance Diagram"), "view_tree", 0, this, SLOT(graphicalClassView()), actionCollection(), "inheritance_dia");
        ac->setToolTip(i18n("Class inheritance diagram"));
        ac->setWhatsThis(i18n("<b>Class inheritance diagram</b><p>Displays inheritance relationship between classes in project. "
                                 "Note, it does not display classes outside inheritance hierarchy."));    
    }

    popup->insertSeparator();
    id = popup->insertItem(i18n("Class Inheritance Diagram"), this, SLOT(graphicalClassView()));
    popup->setWhatsThis(id, i18n("<b>Class inheritance diagram</b><p>Displays inheritance relationship between classes in project. "
                                 "Note, it does not display classes outside inheritance hierarchy."));
}

void ClassViewPart::refresh( )
{
    kdDebug() << "ClassViewPart::refresh" << endl;
    ViewCombosOp::refreshNamespaces(this, m_namespaces->view());
}

void ClassViewPart::selectNamespace( QListViewItem * item )
{
    NamespaceItem *ni = dynamic_cast<NamespaceItem*>(item);
    if (!ni)
        return;
    ViewCombosOp::refreshClasses(this, m_classes->view(), ni->dom()->name());
    ViewCombosOp::refreshFunctions(this, m_functions->view(), ni->dom()->name());
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
    //adymo: this jumps to declaration - commented
/*  FunctionItem *fi = dynamic_cast<FunctionItem*>(item);
    if (!fi)
        return;
    int startLine, startColumn;
    /// \FIXME ROBE
    //fi->dom()->getImplementationStartPosition( &startLine, &startColumn );
    //if (startLine != 0)
    //    partController()->editDocument( KURL(fi->dom()->implementedInFile()), startLine );
    //else
    {
        fi->dom()->getStartPosition( &startLine, &startColumn );
        partController()->editDocument( KURL(fi->dom()->fileName()), startLine );
    }*/

    //adymo: this jumps to definition or declaration if the definition is not found
    FunctionItem *fi = dynamic_cast<FunctionItem*>(item);
    if (!fi)
        return;
    int startLine, startColumn;

    FunctionDefinitionList lst;
    FileList fileList = codeModel()->fileList();
    CodeModelUtils::findFunctionDefinitions( FindOp(fi->dom()), fileList, lst );

    if( lst.isEmpty() )
    {   //definition not found, try declaration instead
        int startLine, startColumn;
        fi->dom()->getStartPosition( &startLine, &startColumn );
        partController()->editDocument( KURL(fi->dom()->fileName()), startLine );
    }
    else
    {   //jump to definition
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
//    else
//        m_classes->view()->lineEdit()->selectAll();
}

void ClassViewPart::focusFunctions( )
{
    if (m_functions->view()->currentText() == EmptyFunctions)
        m_functions->view()->setCurrentText("");
//    else
//        m_functions->view()->lineEdit()->selectAll();
}

void ClassViewPart::unfocusClasses( )
{
    if (m_classes->view()->currentText().isEmpty())
        m_classes->view()->setCurrentText(EmptyClasses);
//    m_classes->view()->lineEdit()->deselect();
}

void ClassViewPart::unfocusFunctions( )
{
    if (m_functions->view()->currentText().isEmpty())
        m_functions->view()->setCurrentText(EmptyFunctions);
//    m_functions->view()->lineEdit()->deselect();
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

        if ( (int(line) >= startLine) && (int(line) <= endLine) )
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

        for (QMap<QString, NamespaceItem*>::const_iterator it = nsmap.begin();
            it != nsmap.end(); ++it)
        {
            kdDebug() << " in nsmap " << it.key() << " data " << it.data() << endl;
            kdDebug() << " in nsmap " << it.key() << " data " << it.data()->text(0) << endl;
        }

        kdDebug() << " in nsmap is ? " << nsdom.data() << endl;
        if (nsdom)
        {
            kdDebug() << " in nsmap is ? " << nsdom->name() << endl;
            if (nsdom->name() != "::")
                return;
        }

        if (nsdom.data() && nsmap[nsdom->name()])
        {
            kdDebug() << "trying namespace " << nsdom->name() << endl;
            m_namespaces->view()->setCurrentActiveItem(nsmap[nsdom->name()]);
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

            if ( (int(line) >= startLine) && (int(line) <= endLine) )
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

        for (QMap<QString, NamespaceItem*>::const_iterator it = nsmap.begin();
            it != nsmap.end(); ++it)
        {
            kdDebug() << " in nsmap " << it.key() << " data " << it.data() << endl;
            kdDebug() << " in nsmap " << it.key() << " data " << it.data()->text(0) << endl;
        }

        kdDebug() << " in nsmap is ? " << nsdom.data() << endl;
        if (nsdom)
        {
            kdDebug() << " in nsmap is ? " << nsdom->name() << endl;
            if (nsdom->name() != "::")
                return;
        }

        if (nsdom.data() && nsmap[nsdom->name()])
        {
            kdDebug() << "trying namespace " << nsdom->name() << endl;
            m_namespaces->view()->setCurrentActiveItem(nsmap[nsdom->name()]);
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
    navigator->stopTimer();
    if (m_activeView)
    {
        disconnect(m_activeView, SIGNAL(cursorPositionChanged()), 
            navigator, SLOT(slotCursorPositionChanged()));
    }
    
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
        navigator->refreshNavBars(m_activeFileName);
        navigator->syncFunctionNavDelayed(200);
/*        if ( languageSupport()->mimeTypes().find(
            KMimeType::findByPath(m_activeFileName)) != languageSupport()->mimeTypes().end() )
            m_activeFileName = QString::null;*/
    }
    if( m_activeViewCursor )
    {
        connect(m_activeView, SIGNAL(cursorPositionChanged()),
            navigator, SLOT(slotCursorPositionChanged()) );
    }
}

NamespaceDom ClassViewPart::syncNamespaces( const FileDom & /*dom*/ )
{
    return NamespaceDom();
}

ClassDom ClassViewPart::syncClasses( const NamespaceDom & /*dom*/ )
{
    return ClassDom();
}

FunctionDom ClassViewPart::syncFunctions( const ClassDom & /*dom*/ )
{
    return FunctionDom();
}

void ClassViewPart::focusNamespaces( )
{
    //m_namespaces->view()->lineEdit()->selectAll();
}

void ClassViewPart::unfocusNamespaces( )
{
//    m_namespaces->view()->lineEdit()->deselect();
}

void ClassViewPart::removeFile( const QString & fileName )
{
    QString fn = URLUtil::canonicalPath( fileName );
    if( !project()->isProjectFile(fn) )
        return;

    FileDom file = codeModel()->fileByName(fileName);
    if (!file)
        return;

    if (ViewCombosOp::removeNamespacesItems(this, m_namespaces->view()->listView(), model_cast<NamespaceDom>(file)))
    {
        if (global_item)
            m_namespaces->view()->setCurrentActiveItem(global_item);
    }

    ViewCombosOp::removeClassItems(this, m_namespaces->view()->listView(), model_cast<ClassDom>(file));

    ViewCombosOp::removeFunctionItems(this, m_namespaces->view()->listView(), model_cast<ClassDom>(file));
}

void ClassViewPart::addFile( const QString & fileName )
{
    QString fn = URLUtil::canonicalPath( fileName );
    if( !project()->isProjectFile(fn) )
        return;

    FileDom file = codeModel()->fileByName(fileName);
    if (!file)
        return;

    //check for namespaces in file
    NamespaceList namespaceList = file->namespaceList();
    for (NamespaceList::const_iterator it = namespaceList.begin(); it != namespaceList.end(); ++it)
    {
        NamespaceDom nsdom = *it;
        if (nsmap.contains(nsdom->name()))
        {
            //namespace item exists - update
            NamespaceItem *ns = nsmap[nsdom->name()];
            ns->setText(0, languageSupport()->formatModelItem(nsdom));
            if (m_namespaces->view()->currentItem() == ns)
            {
                //reload this and dependent combos because namespace item is currently selected
                m_namespaces->view()->setCurrentText(languageSupport()->formatModelItem(nsdom));

                //check classes
                updateClassesForAdd(nsdom);
                //check functions
                updateFunctionsForAdd(model_cast<ClassDom>(nsdom));
            }
            //refresh info about nested namespaces
            kdDebug() << "nested ns check for " << nsdom->name() << endl;
            ViewCombosOp::processNamespace(this, m_namespaces->view(), ns, ViewCombosOp::Refresh);
        }
        else
        {
            //namespace item does not exist - create
            NamespaceItem *item = new NamespaceItem(this, m_namespaces->view()->listView(), languageSupport()->formatModelItem(nsdom), nsdom);
            m_namespaces->view()->addItem(item);
            item->setOpen(true);
            ViewCombosOp::processNamespace(this, m_namespaces->view(), item);
        }
    }

    if (m_namespaces->view()->currentItem())
    {
        NamespaceItem *ni = dynamic_cast<NamespaceItem*>(m_namespaces->view()->currentItem());
        if (ni && (ni->dom() == codeModel()->globalNamespace()))
        {
            //check for classes in file (global namespace)
            updateClassesForAdd(model_cast<NamespaceDom>(file));

            //check for funtions in file (global namespace);
            updateFunctionsForAdd(model_cast<ClassDom>(file));
        }
    }
}

void ClassViewPart::updateFunctionsForAdd( ClassDom cldom )
{
    FunctionList functionList = cldom->functionList();
    for (FunctionList::const_iterator it3 = functionList.begin();
        it3 != functionList.end(); ++it3)
    {
        FunctionDom fndom = *it3;
        if (fnmap.contains(fndom))
        {
            //function item exists - update
            FunctionItem *fn = fnmap[fndom];
            fn->setText(0, languageSupport()->formatModelItem(fndom, true));
            fn->setup();
            if (m_functions->view()->currentItem() == fn)
            {
                //reload this combo because function item is currently selected
                m_functions->view()->setCurrentText(languageSupport()->formatModelItem(fndom, true));
            }
            //refresh info about nested functions
            ViewCombosOp::processFunction(this, m_functions->view(), fn, ViewCombosOp::Refresh);
        }
        else
        {
            //function item does not exists - create
            FunctionItem *item = new FunctionItem(this, m_functions->view()->listView(), languageSupport()->formatModelItem(fndom, true), fndom);
            m_functions->view()->addItem(item);
            item->setOpen(true);
            ViewCombosOp::processFunction(this, m_functions->view(), item);
        }
    }
}

void ClassViewPart::updateClassesForAdd( NamespaceDom nsdom )
{
    ClassList classList = nsdom->classList();
    for (ClassList::const_iterator it2 = classList.begin(); it2 != classList.end(); ++it2)
    {
        ClassDom cldom = *it2;
        if (clmap.contains(cldom))
        {
            //class item exists - update
            ClassItem *cl = clmap[cldom];
            cl->setText(0, languageSupport()->formatModelItem(cldom));
            if (m_classes->view()->currentItem() == cl)
            {
                //reload this and dependent combos because class item is currently selected
                m_classes->view()->setCurrentText(languageSupport()->formatModelItem(cldom));

                //check functions
                updateFunctionsForAdd(cldom);
            }
            //refresh info about nested classes
            ViewCombosOp::processClass(this, m_classes->view(), cl, ViewCombosOp::Refresh);
        }
        else
        {
            //class item does not exists - create
            ClassItem *item = new ClassItem(this, m_classes->view()->listView(), languageSupport()->formatModelItem(cldom), cldom);
            m_classes->view()->addItem(item);
            item->setOpen(true);
            ViewCombosOp::processClass(this, m_classes->view(), item);
        }
    }
}

void ClassViewPart::removeNamespace( const QString & name )
{
    if (nsmap.contains(name))
    {
        NamespaceItem *i = nsmap[name];
        if (i)
        {
            m_namespaces->view()->removeItem(i);
            if (global_item)
                m_namespaces->view()->setCurrentActiveItem(global_item);
        }
    }
}

#include "classviewpart.moc"
