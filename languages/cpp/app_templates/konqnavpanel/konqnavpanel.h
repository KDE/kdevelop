
#ifndef %{APPNAMEUC}_H
#define %{APPNAMEUC}_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <konqsidebarplugin.h>
#include <qstring.h>
//Added by qt3to4:
#include <QLabel>
#include <kconfig.h>


class %{APPNAME} : public KonqSidebarPlugin
{
    Q_OBJECT
    
public:
    /**
     * Construct a @ref KonqSidebarPlugin.
     *
     * @param inst The sidebar's kinstance class.
     * @param parent The sidebar internal button info class responsible for this plugin.
     * @param widgetParent The container which will contain the plugins widget.
     * @param desktopName The filename of the configuration file.
     * @param name A Qt object name for your plugin.
     **/
     %{APPNAME}(KInstance *inst,QObject *parent,QWidget *widgetParent, QString &desktopName, const char* name=0);

    /** destructor */
    ~%{APPNAME}();
    
   virtual QWidget *getWidget(){return widget;}
   virtual void *provides(const QString &) {return 0;}

protected:
	/*Example widget only. You use whichever widget you want. You onl have to care that getWidget returns some kind of widget */
	QLabel *widget;
        virtual void handleURL(const KURL &url);

};

#endif
