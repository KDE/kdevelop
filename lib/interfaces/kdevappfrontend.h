/**
 * The interface to a app frontend
 */

#ifndef _KDEVAPPFRONTEND_H_
#define _KDEVAPPFRONTEND_H_

#include <qstringlist.h>
#include "kdevpart.h"


class KDevAppFrontend : public KDevPart
{
    Q_OBJECT
    
public:
    
    KDevAppFrontend( KDevApi *api, QObject *parent=0, const char *name=0 );
    ~KDevAppFrontend();

    /**
     * The component shall start to execute an app-like command.
     * Running the application is always asynchronous.
     */
    virtual void startAppCommand(const QString &command) = 0;
    /**
     * Returns whether the application is currently running.
     */
    virtual bool isRunning() = 0;
};

#endif
