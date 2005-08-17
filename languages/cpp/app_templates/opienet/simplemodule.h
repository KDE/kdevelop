#ifndef %{APPNAME}_NETWORK_MODULE_H
#define %{APPNAME}_NETWORK_MODULE_H

#include <module.h>
//Added by qt3to4:
#include <Q3CString>

class %{APPNAME}Module : Module {

signals:
    void updateInterface(Interface* i );

public:
    %{APPNAME}Module();
    ~%{APPNAME}Module();

    const QString type() {return QString::fromLatin1("vpn" );}
    void setProfile( const QString& ) {}
    bool isOwner( Interface* );
    QWidget *configure( Interface* );
    QWidget *information(  Interface* );
    QList<Interface> getInterfaces();
    void possibleNewInterfaces( QMap<QString, QString>& );
    Interface *addNewInterface( const QString& );
    bool remove( Interface* iface );
    QString getPixmapName( Interface*  ) {return QString::fromLatin1("Tux"); }
    void receive( const Q3CString&, const QByteArray& ar ) {} // don't listen
private:
    QList<Interface> m_interfaces;
};

extern "C" {
    void* create_plugin() {
        return new %{APPNAME}Module();
    }
};

#endif
