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

#ifndef COMPLETIONHELPER_H
#define COMPLETIONHELPER_H

#include <QStringList>
#include <QVector>

#include <language/duchain/duchainpointer.h>

struct FuncOverrideInfo
{
    QString returnType;
    QString name;
    QStringList params;
    bool isVirtual;
    bool isConst;
};

struct FuncImplementInfo
{
    bool isConstructor;
    bool isDestructor;
    QString templatePrefix;
    QString returnType;
    QString prototype;
};

namespace KTextEditor {
class Cursor;
}

class ParseSession;

Q_DECLARE_TYPEINFO(FuncOverrideInfo, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(FuncImplementInfo, Q_MOVABLE_TYPE);
using FunctionOverrideList = QVector<FuncOverrideInfo>;
using FunctionImplementsList = QVector<FuncImplementInfo>;

class CompletionHelper
{
public:
    CompletionHelper();

    void computeCompletions(const ParseSession& session,
                            const KTextEditor::Cursor& position);

    FunctionOverrideList overrides() const;
    FunctionImplementsList implements() const;
private:
    FunctionOverrideList m_overrides;
    FunctionImplementsList m_implements;
};

#endif //COMPLETIONHELPER_H
