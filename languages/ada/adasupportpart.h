//
#ifndef __ADASUPPORTPART_H__
#define __ADASUPPORTPART_H__

#include "kdevlanguagesupport.h"

#include <qwaitcondition.h>

class AdaSupportPartData;
class KDialogBase;
class QStringList;

class AdaSupportPart : public KDevLanguageSupport
{
    Q_OBJECT
public:
    AdaSupportPart (QObject *parent, const char *name, const QStringList &);
    ~AdaSupportPart ();

    void parseContents (const QString& contents, const QString& fileName);

protected:
    Features features ();
    KMimeType::List mimeTypes ();
    QStringList fileExtensions ();

private slots:
    void projectOpened ();
    void projectClosed ();

    void addedFilesToProject (const QStringList &fileList);
    void removedFilesFromProject (const QStringList &fileList);
    void changedFilesInProject( const QStringList &fileList );

    void initialParse ();
    void savedFile (const KURL&);

private:
    void maybeParse (const QString &fileName);
    void parse (const QString &fileName);

    void saveProjectSourceInfo( );
private:
    AdaSupportPartData* d;
    
friend class ProblemReporter;
};


#endif
