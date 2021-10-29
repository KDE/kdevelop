/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    explicit MesonKWARGSInfo(Function fn, const QString& id);
    virtual ~MesonKWARGSInfo();

    QJsonObject command() override;
    void parseResult(QJsonObject data) override;

    Function function() const;
    QString id() const;

    bool hasKWARG(const QString& kwarg) const;
    QJsonValue get(const QString& kwarg) const;
    QString getString(const QString& kwarg) const;
    QStringList getArray(const QString& kwarg) const;

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
    explicit MesonKWARGSTargetInfo(const QString& id);
    virtual ~MesonKWARGSTargetInfo();
};

class MesonKWARGSDependencyInfo : public MesonKWARGSInfo
{
public:
    explicit MesonKWARGSDependencyInfo(const QString& id);
    virtual ~MesonKWARGSDependencyInfo();
};
