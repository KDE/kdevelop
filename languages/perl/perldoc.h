#ifndef _PERLDOC_H_
#define _PERLDOC_H_

#include <qobject.h>
#include <kio/slavebase.h>


class PerldocProtocol : public KIO::SlaveBase
{
public:
    PerldocProtocol(const QCString &pool, const QCString &app);
    virtual ~PerldocProtocol();

    virtual void get(const KURL& url);
    virtual void stat(const KURL& url);
    virtual void mimetype(const KURL& url);
    virtual void listDir(const KURL& url);

protected:
    void decodeURL(const KURL &url);
    void decodePath(QString path);
    QCString errorMessage();
};

#endif
