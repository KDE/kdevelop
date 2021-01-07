/* This file is part of KDevelop
    Copyright 2018 Daniel Mensinger <daniel@mensinger-ka.de>
    Copyright 2021 BogDan Vatra <bogdan@kde.org>

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

#include "gnconfig.h"

#include <QDialog>

class QDialogButtonBox;

namespace KDevelop
{
class IProject;
}

namespace Ui
{
class GNNewBuildDir;
}

class GNNewBuildDir : public QDialog
{
    Q_OBJECT
public:
    explicit GNNewBuildDir(KDevelop::IProject* project, QWidget* parent = nullptr);
    ~GNNewBuildDir() override;
    GNNewBuildDir() = delete;

    void setStatus(const QString& str, bool validConfig);

    bool isConfigValid() const;
    GN::BuildDir currentConfig() const;
    QStringList gnArgs() const;

private Q_SLOTS:
    void resetFields();
    void updated();

private:
    bool m_configIsValid = false;
    KDevelop::IProject* m_project = nullptr;
    Ui::GNNewBuildDir* m_ui = nullptr;
    QString m_oldBuildDir;
};
