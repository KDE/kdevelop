/*
 * This file is part of KDevelop
 * Copyright 2014 David Stevens <dgedstevens@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OVERRIDECOMPLETIONHELPER_H
#define OVERRIDECOMPLETIONHELPER_H

#include <clang-c/Index.h>
#include <language/editor/simplecursor.h>
#include <QStringList>
#include "codecompletionexport.h"

struct FunctionInfo
{
    QString returnType;
    QString name;
    QStringList params;
    bool isVirtual;
};

Q_DECLARE_TYPEINFO(FunctionInfo, Q_MOVABLE_TYPE);
using FunctionInfoList = QVector<FunctionInfo>;

class KDEVCLANGCODECOMPLETION_EXPORT OverrideCompletionHelper
{
public:
    OverrideCompletionHelper(const CXTranslationUnit& unit,
                             const KDevelop::SimpleCursor& position,
                             const char *filename);

    FunctionInfoList getOverrides();
private:
    FunctionInfoList m_overrides;
};

#endif //OVERRIDECOMPLETIONHELPER_H
