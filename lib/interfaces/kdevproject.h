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

    virtual void openProject(const QString &dirName) = 0;
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
     * Returns a list of all source files.
     * TODO: This is currently a bit broken because the
     * autoproject part doesn't return header files here.
     */
    virtual QStringList allSourceFiles() = 0;

	void setProjectName ( const QString& name ) { m_projectName = name; };
	QString projectName() { return m_projectName; };

signals:
    void addedFileToProject(const QString &fileName);
    void removedFileFromProject(const QString &fileName);

private:
	QString m_projectName;
};

#endif
