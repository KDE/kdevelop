#ifndef _DBGMANAGER_H_
#define _DBGMANAGER_H_

#include "kdevcomponent.h"

class DbgManager : public KDevComponent
{
    Q_OBJECT

public:
    DbgManager( QObject *parent=0, const char *name=0 );
    virtual ~DbgManager();

protected:
    virtual void setupGUI();
    virtual void compilationAborted();
    virtual void projectClosed();
    virtual void projectOpened(CProject *prj);
};

#endif
