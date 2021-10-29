/*
    SPDX-FileCopyrightText: 2017 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "utils.h"

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>

namespace cppcheck
{

QString prettyPathName(const QString& path)
{
    return KDevelop::ICore::self()->projectController()->prettyFileName(
        QUrl::fromLocalFile(path),
        KDevelop::IProjectController::FormatPlain);
}

}
