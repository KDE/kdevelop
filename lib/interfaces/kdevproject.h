#ifndef _KDEVPROJECT_H_
#define _KDEVPROJECT_H_

#include <qstringlist.h>
#include "kdevplugin.h"


class KDevProject : public KDevPlugin
{
    Q_OBJECT
    
public:
    KDevProject( QObject *parent=0, const char *name=0 );
    ~KDevProject();

    /**
     * This method is invoked when the project is opened
     * (i.e. actually just after this class has been
     * instantiated). The first parameter is the project
     * directory, which should afterwards be returned by
     * the projectDirectory() method. The second parameter
     * is the project name, which is equivalent with the
     * project file name without the .kdevelop suffix.
     */
    virtual void openProject(const QString &dirName,
                             const QString &projectName) = 0;
    /**
     * This method is invoked when the project is about
     * to be closed.
     */
    virtual void closeProject() = 0;

    /**
     * Returns the path (relative to the project directory)
     * to main binary program of the project.
     */
    virtual QString mainProgram() = 0;
    /**
     * Returns the toplevel directory of the project.
     */
    virtual QString projectDirectory() = 0;
    /**
     * Returns the name of the project.
     */
    virtual QString projectName() = 0;
    /**
     * Returns the path (relative to the project directory)
     * of the active directory. All newly generated classes
     * are added here.
     */
    virtual QString activeDirectory() = 0;
    /**
     * Returns a list of all source files.
     * TODO: This is currently a bit broken because the
     * autoproject part doesn't return header files here.
     */
    virtual QStringList allSourceFiles() = 0;
    /**
     * Adds a file to the project. The given file name
     * must be relative to the project directory.
     */
    virtual void addFile(const QString &fileName) = 0;
    /**
     * Removes a file from the project. The given file name
     * must be relative to the project directory.
     */
    virtual void removeFile(const QString &fileName) = 0;
    
signals:
    /**
     * Emitted when a new file has been added to the
     * project. The fileName is relative to the project directory.
     */
    void addedFileToProject(const QString &fileName);
    /**
     * Emitted when a file has been removed from the
     * project. The fileName is relative to the project directory.
     */
    void removedFileFromProject(const QString &fileName);
};

#endif
