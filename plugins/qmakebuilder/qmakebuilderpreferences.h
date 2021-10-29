/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QMAKEBUILDERPREFERENCES_H
#define QMAKEBUILDERPREFERENCES_H

#include <project/projectconfigpage.h>

class QWidget;
class QMakeBuildDirChooser;
namespace Ui {
    class QMakeConfig;
    class QMakeBuildDirChooser;
}

/**
 * @author Andreas Pakulat <apaku@gmx.de>
 */
class QMakeBuilderPreferences : public KDevelop::ConfigPage
{
    Q_OBJECT

public:
    explicit QMakeBuilderPreferences(KDevelop::IPlugin* plugin, const KDevelop::ProjectConfigOptions& options, QWidget* parent = nullptr);
    ~QMakeBuilderPreferences() override;

public Q_SLOTS:
    void apply() override;
    void reset() override;
    QString name() const override;

    void loadOtherConfig(const QString &config);
    void addBuildConfig();
    void removeBuildConfig();
    void validate();

private:
    KDevelop::IProject* m_project;

    Ui::QMakeConfig* m_prefsUi;
    QMakeBuildDirChooser* m_chooserUi;
};

#endif
