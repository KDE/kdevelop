/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "urlutil.h"

QString URLUtil::filename(const QString & name) {
  int slashPos = name.findRev("/");
  return slashPos<0 ? name : name.mid(slashPos+1);
}

QString URLUtil::directory(const QString & name) {
  int slashPos = name.findRev("/");
  return slashPos<0 ? QString("") : name.left(slashPos);
}

QString URLUtil::relativePath(const KURL & parent, const KURL & child, uint slashPolicy) {
  bool slashPrefix = slashPolicy & SLASH_PREFIX;
  bool slashSuffix = slashPolicy & SLASH_SUFFIX;
  if (parent.cmp(child,true))
    return slashPrefix ? QString("/") : QString("");

  if (!parent.isParentOf(child)) return QString();
  int a=slashPrefix ? -1 : 1;
  int b=slashSuffix ? 1 : -1;
  return child.path(b).mid(parent.path(a).length());
}

QString URLUtil::relativePath(const QString & parent, const QString & child, uint slashPolicy) {
  return relativePath(KURL(parent), KURL(child), slashPolicy);
}

QString URLUtil::upDir(const QString & path, bool slashSuffix) {
  int slashPos = path.findRev("/");
  if (slashPos<1) return QString::null;
  return path.mid(0,slashPos+ (slashSuffix ? 1 : 0) );
}

KURL URLUtil::mergeURL(const KURL & source, const KURL & dest, const KURL & child) {

  // if already a child of source, then fine
  if (source.isParentOf(child) || source.cmp(child,true)) return child;

  // if not a child of dest, return blank URL (error)
  if (!dest.isParentOf(child) && !dest.cmp(child,true)) return KURL();

  // if child is same as dest, return source
  if (dest.cmp(child,true)) return source;

  // calculate
  QString childUrlStr = child.url(-1);
  QString destStemStr = dest.url(1);
  QString sourceStemStr = source.url(1);
  return KURL(sourceStemStr.append( childUrlStr.mid( destStemStr.length() ) ) );

}

QString URLUtil::getExtension(const QString & path) {
  int dotPos = path.findRev('.');
  if (dotPos<0) return QString("");
  return path.mid(dotPos+1);
}
