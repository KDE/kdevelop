#ifndef EXAMPLE_IFACE_VPN_H
#define EXAMPLE_IFACE_VPN_H

#include <interfaces/interface.h>

class %{APPNAME}Interface : public Interface {
    Q_OBJECT

public:
    %{APPNAME}Interface(QObject* parent, const char* name = "vpn", bool up = false );
    ~%{APPNAME}Interface();

public slots:
    bool refresh(); // refresh information
    void start();
    void stop();

private:
    bool m_isUp : 1;
};

#endif
