/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "mesonactionbase.h"

class MesonKWARGSModify;
class MesonKWARGSProjectModify;
class MesonKWARGSTargetModify;
class MesonKWARGSDependencyModify;

using MesonKWARGSModifyPtr = std::shared_ptr<MesonKWARGSModify>;
using MesonKWARGSProjectModifyPtr = std::shared_ptr<MesonKWARGSProjectModify>;
using MesonKWARGSTargetModifyPtr = std::shared_ptr<MesonKWARGSTargetModify>;
using MesonKWARGSDependencyModifyPtr = std::shared_ptr<MesonKWARGSDependencyModify>;

class MesonKWARGSModify : public MesonRewriterActionBase
{
public:
    enum Function { PROJECT, TARGET, DEPENDENCY };
    enum Operation { SET, DELETE };

public:
    explicit MesonKWARGSModify(Function fn, Operation op, const QString& id);
    virtual ~MesonKWARGSModify();

    QJsonObject command() override;

    Function function() const;
    Operation operation() const;
    QString id() const;

    void set(const QString& kwarg, const QJsonValue& value);
    void unset(const QString& kwarg);
    void clear();

    bool isSet(const QString& kwarg);

private:
    Function m_func;
    Operation m_op;
    QString m_id;

    QJsonObject m_kwargs = QJsonObject();
};

class MesonKWARGSProjectModify : public MesonKWARGSModify
{
public:
    explicit MesonKWARGSProjectModify(Operation op);
    virtual ~MesonKWARGSProjectModify();
};

class MesonKWARGSTargetModify : public MesonKWARGSModify
{
public:
    explicit MesonKWARGSTargetModify(Operation op, const QString& id);
    virtual ~MesonKWARGSTargetModify();
};

class MesonKWARGSDependencyModify : public MesonKWARGSModify
{
public:
    explicit MesonKWARGSDependencyModify(Operation op, const QString& id);
    virtual ~MesonKWARGSDependencyModify();
};
