#ifndef __JAVASUPPORTPART_H__
#define __JAVASUPPORTPART_H__


#include "kdevlanguagesupport.h"

class JavaSupportPartData;

class JavaSupportPart : public KDevLanguageSupport
{
    Q_OBJECT
public:
    JavaSupportPart(QObject *parent, const char *name, const QStringList &);
    ~JavaSupportPart();

    void parseContents( const QString& contents, const QString& fileName );

protected:
    Features features();
    QStringList fileFilters();
    QStringList fileExtensions();

    void addClass();

private slots:
    void projectOpened();
    void projectClosed();

    void addedFileToProject(const QString &fileName);
    void removedFileFromProject(const QString &fileName);

    void initialParse();
    void savedFile( const QString& );

private:
    void maybeParse(const QString &fileName);
    void parse(const QString &fileName);

private:
    JavaSupportPartData* d;
};


#endif
