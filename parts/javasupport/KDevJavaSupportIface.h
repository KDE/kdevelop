
#ifndef KDEVJAVASUPPORTIFACE_H
#define KDEVJAVASUPPORTIFACE_H

#include <qobject.h>
#include <dcopobject.h>

class JavaSupportPart;

class KDevJavaSupportIface : public QObject, public DCOPObject
{
    Q_OBJECT
    K_DCOP
public:
    KDevJavaSupportIface( JavaSupportPart* javaSupport );
    ~KDevJavaSupportIface();
    
k_dcop:
    void addClass();
    void parseProject();
    
private:
    JavaSupportPart* m_javaSupport;
};

#endif
