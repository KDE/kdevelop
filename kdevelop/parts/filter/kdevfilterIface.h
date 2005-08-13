
#ifndef KDEVFILTERIFACE_H
#define KDEVFILTERIFACE_H

#include <qobject.h>
#include <dcopobject.h>

class FilterPart;

class KDevFilterIface : public QObject, public DCOPObject
{
    Q_OBJECT
    K_DCOP
public:
    KDevFilterIface( FilterPart* part );
    ~KDevFilterIface();
    
k_dcop:
    void shellInsert();
    void shellFilter();
    
private:
    FilterPart* m_part;
};

#endif
