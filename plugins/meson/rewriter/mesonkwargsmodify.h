/* This file is part of KDevelop
    Copyright 2019 Daniel Mensinger <daniel@mensinger-ka.de>

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
    explicit MesonKWARGSModify(Function fn, Operation op, QString const& id);
    virtual ~MesonKWARGSModify();

    QJsonObject command() override;

    Function function() const;
    Operation operation() const;
    QString id() const;

    void set(QString const& kwarg, QJsonValue const& value);
    void unset(QString const& kwarg);
    void clear();

    bool isSet(QString const& kwarg);

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
    explicit MesonKWARGSTargetModify(Operation op, QString const& id);
    virtual ~MesonKWARGSTargetModify();
};

class MesonKWARGSDependencyModify : public MesonKWARGSModify
{
public:
    explicit MesonKWARGSDependencyModify(Operation op, QString const& id);
    virtual ~MesonKWARGSDependencyModify();
};
