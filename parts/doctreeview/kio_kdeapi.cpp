#include "kio_kdeapi.h"

#include "../../config.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <qfile.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kinstance.h>

using namespace KIO;


KdeapiProtocol::KdeapiProtocol(const QCString &pool, const QCString &app)
    : SlaveBase("pydoc", pool, app)
{
    KConfig *config = KGlobal::config();
    config->setGroup("General");
    
    qtdocdir = config->readEntry("qtdocdir", QT_DOCDIR);
    kdedocdir = config->readEntry("kdelibsdocdir", KDELIBS_DOXYDIR);
}


KdeapiProtocol::~KdeapiProtocol()
{}


void KdeapiProtocol::get(const KURL &url)
{
    mimeType("text/html");
    QString path = url.path();
    int pos = path.find('/');
    if (pos == -1) {
        error(KIO::ERR_DOES_NOT_EXIST, path);
        return;
    }

    QString dirName = path.left(pos);
    QString className = path.mid(pos+1);
    kdDebug() << "path: " << path << ", dirName: " << dirName << ", className: " << className << endl;

    QString fileName;
    
    if (dirName == "qt") {
        if (className == "index.html")
            fileName = qtdocdir + "/index.html";
        else
            fileName = qtdocdir + "/" + className.lower() + ".html";
    } else {
        if (className == "index.html")
            fileName = kdedocdir + "/" + dirName + "/html/index.html";
        else {
            className.replace(QRegExp("::"), "_1_1");
            fileName = kdedocdir + "/" + dirName + "/html/class" + className + ".html";
            if (!QFile::exists(fileName))
                fileName = kdedocdir + "/" + dirName + "/html/struct" + className + ".html";
        }
    }

    kdDebug() << "Trying filename " << fileName << endl;
    
    int fd = ::open(QFile::encodeName(fileName), O_RDONLY);
    char buffer[4096];
    QByteArray array;
    
    for ( ;; ) {
        int n = read(fd, buffer, sizeof buffer);
        if (n == -1) {
            error(KIO::ERR_COULD_NOT_READ, fileName);
            ::close(fd);
            return;
        } else if (n == 0) {
            break;
        }
        
        array.setRawData(buffer, n);
        data(array);
        array.resetRawData(buffer, n);
    }
    
    ::close(fd);
    finished();
}


void KdeapiProtocol::mimetype(const KURL&)
{
    mimeType("text/html");
    finished();
}


void KdeapiProtocol::stat(const KURL &/*url*/)
{
    UDSAtom uds_atom;
    uds_atom.m_uds = KIO::UDS_FILE_TYPE;
    uds_atom.m_long = S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO;
    
    UDSEntry uds_entry;
    uds_entry.append(uds_atom);
    
    statEntry(uds_entry);
    finished();
}


void KdeapiProtocol::listDir(const KURL &url)
{
    error(KIO::ERR_CANNOT_ENTER_DIRECTORY, url.path());
}


extern "C" {

    int kdemain(int argc, char **argv)
    {
        KInstance instance( "kio_kdeapi" );
        
        if (argc != 4) {
            fprintf(stderr, "Usage: kio_kdeapi protocol domain-socket1 domain-socket2\n");
            exit(-1);
        }
        
        KdeapiProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();
        
        return 0;
    }

}
