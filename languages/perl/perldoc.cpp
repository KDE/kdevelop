#include "perldoc.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <qtextstream.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kinstance.h>
#include <kprocess.h>
#include <kdeversion.h>
#include <kglobal.h>

using namespace KIO;


PerldocProtocol::PerldocProtocol(const QCString &pool, const QCString &app)
    : SlaveBase("perldoc", pool, app)
{}


PerldocProtocol::~PerldocProtocol()
{}


void PerldocProtocol::get(const KURL& url)
{
    QStringList l = QStringList::split('/', url.path());

    mimeType("text/html");

    bool plain = false;
    QString cmd = "perldoc ";
    if (l[0] == "functions") {
        plain = true;
        cmd += "-t -f ";
        cmd += KProcess::quote(l[1]);
    } else if (l[0] == "faq") {
        cmd += "-u -q ";
        cmd += KProcess::quote(l[1]);
        cmd += " | pod2html";
    } else {
        QCString errstr(i18n("The only existing directories are functions and faq.").local8Bit());
        data(errstr);
        finished();
        return;
    }

    kdDebug() << "Command: " << cmd << endl;

    if (plain)
        data(QCString("<blockquote>"));

    FILE *fd = popen(cmd.local8Bit().data(), "r");
    char buffer[4090];
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

    if (plain)
        data(QCString("</blockquote>"));

    finished();
}


void PerldocProtocol::mimetype(const KURL &url)
{
    QStringList l = QStringList::split('/', url.path());
    mimeType((l[0] == "faq")? "text/html" : "text/plain");
    finished();
}


QCString PerldocProtocol::errorMessage()
{
    return QCString( "<html><body bgcolor=\"#FFFFFF\">" + i18n("Error in perldoc").local8Bit() + "</body></html>" );
}


void PerldocProtocol::stat(const KURL &/*url*/)
{
    UDSAtom uds_atom;
    uds_atom.m_uds = KIO::UDS_FILE_TYPE;
    uds_atom.m_long = S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO;

    UDSEntry uds_entry;
    uds_entry.append(uds_atom);

    statEntry(uds_entry);
    finished();
}


void PerldocProtocol::listDir(const KURL &url)
{
    error( KIO::ERR_CANNOT_ENTER_DIRECTORY, url.path() );
}


extern "C" {

    int kdemain(int argc, char **argv)
    {
        KInstance instance( "kio_perldoc" );
        KGlobal::locale()->setMainCatalogue("kdevelop");

        if (argc != 4) {
            fprintf(stderr, "Usage: kio_perldoc protocol domain-socket1 domain-socket2\n");
            exit(-1);
        }

        PerldocProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();

        return 0;
    }

}
