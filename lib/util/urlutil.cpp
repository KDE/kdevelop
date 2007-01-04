/* This file is part of the KDE project
   Copyright (C) 2003 Julian Rockey <linux@jrockey.com>
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>

   This library is free software; you can redistribute it and/or
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
#include <qstringlist.h>

#include <qdir.h>
#include <qfileinfo.h>
#include <kdebug.h>

#include <unistd.h>
#include <limits.h>
#include <stdlib.h>

#include "urlutil.h"

#include <kdeversion.h>

///////////////////////////////////////////////////////////////////////////////
// Namespace URLUtil
///////////////////////////////////////////////////////////////////////////////

QString URLUtil::filename(const QString & name) {
  int slashPos = name.findRev("/");
  return slashPos<0 ? name : name.mid(slashPos+1);
}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::directory(const QString & name) {
  int slashPos = name.findRev("/");
  return slashPos<0 ? QString("") : name.left(slashPos);
}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::relativePath(const KURL & parent, const KURL & child, uint slashPolicy) {
  bool slashPrefix = slashPolicy & SLASH_PREFIX;
  bool slashSuffix = slashPolicy & SLASH_SUFFIX;
  if (parent.equals(child,true))
    return slashPrefix ? QString("/") : QString("");

  if (!parent.isParentOf(child)) return QString();
  int a=slashPrefix ? -1 : 1;
  int b=slashSuffix ? 1 : -1;
  return child.path(b).mid(parent.path(a).length());
}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::relativePath(const QString & parent, const QString & child, uint slashPolicy) {
  return relativePath(KURL(parent), KURL(child), slashPolicy);
}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::upDir(const QString & path, bool slashSuffix) {
  int slashPos = path.findRev("/");
  if (slashPos<1) return QString::null;
  return path.mid(0,slashPos+ (slashSuffix ? 1 : 0) );
}

///////////////////////////////////////////////////////////////////////////////

KURL URLUtil::mergeURL(const KURL & source, const KURL & dest, const KURL & child) {

  // if already a child of source, then fine
  if (source.isParentOf(child) || source.equals(child,true)) return child;

  // if not a child of dest, return blank URL (error)
  if (!dest.isParentOf(child) && !dest.equals(child,true)) return KURL();

  // if child is same as dest, return source
  if (dest.equals(child,true)) return source;

  // calculate
  QString childUrlStr = child.url(-1);
  QString destStemStr = dest.url(1);
  QString sourceStemStr = source.url(1);
  return KURL(sourceStemStr.append( childUrlStr.mid( destStemStr.length() ) ) );

}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::getExtension(const QString & path) {
  int dotPos = path.findRev('.');
  if (dotPos<0) return QString("");
  return path.mid(dotPos+1);
}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::extractPathNameRelative(const KURL &baseDirUrl, const KURL &url )
{
  QString absBase = extractPathNameAbsolute( baseDirUrl ),
    absRef = extractPathNameAbsolute( url );
  int i = absRef.find( absBase, 0, true );

  if (i == -1)
    return QString();

  if (absRef == absBase)
    return QString( "." );
  else
    return absRef.replace( 0, absBase.length(), QString() );
}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::extractPathNameRelative(const QString &basePath, const KURL &url )
{
  KURL baseDirUrl = KURL::fromPathOrURL( basePath );
  return extractPathNameRelative( baseDirUrl, url );
}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::extractPathNameRelative(const QString &basePath, const QString &absFilePath )
{
  KURL baseDirUrl = KURL::fromPathOrURL( basePath ),
       fileUrl = KURL::fromPathOrURL( absFilePath );
  return extractPathNameRelative( baseDirUrl, fileUrl );
}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::extractPathNameAbsolute( const KURL &url )
{
  if (isDirectory( url ))
    return url.path( +1 ); // with trailing "/" if none is present
  else
  {
    // Ok, this is an over-tight pre-condition on "url" since I hope nobody will never
    // stress this function with absurd cases ... but who knows?
  /*
    QString path = url.path();
    QFileInfo fi( path );  // Argh: QFileInfo is back ;))
    return ( fi.exists()? path : QString() );
  */
    return url.path();
  }
}

///////////////////////////////////////////////////////////////////////////////

bool URLUtil::isDirectory( const KURL &url )
{
  return isDirectory( url.path() );
}

///////////////////////////////////////////////////////////////////////////////

bool URLUtil::isDirectory( const QString &absFilePath )
{
  return QDir( absFilePath ).exists();
}

///////////////////////////////////////////////////////////////////////////////

void URLUtil::dump( const KURL::List &urls, const QString &aMessage )
{
  if (!aMessage.isNull())
  {
    kdDebug(9000) << aMessage << endl;
  }
  kdDebug(9000) << " List has " << urls.count() << " elements." << endl;

  for (size_t i = 0; i<urls.count(); ++i)
  {
    KURL url = urls[ i ];
//    kdDebug(9000) << " * Element = "  << url.path() << endl;
  }
}

///////////////////////////////////////////////////////////////////////////////

QStringList URLUtil::toRelativePaths( const QString &baseDir, const KURL::List &urls)
{
  QStringList paths;

  for (size_t i=0; i<urls.count(); ++i)
  {
    paths << extractPathNameRelative( baseDir, urls[i] );
  }

  return paths;
}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::relativePathToFile( const QString & dirUrl, const QString & fileUrl )
{
  if (dirUrl.isEmpty() || (dirUrl == "/"))
    return fileUrl;

  QStringList dir = QStringList::split("/", dirUrl, false);
  QStringList file = QStringList::split("/", fileUrl, false);

  QString resFileName = file.last();
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
      //qWarning("i = %d, currDir = %s, currFile = %s", i, currDir.latin1(), currFile.latin1());
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

///////////////////////////////////////////////////////////////////////////////

//TODO: remove for KDE4
QString URLUtil::canonicalPath( const QString & path )
{
    QDir dir(path);
    return dir.canonicalPath();
}

///////////////////////////////////////////////////////////////////////////////

//written by "Dawit A." <adawit@kde.org>
//borrowed from his patch to KShell
QString URLUtil::envExpand ( const QString& str )
{
    uint len = str.length();

    if (len > 1 && str[0] == '$')
    {
      int pos = str.find ('/');

      if (pos < 0)
        pos = len;

      char* ret = getenv( QConstString(str.unicode()+1, pos-1).string().local8Bit().data() );

      if (ret)
      {
        QString expandedStr ( QFile::decodeName( ret ) );
        if (pos < (int)len)
          expandedStr += str.mid(pos);
        return expandedStr;
      }
    }

    return str;
}

