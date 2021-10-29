/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVCLAZY_UTILS_H
#define KDEVCLAZY_UTILS_H

#include <QString>
class QUrl;

namespace Clazy
{

QString prettyPathName(const QUrl& path);

QString markdown2html(const QByteArray& markdown);

}

#endif
