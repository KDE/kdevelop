/*
 * This file is part of KDevelop
 *
 * Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
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

#ifndef CPP_QUICKOPEN_H
#define CPP_QUICKOPEN_H

#include <quickopendataprovider.h>
#include <kurl.h>

class IncludeFileData : public KDevelop::QuickOpenDataBase {
  public:
    IncludeFileData( const KUrl& file, bool isDir = false );
    
    virtual QString text() const;
    virtual QString htmlDescription() const;
  private:
    KUrl m_file;
    bool m_isDirectory;
};

/**
 * A QuickOpenDataProvider for file-completion using include-paths.
 * It provides all files from the whole include-path, filters them by the text, and
 * also searches sub-directories if the typed text wants it.
 * */

class IncludeFileDataProvider : public KDevelop::QuickOpenDataProviderBase {
  public:
    virtual void setFilterText( const QString& text );
    virtual void reset();
    virtual uint itemCount() const;
    virtual QList<KDevelop::QuickOpenDataPointer> data( uint start, uint end ) const;
  private:
    QString m_oldText;
};

#endif
