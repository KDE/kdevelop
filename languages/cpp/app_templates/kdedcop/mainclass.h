%{H_TEMPLATE}

#ifndef MAINCLASS_H
#define MAINCLASS_H

#include "%{APPNAMELC}_iface.h"

/**
 *
 * @author %{AUTHOR}
 **/
class MainClass : virtual public DCOPDemoIface
{
public:
    MainClass();

    ~MainClass();


    // Here is the implementation of the example DCOP interface methods.
    virtual void setStrVal(const QString &);
    virtual void setIntVal(int);
    virtual QString strVal() const;
    virtual int intVal() const;

private:
    QString m_strValue;
    int m_intValue;

};

#endif
