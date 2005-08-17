#ifndef RURL_H_LIB
#define RURL_H_LIB

#include <kurl.h>

/**
@file rurl.h
Classes and functions to work with relative URLs.
*/

/**Classes and functions to work with relative URLs.*/
namespace Relative
{

/**Relative name of a file or directory.*/   
class Name{
public:
    enum Type { File, Directory, Auto };
    
    /**Constructor takes the relative name of a directory or file.
    Leading slash in the name will be deleted.
    If type is Auto names like:
    name/name/ are directories
    name/name are files.
    Trailing slash will be deleted for files (type == File).*/
    Name(const QString &rurl, const Type type = Auto);
    Name(const char *rurl, const Type type = Auto);
    
    /**Sets the relative name.*/
    void setRURL(const QString &rurl, const Type type);
    /**Gets the relative name in form 
    dir/dir/  -> directory
    or
    dir/dir/file -> file.*/
    QString rurl() const;
    
    /**Adds addendum to the directory path. This honors file names so
    if RName represents /dir1/dir2/fname.ext
    addPath(dir3) will change RName to /dir1/dir2/dir3/fname.ext*/
    void addPath(const QString &addendum);
    /**Removes "//" from the name.*/
    void cleanRURL();
    
    /**Returns the extension of a file or QString() for directories.
    If complete is true then returns extensions like "tar.gz".
    Else returns "gz".*/
    QString extension(bool complete = true) const;
    /**Returns the name of the file without the path or QString()
    for directories.*/
    QString fileName() const;
    /**Returns the name of the directory or QString() if there are no dirs in path.*/
    QString directory() const;

    /**Returns true if the type of RName is file.*/
    bool isFile() const;
    /**Returns true if the type of RName is directory.*/
    bool isDirectory() const;
    /**Checks if RName is valid.*/
    bool isValid() const;
    
    /**Returns a type of the relative name - file or directory.*/
    Type type() const;
    /**Sets a type of the relative name - file or directory.
    If Auto is passed, nothing happens.*/
    void setType(const Type type);
    
    /**Creates and returns relative name between base and url. Base and url should be absolute.
    Base is considered to be a directory.*/
    static Name relativeName(const QString &base, const QString &url);
    /**Cleans rurl by removing extra slashes.*/
    static QString cleanName(const QString &rurl);
    /**Corrects rurl according to the given type and returns corrected url.
    Also cleans url (see @ref cleanRURL).*/
    static QString correctName(const QString &rurl, const Type type = Auto);

    bool operator == (const Name &rname);
    bool operator != (const Name &rname);

protected:
    /**Corrects m_rurl and m_type according to the relative name storing policy,
    i.e. removes leading slash, removes trailing slash for files, changes type
    to be either File or Directory, but not Auto. Also cleans url (see @ref cleanRURL).*/
    void correct();

private:
    QString m_rurl;    
    Type m_type;
};

/**Relative name of file or directory to some base location.*/
class URL: public Name{
public:
    /**Evaluates the relative path between url and base and creates RURL object.
    base should be an url to the directory or location, not a file.
    The check is not performed. url should be the usual url. Only the
    path of this url is taken into account when evaluating relative path.*/
    URL(KURL base, KURL url, Type type = Auto);
    /**Creates RURL object with given base and relative or full url (according to
    the isUrlRelative value).*/
    URL(KURL base, QString url, bool isUrlRelative, Type type = Auto);

    /**Sets a new base for a RURL.*/
    void setBase(const KURL &base);
    /**Sets a new base for a RURL. Base is is considered to be a
    directory and converted to KURL using KURL::setPath().*/
    void setBase(const QString &base);
    /**Returns RURL base.*/
    KURL base() const;
    /**Returns a path of a base KURL (using KURL::path()). Trailing slash is guaranteed.*/
    QString basePath() const;

    /**Returns a complete url to the RURL location. This is basically base + rurl.
    This also resolves ".." components in path.
    Directories always have trailing slash in KURL
    (this means that if url() == "file:/test/dir/" then
    url() != KURL("/test/dir") and
    url() == KURL("/test/dir/").*/
    KURL url() const;
    /**Returns a path of a complete url to the location. The url is basically base + rurl.
    This method only returns a path part of the KURL (using KURL::path()).
    Trailing slash is guaranteed for directories and no trailing slash - for files.
    This also resolves ".." components in path.*/
    QString urlPath() const;
    /**Returns a directory of a complete url to the location. The url is constructed as base + rurl.
    Returns the same as urlPath() for directories.
    This method uses KURL::directory to determine the directory.
    Trailing slash is guaranteed.
    This also resolves ".." components in path.*/
    QString urlDirectory() const;

    /**Returns a new URL that is relative to given base. Relative part is taken from
    current URL object.*/
    URL relativeTo(KURL base);

    /**Returns a new relative URL constructed from base and given url.*/
    static URL relativeURL(KURL base, KURL url);
    /**Returns a new relative URL constructed from base and given url. url parameter
    is either relative or full (depends on isUrlRelative value).*/
    static URL relativeURL(KURL base, QString url, bool isUrlRelative);

    bool operator == (const URL &url);
    bool operator != (const URL &url);

private:
    KURL m_base;
};

/**Relative directory name.*/
class Directory: public URL{
public:
    /**Works as URL::URL(KURL, KURL), only implies Name::Directory mode.*/
    Directory(KURL base, KURL url);
    /**Works as URL::URL(KURL, QString, bool), only implies Name::Directory mode.*/
    Directory(KURL base, QString url, bool isRelativeUrl);

    /**Works as URL::setRURL(QString), only implies Name::Directory mode.*/
    void setRURL(QString rurl);

private:
    void setRURL(QString rurl, Type type);

};

/**Relative file name.*/
class File: public URL{
public:
    /**Works as URL::URL(KURL, KURL), only implies Name::File mode.*/
    File(KURL base, KURL url);
    /**Works as URL::URL(KURL, KURL), only implies Name::File mode.*/
    File(KURL base, QString url, bool isRelativeUrl);

    /**Works as URL::setRURL(QString), only implies Name::File mode.*/
    void setRURL(QString rurl);

private:
    void setRURL(QString rurl, Type type);
    
};

};

#endif
