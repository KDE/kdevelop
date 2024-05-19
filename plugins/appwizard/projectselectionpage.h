/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _PROJECTSELECTIONPAGE_H_
#define _PROJECTSELECTIONPAGE_H_

#include "appwizardpagewidget.h"
#include "appwizarddialog.h"

namespace Ui {
class ProjectSelectionPage;
}

class ProjectTemplatesModel;

namespace KNSCore {
class Entry;
}

class QModelIndex;
class QStandardItem;
class QUrl;

class ProjectSelectionPage : public AppWizardPageWidget {
Q_OBJECT
public:
    explicit ProjectSelectionPage(ProjectTemplatesModel *templatesModel, AppWizardDialog *wizardDialog);
    ~ProjectSelectionPage() override;

    bool shouldContinue() override;

    QString selectedTemplate();
    QString projectName();
    QUrl location();
Q_SIGNALS:
    void locationChanged( const QUrl& );
    void valid();
    void invalid();
private Q_SLOTS:
    void itemChanged( const QModelIndex& current );
    void urlEdited();
    void validateData();
    void nameChanged();
    void typeChanged(const QModelIndex& idx);
    void templateChanged(int);

    void loadFileClicked();
    void handleNewStuffDialogFinished(const QList<KNSCore::Entry>& changedEntries);

private:
    inline QByteArray encodedProjectName();
    inline QStandardItem* currentItem() const;
    void setCurrentTemplate(const QString& fileName);

    Ui::ProjectSelectionPage *ui;
    ProjectTemplatesModel *m_templatesModel;
    AppWizardDialog *m_wizardDialog;
};

#endif
