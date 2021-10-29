/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QString>
#include <memory>

class MesonProjectInfo;
class QJsonObject;

using MesonProjectInfoPtr = std::shared_ptr<MesonProjectInfo>;

class MesonProjectInfo
{
public:
    explicit MesonProjectInfo(const QJsonObject& json);
    virtual ~MesonProjectInfo();

    void fromJSON(const QJsonObject& json);

    QString name() const;
    QString version() const;

private:
    QString m_name;
    QString m_version;
};
