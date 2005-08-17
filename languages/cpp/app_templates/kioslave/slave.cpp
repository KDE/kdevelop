
#include <q3cstring.h>
#include <q3socket.h>
#include <qdatetime.h>
#include <qbitarray.h>

#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kurl.h>
#include <ksock.h>

#include "%{APPNAMELC}.h"

using namespace KIO;


kio_%{APPNAMELC}Protocol::kio_%{APPNAMELC}Protocol(const Q3CString &pool_socket, const Q3CString &app_socket)
    : SlaveBase("kio_%{APPNAMELC}", pool_socket, app_socket)
{
    kdDebug() << "kio_%{APPNAMELC}Protocol::kio_%{APPNAMELC}Protocol()" << endl;
}


kio_%{APPNAMELC}Protocol::~kio_%{APPNAMELC}Protocol()
{
    kdDebug() << "kio_%{APPNAMELC}Protocol::~kio_%{APPNAMELC}Protocol()" << endl;
}


void kio_%{APPNAMELC}Protocol::get(const KURL& url )
{
    kdDebug() << "kio_%{APPNAMELC}::get(const KURL& url)" << endl ;
    
    kdDebug() << "Seconds: " << url.query() << endl;
    QString remoteServer = url.host();
    int remotePort = url.port();
    kdDebug() << "myURL: " << url.prettyURL() << endl;
    
    infoMessage(i18n("Looking for %1...").arg( remoteServer ) );
    // Send the mimeType as soon as it is known
    mimeType("text/plain");
    // Send the data
    QString theData = "This is a test of kio_%{APPNAMELC}";
    data(Q3CString(theData.local8Bit()));
    data(QByteArray()); // empty array means we're done sending the data
    finished();
}


void kio_%{APPNAMELC}Protocol::mimetype(const KURL & /*url*/)
{
    mimeType("text/plain");
    finished();
}


extern "C"
{
    int kdemain(int argc, char **argv)
    {
        KInstance instance( "kio_%{APPNAMELC}" );
        
        kdDebug(7101) << "*** Starting kio_%{APPNAMELC} " << endl;
        
        if (argc != 4) {
            kdDebug(7101) << "Usage: kio_%{APPNAMELC}  protocol domain-socket1 domain-socket2" << endl;
            exit(-1);
        }
        
        kio_%{APPNAMELC}Protocol slave(argv[2], argv[3]);
        slave.dispatchLoop();
        
        kdDebug(7101) << "*** kio_%{APPNAMELC} Done" << endl;
        return 0;
    }
} 
