#ifndef _PERLDOC_H_
#define _PERLDOC_H_

#include <qobject.h>
//Added by qt3to4:
#include <Q3CString>
#include <kio/slavebase.h>


class PerldocProtocol : public KIO::SlaveBase
{
public:
    PerldocProtocol(const Q3CString &pool, const Q3CString &app);
    virtual ~PerldocProtocol();

    virtual void get(const KURL& url);
    virtual void stat(const KURL& url);
    virtual void mimetype(const KURL& url);
    virtual void listDir(const KURL& url);

protected:
    void decodeURL(const KURL &url);
    void decodePath(QString path);
    Q3CString errorMessage();
};

#endif
