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

#include <QJsonObject>

class MesonKWARGSInfo;
class MesonKWARGSProjectInfo;
class MesonKWARGSTargetInfo;
class MesonKWARGSDependencyInfo;

using MesonKWARGSInfoPtr = std::shared_ptr<MesonKWARGSInfo>;
using MesonKWARGSProjectInfoPtr = std::shared_ptr<MesonKWARGSProjectInfo>;
using MesonKWARGSTargetInfoPtr = std::shared_ptr<MesonKWARGSTargetInfo>;
using MesonKWARGSDependencyInfoPtr = std::shared_ptr<MesonKWARGSDependencyInfo>;

class MesonKWARGSInfo : public MesonRewriterActionBase
{
public:
    enum Function { PROJECT, TARGET, DEPENDENCY };

public:
    explicit MesonKWARGSInfo(Function fn, QString id);
    virtual ~MesonKWARGSInfo();

    QJsonObject command() override;
    void parseResult(QJsonObject data) override;

    Function function() const;
    QString id() const;

    bool hasKWARG(QString kwarg) const;
    QJsonValue get(QString kwarg) const;
    QString getString(QString kwarg) const;
    QStringList getArray(QString kwarg) const;

private:
    Function m_func;
    QString m_id;
    QJsonObject m_result;

    QString m_infoID;
};

class MesonKWARGSProjectInfo : public MesonKWARGSInfo
{
public:
    explicit MesonKWARGSProjectInfo();
    virtual ~MesonKWARGSProjectInfo();
};

class MesonKWARGSTargetInfo : public MesonKWARGSInfo
{
public:
    explicit MesonKWARGSTargetInfo(QString id);
    virtual ~MesonKWARGSTargetInfo();
};

class MesonKWARGSDependencyInfo : public MesonKWARGSInfo
{
public:
    explicit MesonKWARGSDependencyInfo(QString id);
    virtual ~MesonKWARGSDependencyInfo();
};
