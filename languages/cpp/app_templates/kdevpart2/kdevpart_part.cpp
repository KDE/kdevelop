%{CPP_TEMPLATE}
#include "%{APPNAMELC}part.h"

#include <qtimer.h>
#include <qpopupmenu.h>
#include <qwhatsthis.h>

#include <klocale.h>
#include <kaction.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kdevplugininfo.h>
#include <kdevgenericfactory.h>

#include <kdevcore.h>
#include <kdevmainwindow.h>
#include <configwidgetproxy.h>

#include "%{APPNAMELC}widget.h"
#include "%{APPNAMELC}globalconfig.h"
#include "%{APPNAMELC}projectconfig.h"

typedef KDevGenericFactory<%{APPNAME}Part> %{APPNAME}Factory;
KDevPluginInfo data("kdev%{APPNAMELC}");
K_EXPORT_COMPONENT_FACTORY( libkdev%{APPNAMELC}, %{APPNAME}Factory( data ) );

#define GLOBALDOC_OPTIONS 1
#define PROJECTDOC_OPTIONS 2

%{APPNAME}Part::%{APPNAME}Part(QObject *parent, const char *name, const QStringList &/*args*/)
    : KDevPlugin(&data, parent, name ? name : "%{APPNAME}Part")
{
    setInstance(%{APPNAME}Factory::instance());
    setXMLFile("kdev%{APPNAMELC}.rc");

    m_widget = new %{APPNAME}Widget(this);
    m_widget->setCaption("widget caption");
    m_widget->setIcon(SmallIcon(info()->icon()));

    QWhatsThis::add(m_widget, i18n("WHAT DOES THIS PART DO?"));
    
    // now you decide what should happen to the widget. Take a look at kdevcore.h
    // or at other plugins how to embed it.
    
    // if you want to embed your widget as an outputview, simply uncomment
    // the following line.
    // mainWindow()->embedOutputView( m_widget, "name that should appear", "enter a tooltip" );
    
    // if you want to embed your widget as a selectview (at the left), simply uncomment
    // the following line.
    // mainWindow()->embedSelectView( m_widget, "name that should appear", "enter a tooltip" );
    
    // if you want to embed your widget as a selectview (at the right), simply uncomment
    // the following line.
    // mainWindow()->embedSelectViewRight( m_widget, "name that should appear", "enter a tooltip" );
    
    setupActions();
    
    m_configProxy = new ConfigWidgetProxy(core());
    m_configProxy->createGlobalConfigPage(i18n("%{APPNAME}"), GLOBALDOC_OPTIONS, info()->icon());
    m_configProxy->createProjectConfigPage(i18n("%{APPNAME}"), PROJECTDOC_OPTIONS, info()->icon());
    connect(m_configProxy, SIGNAL(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )),
        this, SLOT(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int)));

    connect(core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
        this, SLOT(contextMenu(QPopupMenu *, const Context *)));
    connect(core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()));
    connect(core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()));
  
        
    QTimer::singleShot(0, this, SLOT(init()));
}

%{APPNAME}Part::~%{APPNAME}Part()
{
// if you embed a widget, you need to tell the mainwindow when you remove it
//     if ( m_widget )
//     {
//         mainWindow()->removeView( m_widget );
//     }
    delete m_widget;
    delete m_configProxy;
}

void %{APPNAME}Part::init()
{
// delayed initialization stuff goes here
}

void %{APPNAME}Part::setupActions()
{
// create XMLGUI actions here
    action = new KAction(i18n("&Do Something..."), 0,
        this, SLOT(doSomething()), actionCollection(), "plugin_action" );
    action->setToolTip(i18n("Do something"));
    action->setWhatsThis(i18n("<b>Do something</b><p>Describe here what does this action do."));
}

void %{APPNAME}Part::insertConfigWidget(const KDialogBase *dlg, QWidget *page, unsigned int pageNo)
{
// create configuraton dialogs here
    switch (pageNo)
    {
        case GLOBALDOC_OPTIONS:
        {
            %{APPNAME}GlobalConfig *w = new %{APPNAME}GlobalConfig(this, page, "global config");
            connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
            break;
        }
        case PROJECTDOC_OPTIONS:
        {
            %{APPNAME}ProjectConfig *w = new %{APPNAME}ProjectConfig(this, page, "project config");
            connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
            break;
        }
    }
}

void %{APPNAME}Part::contextMenu(QPopupMenu *popup, const Context *context)
{
// put actions into the context menu here
    if (context->hasType(Context::EditorContext))
    {
        // editor context menu
        const EditorContext *econtext = static_cast<const EditorContext*>(context);
        
        // use context and plug actions here
        action->plug(popup);
        
        // or create menu items on the fly
        // int id = -1;
        // id = popup->insertItem(i18n("Do Something Here")),
        //     this, SLOT(doSomething()));
        // popup->setWhatsThis(id, i18n("<b>Do something here</b><p>Describe here what does this action do."
    }
    else if (context->hasType(Context::FileContext)) 
    {
        // file context menu
        const FileContext *fcontext = static_cast<const FileContext*>(context);
        
        //use context and plug actions here
    }
    else if (context->hasType(Context::ProjectModelItemContext)) 
    {
        // project tree context menu
        const ProjectModelItemContext *pcontext = static_cast<const ProjectModelItemContext*>(context);
        
        // use context and plug actions here
    }
    else if (context->hasType(Context::CodeModelItemContext)) 
    {
        // class tree context menu
        const CodeModelItemContext *mcontext = static_cast<const CodeModelItemContext*>(context);
        
        // use context and plug actions here
    }
    else if (context->hasType(Context::DocumentationContext)) 
    {
        // documentation viewer context menu
        const DocumentationContext *dcontext = static_cast<const DocumentationContext*>(context);
        
        // use context and plug actions here
    }
}

void %{APPNAME}Part::projectOpened()
{
// do something when the project is opened
}

void %{APPNAME}Part::projectClosed()
{
// do something when the project is closed
}

void %{APPNAME}Part::doSomething()
{
// do something useful here instead of showing the message box
    KMessageBox::information(m_widget, i18n("This action does nothing."), i18n("%{APPNAME} Plugin"));
}

#include "%{APPNAMELC}part.moc"
