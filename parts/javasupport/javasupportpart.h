#ifndef __JAVASUPPORTPART_H__
#define __JAVASUPPORTPART_H__


#include "kdevlanguagesupport.h"

class JavaSupportPartData;
class KDialogBase;
class QStringList;

class JavaSupportPart : public KDevLanguageSupport
{
    Q_OBJECT
public:
    JavaSupportPart(QObject *parent, const char *name, const QStringList &);
    ~JavaSupportPart();

    void parseContents( const QString& contents, const QString& fileName );

protected:
    Features features();
    KMimeType::List mimeTypes();
    QStringList fileExtensions();

    void addClass();

private slots:
    void projectOpened();
    void projectClosed();

    void addedFilesToProject(const QStringList &fileList);
    void removedFilesFromProject(const QStringList &fileList);

    void initialParse();
    void savedFile( const QString& );

private:
    void maybeParse(const QString &fileName);
    void parse(const QString &fileName);

private:
    JavaSupportPartData* d;
};


#endif
