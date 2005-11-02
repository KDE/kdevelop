%{CPP_TEMPLATE}

#include "plugin_%{APPNAMELC}.h"

#include <kaction.h>
#include <klocale.h>
#include <kstandarddirs.h>

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

#include "plugin_%{APPNAMELC}.moc"

