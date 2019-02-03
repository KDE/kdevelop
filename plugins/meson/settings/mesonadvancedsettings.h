/* This file is part of KDevelop
    Copyright 2018 Daniel Mensinger <daniel@mensinger-ka.de>

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

#include "util/path.h"

#include <QWidget>

namespace Ui
{
class MesonAdvancedSettings;
}

class MesonAdvancedSettings : public QWidget
{
    Q_OBJECT
public:
    struct Data {
        QString backend;
        QString args;
        KDevelop::Path meson;
    };

public:
    explicit MesonAdvancedSettings(QWidget* parent = nullptr);
    ~MesonAdvancedSettings() override;

    Data getConfig() const;
    void setConfig(Data const& conf);

    void setSupportedBackends(QStringList const& backends);

    bool hasMesonChanged();

Q_SIGNALS:
    void configChanged();

public Q_SLOTS:
    void updated();

private:
    Ui::MesonAdvancedSettings* m_ui = nullptr;
    QStringList m_backendList;
    KDevelop::Path m_mesonOldPath;
};
