/**
 * This is the abstract base class which encapsulates everything
 * necessary for communicating with version control systems.
 * Ideally, a new system could be integrated by subclassing this
 * class, implementing it in a dynamic library and letting it
 * register in KDevelop. But as we don't live in an ideal world,
 * the list of supported systems is hardcoded and available
 * by calling VersionControl::getSupportedSystems().
 */

#ifndef _VERSIONCONTROL_H_
#define _VERSIONCONTROL_H_

#include <qstrlist.h>


class VersionControl
{
public:
    /**
     * Adds to list the names of all supported
     * version control systems.
     */
    static void getSupportedSystems(QStrList *list);
    /**
     * Factory method for a new version control object for the
     * system given by the parameter system.
     * All communication with vc system should then be
     * done via the returned object.
     */
    static VersionControl *getVersionControl(const char *name);
    /**
     * Adds a file to the repository.
     * The message shown in the dialog box is text.
     */
    virtual void add(const char *filename, const char *text) = 0;
    /**
     * Removes a file from the repository.
     * The message shown in the dialog box is text.
     */
    virtual void remove(const char *filename, const char *text) = 0;
    /**
     * Tells whether the given file is registered
     * in the version control system.
     */
    virtual bool isRegistered(const char *filename) = 0;
};

#endif
