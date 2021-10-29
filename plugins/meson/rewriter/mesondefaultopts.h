/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "mesonactionbase.h"

class MesonRewriterDefaultOpts;

using MesonDefaultOptsPtr = std::shared_ptr<MesonRewriterDefaultOpts>;

class MesonRewriterDefaultOpts : public MesonRewriterActionBase
{
public:
    enum Action { SET, DELETE };

public:
    explicit MesonRewriterDefaultOpts(Action action);
    virtual ~MesonRewriterDefaultOpts();

    QJsonObject command() override;

    Action action() const;

    void set(const QString& name, const QString& value);
    void clear();

private:
    Action m_action;
    QJsonObject m_data;
};
