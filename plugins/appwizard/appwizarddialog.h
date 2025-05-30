/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _APPWIZARDDIALOG_H_
#define _APPWIZARDDIALOG_H_

#include <QMap>
#include <QUrl>

#include <KAssistantDialog>

#include <vcs/vcslocation.h>

class KPageWidgetItem;
class ProjectSelectionPage;
class ProjectVcsPage;

namespace KDevelop
{
class IPluginController;
class ITemplateProvider;
}

class ApplicationInfo
{
public:
    QString name;
    QUrl location;
    QString vcsPluginName;
    QUrl sourceLocation;
    KDevelop::VcsLocation repository;
    QString importCommitMessage;
    QString appTemplate;
};

class AppWizardDialog: public KAssistantDialog {
Q_OBJECT
public:
    explicit AppWizardDialog(KDevelop::IPluginController* pluginController,
                             KDevelop::ITemplateProvider& templateProvider, QWidget* parent = nullptr);

    ApplicationInfo appInfo() const;

private Q_SLOTS:
    void pageInValid( QWidget* w );
    void pageValid( QWidget* w );
    void next() override;
private:
    QMap<QWidget*, KPageWidgetItem*> m_pageItems;
    ProjectSelectionPage* m_selectionPage;
    ProjectVcsPage* m_vcsPage;
};

#endif

