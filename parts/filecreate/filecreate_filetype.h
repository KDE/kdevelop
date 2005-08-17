/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef __FILECREATE_FILETYPE_H__
#define __FILECREATE_FILETYPE_H__

#include <qstring.h>
#include <q3ptrlist.h>

namespace FileCreate {

class FileType {

public:

  FileType() : m_enabled(false) {
    m_subtypes.setAutoDelete(true);
  }

  void setName(const QString & name) { m_name = name; }
  QString name() const { return m_name; }
  void setExt(const QString & ext) { m_ext = ext; }
  QString ext() const { return m_ext; }
  void setCreateMethod(const QString & createMethod) { m_createMethod = createMethod; }
  QString createMethod() const { return m_createMethod; }
  void setSubtypeRef(const QString & subtypeRef) { m_subtypeRef = subtypeRef; }
  QString subtypeRef() const { return m_subtypeRef; }
  void setIcon(const QString & iconName) { m_iconName = iconName; }
  QString icon() const { return m_iconName; }
  void setDescr(const QString & descr) { m_descr = descr; }
  QString descr() const { return m_descr; }
  void setEnabled(bool on) { m_enabled = on; }
  bool enabled() const { return m_enabled; }

  void setSubtypesEnabled(bool enabled = true);
  
  void addSubtype(const FileType * subtype) { m_subtypes.append(subtype); }
  Q3PtrList<FileType> subtypes() const { return m_subtypes; }
  
private:
  QString m_name;
  QString m_ext;
  QString m_createMethod;
  QString m_subtypeRef;
  QString m_iconName;
  QString m_descr;

  bool m_enabled;
  
  Q3PtrList<FileType> m_subtypes;

};

}

#endif
