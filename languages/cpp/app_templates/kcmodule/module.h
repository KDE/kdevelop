
#ifndef _%{APPNAMEUC}_H_
#define _%{APPNAMEUC}_H_

#include <kcmodule.h>
#include <kaboutdata.h>

class %{APPNAME}: public KCModule
{
    Q_OBJECT

public:
    %{APPNAME}( QWidget *parent=0, const char *name=0, const QStringList& = QStringList() );
    ~%{APPNAME}();

    virtual void load();
    virtual void save();
    virtual void defaults();
    virtual int buttons();
    virtual QString quickHelp() const;
    virtual const KAboutData *aboutData()const
    { return myAboutData; };

public slots:
    void configChanged();

private:
      KAboutData *myAboutData;
};

#endif
