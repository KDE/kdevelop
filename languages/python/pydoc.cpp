#include "pydoc.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <qtextstream.h>
//Added by qt3to4:
#include <Q3CString>
#include <kstandarddirs.h>
#include <kinstance.h>
#include <kprocess.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <klocale.h>

using namespace KIO;


PydocProtocol::PydocProtocol(const Q3CString &pool, const Q3CString &app)
    : SlaveBase("pydoc", pool, app), key()
{
    python = KGlobal::dirs()->findExe("python");
    script = locate("data", "kio_pydoc/kde_pydoc.py");
}


PydocProtocol::~PydocProtocol()
{}


void PydocProtocol::get(const KURL& url)
{
    mimeType("text/html");
    key = url.path();

#if (KDE_VERSION > 305)
    QString cmd = KProcess::quote(python);
    cmd += " ";
    cmd += KProcess::quote(script);
    cmd += " -w ";
    cmd += KProcess::quote(key);
#else
    QString cmd = KShellProcess::quote(python);
    cmd += " ";
    cmd += KShellProcess::quote(script);
    cmd += " -w ";
    cmd += KShellProcess::quote(key);
#endif

    FILE *fd = popen(cmd.local8Bit().data(), "r");
    char buffer[4096];
    QByteArray array;

    while (!feof(fd)) {
        int n = fread(buffer, 1, 2048, fd);
        if (n == -1) {
            pclose(fd);
            return;
        }
        array.setRawData(buffer, n);
        data(array);
        array.resetRawData(buffer, n);
    }

    pclose(fd);
    finished();
}


void PydocProtocol::mimetype(const KURL&)
{
    mimeType( "text/html" );
    finished();
}


Q3CString PydocProtocol::errorMessage()
{
    return Q3CString( "<html><body bgcolor=\"#FFFFFF\">" + i18n("Error in pydoc").local8Bit() + "</body></html>" );
}


void PydocProtocol::stat(const KURL &/*url*/)
{
    UDSAtom uds_atom;
    uds_atom.m_uds = KIO::UDS_FILE_TYPE;
    uds_atom.m_long = S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO;

    UDSEntry uds_entry;
    uds_entry.append(uds_atom);

    statEntry(uds_entry);
    finished();
}


void PydocProtocol::listDir(const KURL &url)
{
    error( KIO::ERR_CANNOT_ENTER_DIRECTORY, url.path() );
}


extern "C" {

    int kdemain(int argc, char **argv)
    {
        KInstance instance( "kio_pydoc" );
        KGlobal::locale()->setMainCatalogue("kdevelop");

        if (argc != 4) {
            fprintf(stderr, "Usage: kio_pydoc protocol domain-socket1 domain-socket2\n");
            exit(-1);
        }

        PydocProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();

        return 0;
    }

}
