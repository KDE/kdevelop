/**
 * The interface to a app frontend
 */

#ifndef _KDEVAPPFRONTEND_H_
#define _KDEVAPPFRONTEND_H_

#include <qstringlist.h>
#include "kdevcomponent.h"


class KDevAppFrontend : public KDevComponent
{
    Q_OBJECT
    
public:
    
    KDevAppFrontend( QObject *parent=0, const char *name=0 );
    ~KDevAppFrontend();

    /**
     * The component shall execute a app-like command.
     */
    virtual void executeAppCommand(const QString &command) = 0;
};

#endif
