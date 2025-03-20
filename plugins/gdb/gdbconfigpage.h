/*
    SPDX-FileCopyrightText: 2006 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GDBCONFIGPAGE_H
#define GDBCONFIGPAGE_H

#include "midebuglauncher.h"

#include <interfaces/launchconfigurationpage.h>

namespace Ui
{
class GdbConfigPage;
}

class GdbConfigPageFactory : public KDevelop::LaunchConfigurationPageFactory
{
public:
    KDevelop::LaunchConfigurationPage* createWidget(QWidget* parent) override;
};

class GdbConfigPage : public KDevelop::LaunchConfigurationPage
{
Q_OBJECT
public:
    explicit GdbConfigPage( QWidget* parent = nullptr );
    ~GdbConfigPage() override;
    QIcon icon() const override;
    void loadFromConfiguration(const KConfigGroup& cfg, KDevelop::IProject* = nullptr) override;
    void saveToConfiguration(KConfigGroup, KDevelop::IProject* = nullptr ) const override;
    QString title() const override;
private:
    Ui::GdbConfigPage* ui;
};

class GdbLauncher : public KDevMI::MIDebugLauncher
{
public:
    explicit GdbLauncher(KDevMI::MIDebuggerPlugin* plugin, IExecutePlugin* execute);
    QString description() const override;
    QString id() override;
    QString name() const override;
};

#endif
