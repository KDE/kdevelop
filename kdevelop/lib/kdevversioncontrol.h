/**
 * This is the abstract base class which encapsulates everything
 * necessary for communicating with version control systems.
 */

#ifndef _KDEVVERSIONCONTROL_H_
#define _KDEVVERSIONCONTROL_H_

#include <qstringlist.h>
#include "kdevcomponent.h"


class KDevVersionControl : public KDevComponent
{
public:
    enum State { canBeCommited = 1, canBeAdded = 2 };
    
    KDevVersionControl( QObject *parent=0, const char *name=0 );
    ~KDevVersionControl();

    /**
     * Adds a file to the repository.
     */
    virtual void add(const QString &filename) = 0;
    /**
     * Removes a file from the repository.
     */
    virtual void remove(const QString &filename) = 0;
    /**
     * Updates a file or directory.
     */
    virtual void update(const QString &filename) = 0;
    /**
     * Commits a file to the repository.
     */
    virtual void commit(const QString &filename) = 0;
    /**
     * Tells whether the given file is registered
     * in the version control system.
     */
    virtual State registeredState(const QString &filename) = 0;
};

#endif
