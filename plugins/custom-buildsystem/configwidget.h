/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include <QWidget>

#include "custombuildsystemconfig.h"

namespace Ui
{
class ConfigWidget;
}

namespace KDevelop
{
    class IProject;
}

class ConfigWidget : public QWidget
{
Q_OBJECT
public:
    explicit ConfigWidget( QWidget* parent = nullptr );
    void loadConfig(const CustomBuildSystemConfig& cfg);
    CustomBuildSystemConfig config() const;
    void clear();
Q_SIGNALS:
    void changed();
private Q_SLOTS:
    void changeAction( int );
    void toggleActionEnablement( bool );
    void actionArgumentsEdited( const QString& );
    void actionEnvironmentChanged(const QString&);
    void actionExecutableChanged( const QUrl& );
    void actionExecutableChanged( const QString& );
private:
    template<typename F>
    void applyChange(F toolChanger);

    Ui::ConfigWidget* ui;
    QVector<CustomBuildSystemTool> m_tools;
    void setTool( const CustomBuildSystemTool& tool );
};

#endif
