#ifndef _KDEVCOREIFACE_H_
#define _KDEVCOREIFACE_H_

#include <dcopobject.h>
#include <dcopref.h>

class KDevCore;


class KDevCoreIface : public QObject, public DCOPObject
{
    Q_OBJECT
    K_DCOP
    
public:
    
    KDevCoreIface( KDevCore *core );
    ~KDevCoreIface();

k_dcop:
    /**
     * embed=0: Replace, =1: SplitHorizontal, =2: SplitVertical
     */
    void gotoDocumentationFile(const QString &url, int embed);
    void gotoSourceFile(const QString &fileName, int lineNum, int embed);

private slots:
    void forwardProjectOpened();
    void forwardProjectClosed();
    
private:
    KDevCore *m_core;
};

#endif
