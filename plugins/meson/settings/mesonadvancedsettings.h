/*
    SPDX-FileCopyrightText: 2018 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    void setConfig(const Data& conf);

    void setSupportedBackends(const QStringList& backends);

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
