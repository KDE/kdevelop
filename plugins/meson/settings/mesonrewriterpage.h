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

#include <interfaces/configpage.h>
#include <QAbstractButton>

namespace KDevelop
{
class IPlugin;
class IProject;
}

namespace Ui
{
class MesonRewriterPage;
}

class MesonRewriterPage : public KDevelop::ConfigPage
{
    Q_OBJECT
public:
    enum State {START, LOADING, WRITING, READY, ERROR};

public:
    explicit MesonRewriterPage(KDevelop::IPlugin* plugin, KDevelop::IProject* project, QWidget* parent = nullptr);

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

public Q_SLOTS:
    void apply() override;
    void defaults() override;
    void reset() override;

    void emitChanged();

private:
    void setWidgetsDisabled(bool disabled);
    void checkStatus();
    void setStatus(State s);

private:
    KDevelop::IProject* m_project = nullptr;
    Ui::MesonRewriterPage* m_ui = nullptr;
    bool m_configChanged = false;
    State m_state = START;
};
