/*  This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qcstring.h>
#include <qbitarray.h>
#include <qfile.h>
#include <qregexp.h>

#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kurl.h>
#include <kmimemagic.h>
#include <ktempfile.h>

#include "chm.h"


using namespace KIO;

extern "C"
{
    int kdemain( int argc, char **argv )
    {
        KInstance instance( "kio_chm" );

        kdDebug() << "*** Starting kio_chm " << endl;

        if (argc != 4) {
            kdDebug() << "Usage: kio_chm  protocol domain-socket1 domain-socket2" << endl;
            exit(-1);
        }

        ChmProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();

        kdDebug() << "*** kio_chm Done" << endl;
        return 0;
    }
}

ChmProtocol::ChmProtocol(const QCString &pool_socket, const QCString &app_socket)
: SlaveBase("kio_chm", pool_socket, app_socket)
{
    kdDebug() << "ChmProtocol::ChmProtocol()" << endl;
}
/* ---------------------------------------------------------------------------------- */


ChmProtocol::~ChmProtocol()
{
    kdDebug() << "ChmProtocol::~ChmProtocol()" << endl;
}


/* ---------------------------------------------------------------------------------- */
void ChmProtocol::get( const KURL& url )
{
    kdDebug() << "kio_chm::get(const KURL& url) " << url.path() << endl;

    QString path;
    if ( !checkNewFile( url.path(), path ) ) {
        error( KIO::ERR_DOES_NOT_EXIST, url.prettyURL() );
        return;
    }

    if (m_dirMap.find(path) == m_dirMap.end()) {
        error( KIO::ERR_DOES_NOT_EXIST, url.prettyURL() );
        return;
    }

    QByteArray theData;

    //init..
    //added by lucida lucida@users.sf.net
    QString fname = QString();
    QString chmpath = QString();
    KTempFile f("",".html");
    fname = f.name();
    QTextStream *t = f.textStream();
    QString firstPage = QString("");
    QString m_strIndex = QString("");
    QString tmpstr = QString("");
    bool m_bIndex = 0;

    //try get some page to display, if the chm missing index
    ChmDirectoryMap::Iterator it;
    for ( it = m_dirMap.begin(); it != m_dirMap.end(); ++it) {
        tmpstr.sprintf("%s", it.key().latin1());
        if ((m_strIndex == "") && 
                        (tmpstr.endsWith(".htm") || tmpstr.endsWith(".html")))
        m_strIndex = tmpstr;
        if ((tmpstr == "/index.htm") || (tmpstr == "/index.html")) {
        m_strIndex = tmpstr;
        break;
        }
    }
    m_strIndex.remove(0,1);
    
    if (path == "/") {
        int offset = m_dirMap["/@contents"].offset;
        int length = m_dirMap["/@contents"].length;
        theData.setRawData(&m_contents[offset], length);
        QString s(theData);
        QString output;

        QRegExp object("<OBJECT type=\"text/sitemap\">(.*)</OBJECT>", false);
        object.setMinimal(true);

        QRegExp nameParam("<param name=\"Name\" value=\"(.*)\">", false);
        nameParam.setMinimal(true);

        QRegExp localParam("<param name=\"Local\" value=\"(.*)\">", false);
        localParam.setMinimal(true);

        QRegExp mergeParam("<param name=\"Merge\" value=\"(.*)\">", false);
        localParam.setMinimal(true);

        int old = 0, pos = 0;
        while ((pos = s.find(object, pos)) != -1) {
            output += s.mid(old, pos - old);
            pos += object.matchedLength();
            old = pos;
            QString obj = object.cap(1);
            QString name, local;
            if (obj.find(nameParam) != -1) {
                name = nameParam.cap(1);
                if (obj.find(localParam) != -1) {
                    local = localParam.cap(1);
                    //output += "<a href=\"" + local + "\">" + name + "</a>";
                    //added by lucida lucida@users.sf.net
                    if (local != "" && local != "/") {
                        output += "<a target=\"browse\" href=\"" + url.url() + local + "\">" + name + "</a>";
                        m_bIndex = 1;
                        if (firstPage == "") firstPage = url.url()+QString::fromLocal8Bit(local.latin1());
                    }
                    else 
                        output += name;
                } else {
                    output += name;
                }
            }
            if (obj.find(mergeParam) != -1) {
                QString link = mergeParam.cap(1);
                QString href = link.left(link.find("::"));
                QString path = m_chmFile.left(m_chmFile.findRev("/") + 1);
                //output += " (<a href=\"" + path + href + "\">link</a>)";
                m_bIndex = 1;
                output += " (<a target=\"browse\" href=\"" + url.url() + path + href + "\">link</a>)";
                if (firstPage == "") firstPage = url.url()+QString::fromLocal8Bit(local.latin1());
            }
        }
        output += s.mid(old);

        //set left pane
        //added by lucida, lucida@users.sf.net
        QString lframe = QString("</HEAD><FRAMESET COLS=\"25%,*\">\n");
        lframe += "<FRAME NAME=\"index\" src=\"file:"+ fname+"\"" + " marginwidth=\"0\"></FRAME>\n";
        if (!m_bIndex) {
            lframe = "</HEAD><FRAMESET>";
            firstPage = url.url() + QString::fromLocal8Bit(m_strIndex.latin1()); 
        }
        theData.resetRawData(&m_contents[offset], length);
        //KMimeMagicResult * result = KMimeMagic::self()->findBufferFileType( output, path );
        //kdDebug() << "Emitting mimetype " << result->mimeType() << endl;
        //mimeType( result->mimeType() );
/*        QCString output1 = (QCString)(output.latin1()); 
        data(output1); 
        processedSize(output1.length());*/
        
        //construct the frame
        //added by lucida lucida@users.sf.net
        QString framestr = QString("<HTML><HEAD>\n");
        framestr += lframe;
        framestr += "<FRAME NAME=\"browse\" src=\"" + firstPage + "\">\n";
        framestr += "</FRAME>\n";
        framestr += "</FRAMESET></HTML>";
        //write index file
        //added by lucida lucida@users.sf.net
        *t << QString::fromLocal8Bit(output.latin1()) << endl;

        data(framestr.local8Bit());
        processedSize(framestr.length());
    } else {
        int offset = m_dirMap[path].offset;
        int length = m_dirMap[path].length;
        totalSize(length);
        theData.setRawData(&m_contents[offset], length);

        KMimeMagicResult * result = KMimeMagic::self()->findBufferFileType( theData, path );
        kdDebug() << "Emitting mimetype " << result->mimeType() << endl;
        mimeType( result->mimeType() );
        data(theData);
        theData.resetRawData(&m_contents[offset], length);
        processedSize(length);
    }

    finished();
}

/* --------------------------------------------------------------------------- */
bool ChmProtocol::checkNewFile( QString fullPath, QString& path )
{
    //kdDebug() << "ChmProtocol::checkNewFile " << fullPath << endl;

    fullPath = fullPath.replace(QRegExp("::"), "");

	// Are we already looking at that file ?
    if ( !m_chmFile.isEmpty() && fullPath.startsWith(m_chmFile) )
    {
        path = fullPath.mid(m_chmFile.length()).lower();
        return true;
    }

    kdDebug() << "Need to open a new file" << endl;

    m_chmFile = "";

    // Find where the chm file is in the full path
    int pos = 0;
    QString chmFile;
    path = "";

    int len = fullPath.length();
    if ( len != 0 && fullPath[ len - 1 ] != '/' )
        fullPath += '/';

    //kdDebug() << "the full path is " << fullPath << endl;
    while ( (pos=fullPath.find( '/', pos+1 )) != -1 )
    {
        QString tryPath = fullPath.left( pos );
        //kdDebug() << fullPath << "  trying " << tryPath << endl;
        struct stat statbuf;
        if ( ::stat( QFile::encodeName(tryPath), &statbuf ) == 0 && !S_ISDIR(statbuf.st_mode) )
        {
            chmFile = tryPath;
            path = fullPath.mid( pos ).lower();
            kdDebug() << "fullPath=" << fullPath << " path=" << path << endl;
            len = path.length();
            if ( len > 2 )
            {
                if ( path[ len - 1 ] == '/' )
                    path.truncate( len - 1 );
            }
            else
            {
                path = QString::fromLatin1("/");
            }
            kdDebug() << "Found. chmFile=" << chmFile << " path=" << path << endl;
            break;
        }
    }
    if ( chmFile.isEmpty() )
    {
        kdDebug() << "ChmProtocol::checkNewFile: not found" << endl;
        return false;
    }

    m_chmFile = chmFile;

    // Open new file
    //kdDebug() << "Opening Chm file on " << chmFile << endl;
    return m_chm.read(chmFile, m_dirMap, m_contents);
}
