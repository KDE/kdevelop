/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QJsonObject>
#include <memory>

class MesonRewriterActionBase;

using MesonRewriterActionPtr = std::shared_ptr<MesonRewriterActionBase>;

class MesonRewriterActionBase
{
public:
    explicit MesonRewriterActionBase();
    virtual ~MesonRewriterActionBase();

    virtual QJsonObject command() = 0;
    virtual void parseResult(QJsonObject);
};
