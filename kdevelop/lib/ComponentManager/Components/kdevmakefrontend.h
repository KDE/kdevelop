/**
 * The interface to a make frontend
 */

#ifndef _KDEVMAKEFRONTEND_H_
#define _KDEVMAKEFRONTEND_H_

#include <qstringlist.h>
#include "kdevcomponent.h"


class KDevMakeFrontend : public KDevComponent
{
    Q_OBJECT
    
public:
    
    KDevMakeFrontend( QObject *parent=0, const char *name=0 );
    ~KDevMakeFrontend();

    /**
     * The component shall execute a make-like command.
     */
    virtual void executeMakeCommand(const QString &command) = 0;
};

#endif
