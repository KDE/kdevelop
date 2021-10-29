/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PROJECTVCSPAGE_H
#define KDEVPLATFORM_PLUGIN_PROJECTVCSPAGE_H

#include "appwizardpagewidget.h"

namespace Ui
{
class ProjectVcsPage;
}

namespace KDevelop
{
class IPluginController;
class VcsImportMetadataWidget;
class VcsLocation;
}

class QUrl;

class ProjectVcsPage : public AppWizardPageWidget
{
    Q_OBJECT
public:
    explicit ProjectVcsPage( KDevelop::IPluginController*, QWidget* parent = nullptr );
    ~ProjectVcsPage() override;
    bool shouldContinue() override;
Q_SIGNALS:
    void valid();
    void invalid();
public Q_SLOTS:
    void setSourceLocation( const QUrl& );
    void vcsTypeChanged(int);
    void validateData();
public:
    QString pluginName() const;
    QUrl source() const;
    KDevelop::VcsLocation destination() const;
    QString commitMessage() const;
private:
    QList<KDevelop::VcsImportMetadataWidget*> importWidgets;
    KDevelop::VcsImportMetadataWidget* m_currentImportWidget;
    QVector<QPair<QString, QString>> vcsPlugins;
    Ui::ProjectVcsPage* m_ui;
};

#endif
