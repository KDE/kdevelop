#ifndef _PYDOC_H_
#define _PYDOC_H_

#include <qobject.h>
#include <kio/slavebase.h>


class PydocProtocol : public KIO::SlaveBase
{
public:
    PydocProtocol(const QCString &pool, const QCString &app);
    virtual ~PydocProtocol();

    virtual void get(const KURL& url);
    virtual void stat(const KURL& url);
    virtual void mimetype(const KURL& url);
    virtual void listDir(const KURL& url);

protected:
    void decodeURL(const KURL &url);
    void decodePath(QString path);
    QCString errorMessage();

private:
    QString python;
    QString script;
    QString key;
};

#endif
