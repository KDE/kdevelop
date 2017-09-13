/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2011 Martin Heide <martin.heide@gmx.net>                *
 *   Copyright (C) 2011 Julien Desgats <julien.desgats@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef QMAKEBUILDDIRCHOOSER_H
#define QMAKEBUILDDIRCHOOSER_H

#include "ui_qmakebuilddirchooser.h"

#include <KConfigGroup>

namespace KDevelop {
    class IProject;
}

/**
 * Handles QMake project configuration on both project creation/import and
 * regular project configuration.
 */
class QMakeBuildDirChooser : public QWidget, public Ui::QMakeBuildDirChooser
{
    Q_OBJECT

public:
    explicit QMakeBuildDirChooser(KDevelop::IProject* project, QWidget *parent = nullptr);
    ~QMakeBuildDirChooser() override;

    KDevelop::IProject* project() const;

public:
    /**
     * Validate user input. Returns true is data is valid.
     * If provided, message is filled with error message, if any.
     * Error message is displayed in dialog anyway.
     */
    bool validate(QString *message=nullptr);

    /**
     * Saves current data to this build dir's config group (not to current values).
     */
    void saveConfig();

    /**
     * Saves current data to the given config group.
     */
    void saveConfig(KConfigGroup& config);

    /**
     * Loads current config (or a new one) into fields.
     */
    void loadConfig();

    /**
     * Loads given config into fields.
     */
    void loadConfig(const QString &config);

    QString errorString() const;

    QString qmakeExecutable() const;
    QString buildDir() const;
    QString installPrefix() const;
    int buildType() const;
    QString extraArgs() const;

    void setQMakeExecutable(const QString& executable);
    void setBuildDir(const QString &buildDir);
    void setInstallPrefix(const QString &prefix);
    void setBuildType(int type);
    void setExtraArgs(const QString &args);

Q_SIGNALS:
    /// Emitted whenever one of the fields of this dialog is changed
    void changed();

private:
    void setErrorString(const QString& errorString);

    KDevelop::IProject* m_project;
};

#endif
