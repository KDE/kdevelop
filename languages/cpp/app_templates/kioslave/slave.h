
#ifndef _%{APPNAMELC}_H_
#define _%{APPNAMELC}_H_

#include <qstring.h>
#include <q3cstring.h>

#include <kurl.h>
#include <kio/global.h>
#include <kio/slavebase.h>

class Q3CString;

class kio_%{APPNAMELC}Protocol : public KIO::SlaveBase
{
public:
    kio_%{APPNAMELC}Protocol(const Q3CString &pool_socket, const Q3CString &app_socket);
    virtual ~kio_%{APPNAMELC}Protocol();
    virtual void mimetype(const KURL& url);
    virtual void get(const KURL& url);
};

#endif
