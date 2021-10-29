/*
    SPDX-FileCopyrightText: 2015 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef UNSAVEDFILE_H
#define UNSAVEDFILE_H

#include <QStringList>

#include "clangprivateexport.h"

struct CXUnsavedFile;

/**
 * Wrapper API to map unsaved editor contents to the CXUnsavedFile API for clang.
 */
class KDEVCLANGPRIVATE_EXPORT UnsavedFile
{
public:
    explicit UnsavedFile(const QString& fileName = {}, const QStringList& contents = {});

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
