/*
 * KDevelop PHP Language Support
 *
 * Copyright (c) 2005 Escuder Nicolas <n.escuder@intra-links.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef PHPLANGUAGESUPPORT_H
#define PHPLANGUAGESUPPORT_H

#include <kdevlanguagesupport.h>

class PHPLanguageSupport: public KDevLanguageSupport
{
  Q_OBJECT
public:
  PHPLanguageSupport(QObject *parent, const char *name, const QStringList &args);
  virtual ~PHPLanguageSupport();

  /**@return The feature set of the language. This is e.g. used
  by the class view to decide which organizer items to display and which not.*/
  virtual int features() const;

  /**@return A typical mimetype list for the support language, this list
  should be configurable in the languagesupport dialog.*/
  virtual QStringList mimeTypes() const;

  /**Formats a Tag as used by the persistant symbol store to the human-readable convention.
  @param tag Tag to format.*/
  virtual QString formatTag(const Tag& tag) const;

  /**Formats a KDevCodeItem as used by the CodeModel to the human-readable convention.
  @param item Symbol to format.
  @param shortDescription Show short description of a symbol. For example, when
  formatting functions short description could be a function signature without
  the return type and argument default values.*/
  virtual QString formatModelItem(const KDevCodeItem *item, bool shortDescription=false) const;

  /**Formats a canonicalized class path as used by the symbol store to the
  human-readable convention. For example, the C++ support part formats the
  string "KParts.Part" into "KParts::Part".
  @param name Class name.*/
  virtual QString formatClassName(const QString &name) const;

  /**The opposite of @ref formatClassName. Reverts formatting.
  @param name Class name.*/
  virtual QString unformatClassName(const QString &name) const;

private slots:
    void documentLoaded( const KURL &url );
    void documentClosed( const KURL &url );

private:
    bool isPHPLanguageDocument( const KURL &url );

private:
    int m_features;
    QStringList m_mimetypes;
};

#endif // PHPLANGUAGESUPPORT_H
