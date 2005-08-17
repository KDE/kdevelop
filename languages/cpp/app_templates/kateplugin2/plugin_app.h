//Added by qt3to4:
#include <QPixmap>
#include <Q3PtrList>
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
#include <kate/pluginconfiginterface.h>
#include <kate/pluginconfiginterfaceextension.h>

#include <klibloader.h>
#include <klocale.h>

class %{APPNAME}ConfigPage;

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

class KatePlugin%{APPNAME} : public Kate::Plugin, Kate::PluginViewInterface, Kate::PluginConfigInterfaceExtension
{
    Q_OBJECT

public:
    KatePlugin%{APPNAME}( QObject* parent = 0, const char* name = 0 );
    virtual ~KatePlugin%{APPNAME}();

    void addView (Kate::MainWindow *win);
    void removeView (Kate::MainWindow *win);

    /** overwrite some functions  */
    uint configPages () const { return 1; }
    Kate::PluginConfigPage *configPage (uint , QWidget *w, const char *name=0);
    QString configPageName(uint) const { return i18n("%{APPNAME}"); };
    QString configPageFullName(uint) const { return i18n("Configure KatePlugin%{APPNAME}"); };
    QPixmap configPagePixmap (uint number = 0, int size = KIcon::SizeSmall) const { return 0L; };

public slots:
    void slotInsertHello();
    void slotApplyConfig(%{APPNAME}ConfigPage*);

private:
    void initConfigPage( %{APPNAME}ConfigPage* );

private:
    Q3PtrList<class PluginView> m_views;
};


class %{APPNAME}ConfigPage : public Kate::PluginConfigPage
{
    Q_OBJECT
    friend class KatePlugin%{APPNAME};

public:
    %{APPNAME}ConfigPage (QObject* parent = 0L, QWidget *parentWidget = 0L);
    ~%{APPNAME}ConfigPage ();

    /** Reimplemented from Kate::PluginConfigPage; just emits configPageApplyRequest( this ).  */
    virtual void apply();

    virtual void reset () { ; };
    virtual void defaults () { ; };

signals:
    /** Ask the plugin to set initial values  */
    void configPageApplyRequest( %{APPNAME}ConfigPage* );
    /** Ask the plugin to apply changes  */
    void configPageInitRequest( %{APPNAME}ConfigPage* );

private: // variables

};

#endif // _PLUGIN_%{APPNAMEUC}_H_

