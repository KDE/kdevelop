#ifndef kdevprojectimpl_h
#define kdevprojectimpl_h

#include "kdevproject.h"

class KDevProjectImpl : public KDevProject
{
    Q_OBJECT

public:
    KDevProjectImpl( QObject *parent, const char *name = 0 );
    ~KDevProjectImpl();

    virtual void openProject(const QString &dirName);
    virtual void closeProject();

    /**
     * Returns the path (relative to the project directory)
     * to main binary program of the project.
     */
    virtual QString mainProgram();
    /**
     * Returns the toplevel directory of the project.
     */
    virtual QString projectDirectory();
    /**
     * Returns a list of all source files.
     * TODO: This is currently a bit broken because the
     * autoproject part doesn't return header files here.
     */
    virtual QStringList allSourceFiles();

};


#endif
