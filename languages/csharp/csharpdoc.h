#ifndef _CSHARPDOC_H_
#define _CSHARPDOC_H_

#include <qobject.h>
#include <kio/slavebase.h>


class CSharpdocProtocol : public KIO::SlaveBase
{
public:
    CSharpdocProtocol(const QCString &pool, const QCString &app);
    virtual ~CSharpdocProtocol();

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
