#include <qstringlist.h>

#include "rurl.h"

namespace Relative{


//class Name

Name::Name( const QString & rurl, const Type type )
    :m_rurl(rurl), m_type(type)
{
    correct();
}

Name::Name( const char * rurl, const Type type )
    :m_rurl(rurl), m_type(type)
{
    correct();
}

void Name::correct()
{
    cleanRURL();
    if (m_rurl[0] == '/')
        m_rurl = m_rurl.mid(1);
    switch (m_type)
    {
        case File:
            if (m_rurl.endsWith("/"))
                m_rurl = m_rurl.mid(0, m_rurl.length()-1);
            break;
        case Directory:
            if (!m_rurl.endsWith("/"))
                m_rurl += "/";
            break;
        case Auto:
            if (m_rurl.endsWith("/"))
                m_type = Directory;
            else
                m_type = File;
            break;
    }
}

QString Name::correctName( const QString & rurl, const Type type )
{
    QString temp = rurl;
    temp = Name::cleanName(temp);
    if (temp[0] == '/')
        temp = temp.mid(1);
    
    switch (type)
    {
        case File:
            if (temp.endsWith("/"))
                temp = temp.mid(0, temp.length()-1);
            break;
        case Directory:
            if (!temp.endsWith("/"))
                temp += "/";
            break;
    }
    
    return temp;
}

void Name::setRURL( const QString & rurl, const Type type )
{
    m_rurl = rurl;
    m_type = type;
    correct();
}

QString Name::rurl( ) const
{
    return m_rurl;
}

void Name::addPath( const QString & addendum )
{
    QString temp = correctName(addendum, Directory);
    m_rurl = directory() + temp + fileName();
}

void Name::cleanRURL( )
{
    m_rurl = cleanName(m_rurl);
}

QString Name::cleanName( const QString & rurl )
{
    QString temp;
    bool wasSlash = false;
    for (unsigned int i = 0; i < rurl.length(); ++i)
    {
        if (wasSlash && (rurl[i] == '/'))
            continue;
        
        temp += rurl[i];
        if (rurl[i] == '/')
            wasSlash = true;
        else if (wasSlash)
            wasSlash = false;
    }
    
    return temp;
}

QString Name::extension( bool complete ) const
{
    if (m_type == File)
    {
        QString temp = fileName();
        if (complete)
            return temp.mid(temp.find('.')+1);
        else
            return temp.mid(temp.findRev('.')+1);
    }
    return QString::null;
}

QString Name::fileName( ) const
{
    if (m_type == File)
        return m_rurl.section('/', -1);
    return QString::null;
}

QString Name::directory( ) const
{
    if ( (m_type == File) && (m_rurl.findRev('/') == -1) )
        return QString::null;
    
    return m_rurl.mid(0, m_rurl.findRev('/')+1);
}

bool Name::isFile( ) const
{
    return m_type == File;
}

bool Name::isDirectory( ) const
{
    return m_type == Directory;
}

bool Name::operator ==( const Name & rname )
{
    return rname.rurl() == m_rurl;
}

bool Name::operator !=( const Name & rname )
{
    return rname.rurl() != m_rurl;
}

bool Name::isValid( ) const
{
    if (m_rurl.startsWith("/"))
        return false;
    if (m_rurl.contains("//"))
        return false;
    if ( (m_rurl.endsWith("/")) && (m_type == File) )
        return false;
    if ( (!m_rurl.endsWith("/")) && (m_type == Directory) )
        return false;
    if (m_type == Auto)
        return false;
    
    return true;
}

Name::Type Name::type( ) const
{
    return m_type;
}

void Name::setType( const Type type )
{
    m_type = type;
}

Name Name::relativeName( const QString &base, const QString &url )
{
    QString dirUrl = base;
    QString fileUrl = url;
        
    if (dirUrl.isEmpty() || (dirUrl == "/"))
        return Name(fileUrl);

    QStringList dir = QStringList::split("/", dirUrl, false);
    QStringList file = QStringList::split("/", fileUrl, false);

    QString resFileName = file.last();
    if (url.endsWith("/"))
        resFileName += "/";
    file.remove(file.last());

    uint i = 0;
    while ( (i < dir.count()) && (i < (file.count())) && (dir[i] == file[i]) )
        i++;

    QString result_up;
    QString result_down;
    QString currDir;
    QString currFile;
    do
    {
        i >= dir.count() ? currDir = "" : currDir = dir[i];
        i >= file.count() ? currFile = "" : currFile = file[i];
//        qWarning("i = %d, currDir = %s, currFile = %s", i, currDir.latin1(), currFile.latin1());
        if (currDir.isEmpty() && currFile.isEmpty())
            break;
        else if (currDir.isEmpty())
            result_down += file[i] + "/";
        else if (currFile.isEmpty())
            result_up += "../";
        else
        {
            result_down += file[i] + "/";
            result_up += "../";
        }
        i++;
    }
    while ( (!currDir.isEmpty()) || (!currFile.isEmpty()) );

    return result_up + result_down + resFileName;            
}



//class URL

URL::URL( KURL base, KURL url, Type type )
    :Name(Name::relativeName(base.path(), url.path()).rurl(), type), m_base(base)
{
}

URL::URL( KURL base, QString url, bool isUrlRelative, Type type )
    :Name(isUrlRelative ? url : Name::relativeName(base.path(), url).rurl(), type), m_base(base)
{
}

void URL::setBase( const KURL & base )
{
    m_base = base;
}

void URL::setBase( const QString & base )
{
    KURL url;
    url.setPath(base);
    m_base = url;
}

KURL URL::base( ) const
{
    return m_base;
}

QString URL::basePath( ) const
{
    return m_base.path(1);
}

KURL URL::url( ) const
{
    KURL url = m_base;
    url.addPath(rurl());
    url.cleanPath();
    return url;
}

QString URL::urlPath( ) const
{
    KURL url = m_base;
    url.addPath(rurl());
    int mod = 0;
    if (type() == File)
        mod = -1;
    else if (type() == Directory)
        mod = 1;
    url.cleanPath();
    return url.path(mod);
}

QString URL::urlDirectory( ) const
{
    KURL url = m_base;
    url.addPath(rurl());
    url.cleanPath();
    return url.directory(false, false);
}

URL URL::relativeTo( KURL base )
{
    return URL(base, url(), type());
}

URL URL::relativeURL( KURL base, KURL url )
{
    return URL(base, url);
}

URL URL::relativeURL( KURL base, QString url, bool isUrlRelative )
{
    return URL(base, url, isUrlRelative);
}

bool Relative::URL::operator ==( const URL & url )
{
    return (m_base == url.base()) && (rurl() == url.rurl());
}

bool Relative::URL::operator !=( const URL & url )
{
    return (m_base != url.base()) || (rurl() != url.rurl());
}



// Directory class

Directory::Directory( KURL base, KURL url )
    :URL(base, url, Name::Directory)
{
}

Directory::Directory( KURL base, QString url, bool isRelativeUrl )
    :URL(base, url, isRelativeUrl, Name::Directory)
{
}

void Directory::setRURL( QString rurl )
{
    URL::setRURL(rurl, Name::Directory);
}

void Directory::setRURL( QString rurl, Type type )
{
    URL::setRURL(rurl, type);
}



//File class

File::File( KURL base, KURL url )
    :URL(base, url, Name::File)
{
}

File::File( KURL base, QString url, bool isRelativeUrl )
    :URL(base, url, isRelativeUrl, Name::File)
{
}

void File::setRURL( QString rurl, Type type )
{
    URL::setRURL(rurl, type);
}

void File::setRURL( QString rurl )
{
    URL::setRURL(rurl, Name::File);
}

}
