/**
 * The interface to a make frontend
 */

#ifndef _KDEVMAKEFRONTEND_H_
#define _KDEVMAKEFRONTEND_H_

#include <qstringlist.h>
#include "kdevpart.h"


class KDevMakeFrontend : public KDevPart
{
    Q_OBJECT
    
public:
    
    KDevMakeFrontend( KDevApi *api, QObject *parent=0, const char *name=0 );
    ~KDevMakeFrontend();

    /**
     * The component shall start to execute a make-like command.
     * Commands are always asynchronous.
     */
    virtual void startMakeCommand(const QString &dir, const QString &command) = 0;
    /**
     * Returns whether the application is currently running.
     */
    virtual bool isRunning() = 0;
};

#endif
