/**
 * This is the abstract base class which encapsulates everything
 * necessary for communicating with version control systems.
 */

#ifndef _KDEVVERSIONCONTROL_H_
#define _KDEVVERSIONCONTROL_H_

#include <qstringlist.h>
#include "kdevplugin.h"


class KDevVersionControl : public KDevPlugin
{
    Q_OBJECT

public:
    enum State { canBeCommited = 1, canBeAdded = 2 };

    KDevVersionControl( const QString& pluginName, const QString& icon, QObject *parent, const char *name=0 );
    ~KDevVersionControl();

    /**
     * Tells whether the given file is registered
     * in the version control system.
     */
     //    This will be replaced by a smarter solution
     //    virtual State registeredState(const QString &fileName) = 0;
};

#endif
