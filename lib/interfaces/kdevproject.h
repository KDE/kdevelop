#ifndef _KDEVPROJECT_H_
#define _KDEVPROJECT_H_

#include <qstringlist.h>
#include "kdevpart.h"


class KDevProject : public KDevPart
{
    Q_OBJECT
    
public:
    KDevProject( KDevApi *api, QObject *parent=0, const char *name=0 );
    ~KDevProject();

    virtual void openProject(const QString &dirName) = 0;
    virtual void closeProject() = 0;

    virtual QString projectDirectory() = 0;
    virtual QStringList allSourceFiles() = 0;

signals:
    void addedFileToProject(const QString &fileName);
    void removedFileFromProject(const QString &fileName);
};

#endif
