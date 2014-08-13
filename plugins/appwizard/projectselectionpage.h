/***************************************************************************
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _PROJECTSELECTIONPAGE_H_
#define _PROJECTSELECTIONPAGE_H_

#include "appwizardpagewidget.h"
#include "appwizarddialog.h"
#include <knewstuff3/entry.h>
#include <kconfiggroup.h>
#include <QRegExpValidator>

namespace KDevelop
{
class MultiLevelListView;
}

namespace Ui {
class ProjectSelectionPage;
}

class ProjectTemplatesModel;
class QModelIndex;
class QStandardItem;
class KUrl;

class ProjectSelectionPage : public AppWizardPageWidget {
Q_OBJECT
public:
    explicit ProjectSelectionPage(ProjectTemplatesModel *templatesModel, AppWizardDialog *wizardDialog);
    ~ProjectSelectionPage();

    bool shouldContinue();

    QString selectedTemplate();
    QString appName();
    QUrl location();
signals:
    void locationChanged( const QUrl& );
    void valid();
    void invalid();
private slots:
    void itemChanged( const QModelIndex& current );
    void urlEdited();
    void validateData();
    void nameChanged();
    void typeChanged(const QModelIndex& idx);
    void templateChanged(int);

    void loadFileClicked();
    void templatesDownloaded (const KNS3::Entry::List& entries);


private:
    inline QByteArray encodedAppName();
    inline QStandardItem* getCurrentItem() const;
    void setCurrentTemplate(const QString& fileName);

    Ui::ProjectSelectionPage *ui;
    KDevelop::MultiLevelListView* m_listView;
    ProjectTemplatesModel *m_templatesModel;
    AppWizardDialog *m_wizardDialog;
};

#endif

