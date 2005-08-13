%{H_TEMPLATE}

#ifndef __%{APPNAMEUC}_CONFIGWIDGET_H__
#define __%{APPNAMEUC}_CONFIGWIDGET_H__

#include <kbear/kbearconfigwidgetiface.h>
#include <qstring.h>

class KBear%{APPNAME}ConfigWidget : public KBear::KBearConfigWidgetIface
{
    Q_OBJECT
public:
    KBear%{APPNAME}ConfigWidget( QWidget* parent, const char* name  );
    virtual ~KBear%{APPNAME}ConfigWidget();

    /**
    * Reimplement from @ref KBear::KBearConfigWidgetIface
    */
    virtual bool helpEnabled() const { return false; }
    /**
    * Reimplement from @ref KBear::KBearConfigWidgetIface
    */
    virtual void saveSettings();
    /**
    * Reimplement from @ref KBear::KBearConfigWidgetIface
    */
    virtual void readSettings( bool defaultSettings = false );
public slots:
    void slotSaveSettings();
};

#endif // __%{APPNAMEUC}_CONFIGWIDGET_H__

