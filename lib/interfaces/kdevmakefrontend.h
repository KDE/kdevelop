/**
 * The interface to a make frontend
 */

#ifndef _KDEVMAKEFRONTEND_H_
#define _KDEVMAKEFRONTEND_H_

#include <qstringlist.h>
#include "kdevplugin.h"


class KDevMakeFrontend : public KDevPlugin
{
    Q_OBJECT

public:

    KDevMakeFrontend( const QString& pluginName, const QString& icon, QObject *parent=0, const char *name=0 );
    ~KDevMakeFrontend();

    virtual QWidget* widget() { return 0L; }

    /**
     * The component shall start to execute a make-like command.
     * Commands are always asynchronous. You can submit several jobs
     * without caring about another job already running. There are
     * executed in the order in which they are submitted. If one of
     * then fails, all following jobs are dropped.
     * You should not make any assumptions about the directory in which
     * the command is started. If the command depends on that, put and
     * explicit 'cd' into the command. The parameter dir is interpreted
     * as a starting directory to find files when parsing compiler error
     * messages.
     */
    virtual void queueCommand(const QString &dir, const QString &command) = 0;
    /**
     * Returns whether the application is currently running.
     */
    virtual bool isRunning() = 0;
    /**
     * Advices to synchronize the settings from KConfig because they've changed externally.
     */
    virtual void updateSettingsFromConfig() = 0;
signals:
    /**
     * Only emitted if the command was succesfully finished.
     */
    void commandFinished(const QString &command);
    /**
     * Emitted if a command failed.
     */
    void commandFailed(const QString &command);
};

#endif
