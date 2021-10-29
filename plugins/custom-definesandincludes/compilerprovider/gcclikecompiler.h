/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef GCCLIKECOMPILER_H
#define GCCLIKECOMPILER_H

#include "icompiler.h"

class GccLikeCompiler : public QObject, public ICompiler
{
    Q_OBJECT
public:
    GccLikeCompiler( const QString& name, const QString& path, bool editable, const QString& factoryName );

    KDevelop::Defines defines(Utils::LanguageType type, const QString& arguments) const override;

    KDevelop::Path::List includes(Utils::LanguageType type, const QString& arguments) const override;

private:
    void invalidateCache();

    template <typename T>
    struct Cached {
        T data;
        bool wasCached = false;
    };
    struct DefinesIncludes {
        Cached<KDevelop::Defines> definedMacros;
        Cached<KDevelop::Path::List> includePaths;
    };

    /// List of defines/includes per arguments
    mutable QHash<Utils::LanguageType, QHash<QString, DefinesIncludes>> m_definesIncludes;
    mutable QHash<QStringList, Cached<KDevelop::Defines>> m_defines;
    mutable QHash<QStringList, Cached<KDevelop::Path::List>> m_includes;
};

#endif // GCCLIKECOMPILER_H

