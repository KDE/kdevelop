
#include "plugin_%{APPNAMELC}.h"

#include <kaction.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>

#include <qlayout.h>
#include <qlabel.h>

class PluginView : public KXMLGUIClient
{
    friend class KatePlugin%{APPNAME};

public:
    Kate::MainWindow *win;
};

extern "C"
{
    void* init_lib%{APPNAMELC}plugin()
    {
        KGlobal::locale()->insertCatalogue("kate%{APPNAMELC}");
        return new KatePluginFactory;
    }
}

KatePluginFactory::KatePluginFactory()
{
    s_instance = new KInstance( "kate" );
}

KatePluginFactory::~KatePluginFactory()
{
    delete s_instance;
}

QObject* KatePluginFactory::createObject( QObject* parent, const char* name, const char*, const QStringList & )
{
    return new KatePlugin%{APPNAME}( parent, name );
}

KInstance* KatePluginFactory::s_instance = 0L;

KatePlugin%{APPNAME}::KatePlugin%{APPNAME}( QObject* parent, const char* name )
    : Kate::Plugin ( (Kate::Application*)parent, name )
{
}

KatePlugin%{APPNAME}::~KatePlugin%{APPNAME}()
{
}

void KatePlugin%{APPNAME}::addView(Kate::MainWindow *win)
{
    /// @todo doesn't this have to be deleted?
    PluginView *view = new PluginView ();

    (void) new KAction ( i18n("Insert Hello World"), 0, this,
                         SLOT( slotInsertHello() ), view->actionCollection(),
                         "edit_insert_%{APPNAMELC}" );

    view->setInstance (new KInstance("kate"));
    view->setXMLFile("plugins/%{APPNAMELC}/plugin_%{APPNAMELC}.rc");
    win->guiFactory()->addClient (view);
    view->win = win;

    m_views.append (view);
}

void KatePlugin%{APPNAME}::removeView(Kate::MainWindow *win)
{
    for (uint z=0; z < m_views.count(); z++)
        if (m_views.at(z)->win == win)
        {
            PluginView *view = m_views.at(z);
            m_views.remove (view);
            win->guiFactory()->removeClient (view);
            delete view;
        }
}

void KatePlugin%{APPNAME}::slotInsertHello()
{
    Kate::View *kv = application()->activeMainWindow()->viewManager()->activeView();

    if (kv)
        kv->insertText ("Hello World");
}

Kate::PluginConfigPage* KatePlugin%{APPNAME}::configPage (uint, QWidget *w, const char* name)
{
    %{APPNAME}ConfigPage* p = new %{APPNAME}ConfigPage(this, w);
    initConfigPage( p );
    connect( p, SIGNAL(configPageApplyRequest(%{APPNAME}ConfigPage*)), this, SLOT(slotApplyConfig(%{APPNAME}ConfigPage*)) );
    return (Kate::PluginConfigPage*)p;
}

void KatePlugin%{APPNAME}::initConfigPage( %{APPNAME}ConfigPage* p )
{
    // TODO: initialize %{APPNAME}ConfigPage here
    // NOTE: KatePlugin%{APPNAME} is friend of %{APPNAME}ConfigPage
}

void KatePlugin%{APPNAME}::slotApplyConfig( %{APPNAME}ConfigPage* p )
{
    // TODO: save %{APPNAME}ConfigPage here
    // NOTE: KatePlugin%{APPNAME} is friend of %{APPNAME}ConfigPage
}


/**
 * %{APPNAME}ConfigPage
 */
%{APPNAME}ConfigPage::%{APPNAME}ConfigPage (QObject* parent /*= 0L*/, QWidget *parentWidget /*= 0L*/)
    : Kate::PluginConfigPage( parentWidget )
{
    QVBoxLayout* lo = new QVBoxLayout( this, 0, 0, "config_page_layout" );
    lo->setSpacing(KDialogBase::spacingHint());

    QLabel* lab = new QLabel("KatePlugin%{APPNAME}'s config page", this);

    lo->addWidget(lab);

    // TODO: add connection to emit SLOT( changed() )
}

%{APPNAME}ConfigPage::~%{APPNAME}ConfigPage()
{
}

void %{APPNAME}ConfigPage::apply()
{
    emit configPageApplyRequest( this );
}

#include "plugin_%{APPNAMELC}.moc"

