/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MSVCCOMPILER_H
#define MSVCCOMPILER_H

#include "icompiler.h"

class MsvcCompiler : public ICompiler
{
public:
    MsvcCompiler(const QString& name, const QString& path, bool editable, const QString& factoryName);

    KDevelop::Defines defines(Utils::LanguageType type, const QString& arguments) const override;

    KDevelop::Path::List includes(Utils::LanguageType type, const QString& arguments) const override;
};

#endif // MSVCCOMPILER_H
