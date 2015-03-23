/***************************************************************************
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _APPWIZARDDIALOG_H_
#define _APPWIZARDDIALOG_H_

#include <QMap>
#include <QUrl>

#include <KAssistantDialog>

#include <vcs/interfaces/ibasicversioncontrol.h>

class KPageWidgetItem;
class QSignalMapper;
class ProjectSelectionPage;
class ProjectVcsPage;
class ProjectTemplatesModel;

namespace KDevelop
{
class IPluginController;
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
    AppWizardDialog( KDevelop::IPluginController*, ProjectTemplatesModel*,
                     QWidget *parent = 0, Qt::WindowFlags flags = 0);

    ApplicationInfo appInfo() const;

private slots:
    void pageInValid( QWidget* w );
    void pageValid( QWidget* w );
    void next() override;
private:
    QMap<QWidget*, KPageWidgetItem*> m_pageItems;
    QSignalMapper* m_invalidMapper;
    QSignalMapper* m_validMapper;
    ProjectSelectionPage* m_selectionPage;
    ProjectVcsPage* m_vcsPage;
};

#endif

