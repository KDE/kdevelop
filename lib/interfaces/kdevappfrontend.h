/**
 * The interface to a app frontend
 */

#ifndef _KDEVAPPFRONTEND_H_
#define _KDEVAPPFRONTEND_H_

#include <qstringlist.h>
#include "kdevplugin.h"


class KDevAppFrontend : public KDevPlugin
{
    Q_OBJECT
    
public:
    
    KDevAppFrontend( QObject *parent=0, const char *name=0 );
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
    /**
     * Inserts a string into the view.
     */
    virtual void insertStdoutLine(const QString &line) = 0;
    /**
     * Inserts a string into the view marked as stderr output
     * (colored in the current implementation).
     */
    virtual void insertStderrLine(const QString &line) = 0;
};

#endif
