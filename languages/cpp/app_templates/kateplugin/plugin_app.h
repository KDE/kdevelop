%{H_TEMPLATE}

#ifndef _PLUGIN_%{APPNAMEUC}_H_
#define _PLUGIN_%{APPNAMEUC}_H_

#include <kate/application.h>
#include <kate/documentmanager.h>
#include <kate/document.h>
#include <kate/mainwindow.h>
#include <kate/plugin.h>
#include <kate/view.h>
#include <kate/viewmanager.h>

#include <klibloader.h>
#include <klocale.h>

class KatePluginFactory : public KLibFactory
{
  Q_OBJECT

  public:
    KatePluginFactory();
    virtual ~KatePluginFactory();

    virtual QObject* createObject( QObject* parent = 0, const char* pname = 0, const char* name = "QObject", const QStringList &args = QStringList() );

  private:
    static KInstance* s_instance;
};

class KatePlugin%{APPNAME} : public Kate::Plugin, Kate::PluginViewInterface
{
  Q_OBJECT

  public:
    KatePlugin%{APPNAME}( QObject* parent = 0, const char* name = 0 );
    virtual ~KatePlugin%{APPNAME}();

    void addView (Kate::MainWindow *win);
    void removeView (Kate::MainWindow *win);

  public slots:
    void slotInsertHello();

  private:
    QPtrList<class PluginView> m_views;
};

#endif // _PLUGIN_%{APPNAMEUC}_H_
