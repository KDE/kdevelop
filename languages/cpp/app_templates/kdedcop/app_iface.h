%{H_TEMPLATE}

#include <dcopobject.h>

class DCOPDemoIface : virtual public DCOPObject
{
    K_DCOP
    k_dcop:


    // Here you should place your DCOP interface.
    // Below are four example methods.
    virtual QString strVal() const = 0;
    virtual int intVal() const = 0;

    virtual void setIntVal( int num ) = 0;
    virtual void setStrVal( const QString &str ) = 0;

};
