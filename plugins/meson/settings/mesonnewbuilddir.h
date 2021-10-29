/*
    SPDX-FileCopyrightText: 2018 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "mesonconfig.h"

#include <QDialog>

class QDialogButtonBox;

namespace KDevelop
{
class IProject;
}

namespace Ui
{
class MesonNewBuildDir;
}

class MesonNewBuildDir : public QDialog
{
    Q_OBJECT
public:
    explicit MesonNewBuildDir(KDevelop::IProject* project, QWidget* parent = nullptr);
    ~MesonNewBuildDir() override;
    MesonNewBuildDir() = delete;

    void setStatus(const QString& str, bool validConfig);

    bool isConfigValid() const;
    Meson::BuildDir currentConfig() const;
    QStringList mesonArgs() const;

private Q_SLOTS:
    void resetFields();
    void updated();

private:
    bool m_configIsValid = false;
    KDevelop::IProject* m_project = nullptr;
    Ui::MesonNewBuildDir* m_ui = nullptr;
    QString m_oldBuildDir;
};
