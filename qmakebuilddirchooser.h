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
class QMakeBuildDirChooser : public Ui::QMakeBuildDirChooser
{
public:
    explicit QMakeBuildDirChooser(QWidget *parent, KDevelop::IProject* project);
    virtual ~QMakeBuildDirChooser();

public:
    /**
     * Validate user input. Returns true is data is valid.
     * If provided, message is filled with error message, if any.
     * Error message is displayed in dialog anyway.
     */
    bool isValid(QString *message=0);

    /**
     * Saves current data to this build dir's config group (not to current values).
     */
    virtual void saveConfig();

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

    QString qmakeBin() const;
    QString buildDir() const;
    QString installPrefix() const;
    int buildType() const;
    QString extraArgs() const;

    void setQmakeBin(const QString &binary);
    void setBuildDir(const QString &buildDir);
    void setInstallPrefix(const QString &prefix);
    void setBuildType(int type);
    void setExtraArgs(const QString &args);

protected:
    KDevelop::IProject* m_project;
};

#endif
