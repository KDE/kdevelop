#ifndef _KIO_KDEAPI_H_
#define _KIO_KDEAPI_H_

#include <qobject.h>
#include <kio/slavebase.h>


class KdeapiProtocol : public KIO::SlaveBase
{
public:
    KdeapiProtocol(const QCString &pool, const QCString &app);
    virtual ~KdeapiProtocol();

    virtual void get(const KURL &url);
    virtual void stat(const KURL &url);
    virtual void mimetype(const KURL &url);
    virtual void listDir(const KURL &url);

private:
    QString qtdocdir;
    QString kdedocdir;
};

#endif
