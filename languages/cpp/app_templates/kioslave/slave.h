
#ifndef _%{APPNAMELC}_H_
#define _%{APPNAMELC}_H_

#include <qstring.h>
#include <qcstring.h>

#include <kurl.h>
#include <kio/global.h>
#include <kio/slavebase.h>

class QCString;

class kio_%{APPNAMELC}Protocol : public KIO::SlaveBase
{
public:
    kio_%{APPNAMELC}Protocol(const QCString &pool_socket, const QCString &app_socket);
    virtual ~kio_%{APPNAMELC}Protocol();
    virtual void mimetype(const KURL& url);
    virtual void get(const KURL& url);
};

#endif
