/*
    This file is part of KDevelop

    Copyright 2015 Milian Wolff <mail@milianw.de>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef UNSAVEDFILE_H
#define UNSAVEDFILE_H

#include <QStringList>

struct CXUnsavedFile;

/**
 * Wrapper API to map unsaved editor contents to the CXUnsavedFile API for clang.
 */
class UnsavedFile
{
public:
    UnsavedFile(const QString& fileName = {}, const QStringList& contents = {});

    CXUnsavedFile toClangApi() const;

private:
    QString m_fileName;
    QStringList m_contents;
    // lazy-loaded byte arrays for usage in clang API
    void convertToUtf8();
    QByteArray m_fileNameUtf8;
    QByteArray m_contentsUtf8;
};

Q_DECLARE_TYPEINFO(UnsavedFile, Q_MOVABLE_TYPE);

#endif // UNSAVEDFILE_H
