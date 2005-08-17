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

#include <q3whatsthis.h>
#include <q3listview.h>
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
#include <kdevplugininfo.h>

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
static const KDevPluginInfo data("kdevclassview");
K_EXPORT_COMPONENT_FACTORY( libkdevclassview, ClassViewFactory( data ) )

ClassViewPart::ClassViewPart(QObject *parent, const char *name, const QStringList& )
    : KDevPlugin(&data, parent, name ? name : "ClassViewPart" ),
    m_activeDocument(0), m_activeView(0), m_activeSelection(0), m_activeEditor(0), m_activeViewCursor(0)
{
    setInstance(ClassViewFactory::instance());
    setXMLFile("kdevclassview.rc");
    
    navigator = new Navigator(this);

    setupActions();

    m_widget = new ClassViewWidget(this);
    m_widget->setIcon( SmallIcon("view_tree") );
    m_widget->setCaption(i18n("Class Browser"));
    mainWindow()->embedSelectView( m_widget, i18n("Classes"), i18n("Class browser") );
    Q3WhatsThis::add(m_widget, i18n("<b>Class browser</b><p>"
            "The class browser shows all namespaces, classes and namespace and class members in a project."));

    connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(slotProjectClosed()) );
    connect( core(), SIGNAL(languageChanged()), this, SLOT(slotProjectOpened()) );
    connect( partController(), SIGNAL(activePartChanged(KParts::Part*)),
        this, SLOT(activePartChanged(KParts::Part*)));
    connect( m_widget, SIGNAL(removedNamespace(const QString&)), this, SLOT(removeNamespace(const QString& )));
}


ClassViewPart::~ClassViewPart()
{
    mainWindow()->removeView( m_widget );
    delete (ClassViewWidget*) m_widget;
}

void ClassViewPart::slotProjectOpened( )
{
//    connect( languageSupport(), SIGNAL(updatedSourceInfo()), navigator, SLOT(refresh()) );
    connect( languageSupport(), SIGNAL(addedSourceInfo(const QString& )), navigator, SLOT(addFile(const QString& )));
    navigator->refresh();
}

void ClassViewPart::slotProjectClosed( )
{
//    navigator->refresh();
//    disconnect( languageSupport(), SIGNAL(updatedSourceInfo()), navigator, SLOT(refresh()) );
}

void ClassViewPart::setupActions( )
{
    m_functionsnav = new KListViewAction( new KComboView(true, 150, 0, "m_functionsnav_combo"), i18n("Functions Navigation"), 0, 0, 0, actionCollection(), "functionsnav_combo", true );
    connect(m_functionsnav->view(), SIGNAL(activated(Q3ListViewItem*)), navigator, SLOT(selectFunctionNav(Q3ListViewItem*)));
//    m_functionsnav->view()->setEditable(false);
    connect(m_functionsnav->view(), SIGNAL(focusGranted()), navigator, SLOT(functionNavFocused()));
    connect(m_functionsnav->view(), SIGNAL(focusLost()), navigator, SLOT(functionNavUnFocused()));
    m_functionsnav->setToolTip(i18n("Functions in file"));
    m_functionsnav->setWhatsThis(i18n("<b>Function navigator</b><p>Navigates over functions contained in the file."));
//    m_functionsnav->view()->setCurrentText(NAV_NODEFINITION);
    m_functionsnav->view()->setDefaultText(NAV_NODEFINITION);
    
    if (langHasFeature(KDevLanguageSupport::Classes))
    {
        KAction *ac = new KAction(i18n("Class Inheritance Diagram"), "view_tree", 0, this, SLOT(graphicalClassView()), actionCollection(), "inheritance_dia");
        ac->setToolTip(i18n("Class inheritance diagram"));
        ac->setWhatsThis(i18n("<b>Class inheritance diagram</b><p>Displays inheritance relationship between classes in project. "
                                 "Note, it does not display classes outside inheritance hierarchy."));    
    }
}

bool ClassViewPart::langHasFeature(KDevLanguageSupport::Features feature)
{
    bool result = false;
    if (languageSupport())
        result = (feature & languageSupport()->features());
    return result;
}

void ClassViewPart::graphicalClassView( )
{
    HierarchyDialog dia(this);
    dia.exec();
}

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

#include "classviewpart.moc"
