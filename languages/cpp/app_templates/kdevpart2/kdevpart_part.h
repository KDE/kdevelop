%{H_TEMPLATE}
#ifndef KDEV%{APPNAMEUC}_H
#define KDEV%{APPNAMEUC}_H

#include <kdevplugin.h>

#include <qguardedptr.h>

class QPopupMenu;
class KAction;
class KDialogBase;
class Context;
class ConfigWidgetProxy;
class %{APPNAME}Widget;

/**
Please read the README.dox file for more info about this part
*/
class %{APPNAME}Part: public KDevPlugin
{
    Q_OBJECT
public:
    %{APPNAME}Part(QObject *parent, const char *name, const QStringList &args);
    ~%{APPNAME}Part();
  
private slots:
    void init();
    
    void insertConfigWidget(const KDialogBase *dlg, QWidget *page, unsigned int pageNo);
    void contextMenu(QPopupMenu *popup, const Context *context);
    void projectOpened();
    void projectClosed();
    
    void doSomething();

private:
    void setupActions();
    
    KAction *action;
    
    QPointer<%{APPNAME}Widget> m_widget;
    ConfigWidgetProxy *m_configProxy;
};

#endif
