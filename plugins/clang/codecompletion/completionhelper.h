/*
    SPDX-FileCopyrightText: 2014 David Stevens <dgedstevens@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef COMPLETIONHELPER_H
#define COMPLETIONHELPER_H

#include <QStringList>
#include <QVector>
#include <clang-c/Index.h>

#include <language/duchain/duchainpointer.h>

struct FuncParameterInfo
{
    QString type;
    QString id;
    /// Returns true if types are equal, id is ignored
    bool operator==(const FuncParameterInfo& rhs) const { return type == rhs.type; }
};
Q_DECLARE_TYPEINFO(FuncParameterInfo, Q_MOVABLE_TYPE);
using FuncParameterList = QVector<FuncParameterInfo>;

struct FuncOverrideInfo
{
    QString returnType;
    QString name;
    FuncParameterList params;
    bool isPureVirtual;
    bool isConst;
    /// Returns true if equal, isPureVirtual & parameter ids are ignored
    bool operator==(const FuncOverrideInfo& rhs) const;
};

struct FuncImplementInfo
{
    bool isConstructor;
    bool isDestructor;
    QString templatePrefix;
    QString returnType;
    QString prototype;
    KDevelop::DeclarationPointer declaration;
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

    void computeCompletions(const ParseSession& session, CXFile file,
                            const KTextEditor::Cursor& position);

    FunctionOverrideList overrides() const;
    FunctionImplementsList implements() const;
private:
    FunctionOverrideList m_overrides;
    FunctionImplementsList m_implements;
};

#endif //COMPLETIONHELPER_H
