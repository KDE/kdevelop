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
