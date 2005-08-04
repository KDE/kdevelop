/***************************************************************************
 *   Copyright (C) 2005 by Mathieu Chouinard                               *
 *   mchoui@e-c.qc.ca                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/

#include "annotationpart.h"

#include <qtimer.h>
#include <q3popupmenu.h>
#include <q3whatsthis.h>

#include <klocale.h>
#include <kaction.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kdevplugininfo.h>
#include <kdevgenericfactory.h>
#include <kaboutdata.h>
#include <kdevcore.h>
#include <kdevmainwindow.h>
#include <kurl.h>
#include <configwidgetproxy.h>

#include <kdebug.h>
#include <codemodel.h>
#include <codemodel_utils.h>
#include "annotationwidget.h"
#include "annotationglobalconfig.h"
#include "annotationprojectconfig.h"

typedef KDevGenericFactory<annotationPart> annotationFactory;
KDevPluginInfo data("kdevannotation");
K_EXPORT_COMPONENT_FACTORY( libkdevannotation, annotationFactory( data ) );

#define GLOBALDOC_OPTIONS 1
#define PROJECTDOC_OPTIONS 2

annotationPart::annotationPart(QObject *parent, const char *name, const QStringList &/*args*/)
    : KDevPlugin(&data, parent, name ? name : "annotationPart")
{
    setInstance(annotationFactory::instance());
    setXMLFile("kdevannotation.rc");

    m_widget = new annotationWidget(this);
    m_widget->setCaption("Annotations");
    m_widget->setIcon(SmallIcon("notes"));

    Q3WhatsThis::add(m_widget, i18n("Some annotation about the code"));
    
    // now you decide what should happen to the widget. Take a look at kdevcore.h
    // or at other plugins how to embed it.
    
    // if you want to embed your widget as an outputview, simply uncomment
    // the following line.
    //mainWindow()->embedOutputView( m_widget, "Annotations", "Annotations" );
    
    // if you want to embed your widget as a selectview (at the left), simply uncomment
    // the following line.
    // mainWindow()->embedSelectView( m_widget, "name that should appear", "enter a tooltip" );
    
    // if you want to embed your widget as a selectview (at the right), simply uncomment
    // the following line.
     mainWindow()->embedSelectViewRight( m_widget, "Annotations", "annotations" );
    
    setupActions();
    
    m_configProxy = new ConfigWidgetProxy(core());
    m_configProxy->createGlobalConfigPage(i18n("annotation"), GLOBALDOC_OPTIONS, info()->icon());
    m_configProxy->createProjectConfigPage(i18n("annotation"), PROJECTDOC_OPTIONS, info()->icon());
    connect(m_configProxy, SIGNAL(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )),
        this, SLOT(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int)));

    connect(core(), SIGNAL(contextMenu(Q3PopupMenu *, const Context *)),
        this, SLOT(contextMenu(Q3PopupMenu *, const Context *)));
    connect(core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()));
    connect(core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()));
  
        
    QTimer::singleShot(0, this, SLOT(init()));
}

annotationPart::~annotationPart()
{
// if you embed a widget, you need to tell the mainwindow when you remove it
//     if ( m_widget )
//     {
//         mainWindow()->removeView( m_widget );
//     }
    delete m_widget;
}

void annotationPart::init()
{
// delayed initialization stuff goes here
}

void annotationPart::setupActions()
{
// create XMLGUI actions here
  
}

void annotationPart::insertConfigWidget(const KDialogBase *dlg, QWidget *page, unsigned int pageNo)
{
// create configuraton dialogs here
    switch (pageNo)
    {
        case GLOBALDOC_OPTIONS:
        {
            annotationGlobalConfig *w = new annotationGlobalConfig(this, page, "global config");
            connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
            break;
        }
        case PROJECTDOC_OPTIONS:
        {
            annotationProjectConfig *w = new annotationProjectConfig(this, page, "project config");
            connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
            break;
        }
    }
}

void annotationPart::contextMenu(Q3PopupMenu *popup, const Context *context)
{
// put actions into the context menu here
    if (context->hasType(Context::EditorContext))
    {
        // editor context menu
        const EditorContext *econtext = static_cast<const EditorContext*>(context);
        
        // use context and plug actions here
        action->plug(popup);
        
        // or create menu items on the fly
         int id = -1;
	 KURL tmpurl = econtext->url();
	 m_itemAnnotationFilename = tmpurl.filename(true);
	 m_itemAnnotationName = econtext->currentWord();
	 popup->insertSeparator();
	 id = popup->insertItem(i18n("Add Annotation"),this, SLOT(SlotDoAnnotate( ) ) );
         popup->setWhatsThis(id, i18n("<b>Add Annotation</b><p>Add out of source comment"));
    }
   
    else if (context->hasType(Context::CodeModelItemContext)) 
    {
        // class tree context menu
        const CodeModelItemContext *mcontext = static_cast<const CodeModelItemContext*>(context);
        
        // use context and plug actions here
	int id = -1;
	m_itemAnnotationName = mcontext->item()->name();
	m_itemAnnotationFilename = mcontext->item()->fileName();
	
	id = popup->insertItem(i18n("Add Annotations"),this, SLOT(SlotDoAnnotate()) );
	popup->setWhatsThis(id, i18n("<b>Do something here</b><p>Describe here what does this action do."));
        
    }
  
}

void annotationPart::projectOpened()
{
// do something when the project is opened
}

void annotationPart::projectClosed()
{
// do something when the project is closed
}

void annotationPart::doSomething()
{
// do something useful here instead of showing the message box
    KMessageBox::information(m_widget, i18n("This action does nothing."), i18n("Annotation Plugin"));
}

#include "annotationpart.moc"


/*!
    \fn annotationPart::SlotDoAnnotate()
 */
void annotationPart::SlotDoAnnotate()
{
  kdDebug()<< "Annotation: Editor Windows";
  KMessageBox::information(m_widget,m_itemAnnotationFilename +":"+  m_itemAnnotationName , i18n("Annotation Plugin"));
}

void annotationPart::SlotDoAnnotate(QString itemname)
{
  kdDebug()<< "Annotation: "<<itemname;

  KMessageBox::information(0 ,itemname,i18n("Annotation Plugin"));
//  kdDebug()<< context->item()->name();
}
/*!
    \fn annotationPart::aboutData()
 */
KAboutData* annotationPart::aboutData()
{
  KAboutData *data = new KAboutData  ("annotationpart", I18N_NOOP("annotationPart"), "0.3",
				      I18N_NOOP( "AnnotationPart for KDevelop" ),
				      KAboutData::License_LGPL_V2,
				      I18N_NOOP( "(c) 2005" ), 0, "http://www.kdevelop.org");
  data->addAuthor ("Mathieu Chouinard", I18N_NOOP("Author"), "mchoui@e-c.qc.ca", "http://ulyx.ath.cx");


  return data;
}
