%{CPP_TEMPLATE}

#ifndef %{APPNAME}_H__
#define %{APPNAME}_H__

#include "kscreensaver.h"

#include "%{APPNAMELC}ui.h"

class %{APPNAME} : public KScreenSaver
{
    Q_OBJECT
public:
    %{APPNAME}( WId drawable );
    virtual ~%{APPNAME}();
private:
    void readSettings();
    void blank();
};

class %{APPNAME}Setup : public %{APPNAME}UI
{
    Q_OBJECT
public:
    %{APPNAME}Setup( QWidget *parent = NULL, const char *name = NULL );

private slots:
    void slotOkPressed();
    void slotCancelPressed();

private:
    void readSettings();
    %{APPNAME} *saver;
};

#endif
