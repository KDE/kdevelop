 
#ifndef __KBEARPLUGIN_%{APPNAMEUC}_H__
#define __KBEARPLUGIN_%{APPNAMEUC}_H__

#include <kbear/kbearplugin.h>

class KDialogBase;
class KWizard;
class KBear%{APPNAME}Widget;

class KBear%{APPNAME}Plugin : public KBear::KBearPlugin {
    Q_OBJECT
public:
    KBear%{APPNAME}Plugin(QObject* parent, const char* name, const QStringList& args = QStringList());
    virtual ~KBear%{APPNAME}Plugin();

    virtual QString unloadWarning() const;
public slots:
    virtual void slotInit();
protected slots:
    virtual void slotConfigWidget( KDialogBase* dlg );
    virtual void slotConfigWidget( KWizard* wiz );
private:
    KBear%{APPNAME}OutputWidget* m_outputWidget;
};

#endif // __KBEARPLUGIN_%{APPNAMEUC}_H__

