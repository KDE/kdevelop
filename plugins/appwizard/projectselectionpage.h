/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _PROJECTSELECTIONPAGE_H_
#define _PROJECTSELECTIONPAGE_H_

#include "appwizardpagewidget.h"

#include <memory>

namespace Ui {
class ProjectSelectionPage;
}

class ProjectTemplatesViewHelper;

class QModelIndex;
class QStandardItem;
class QUrl;

namespace KDevelop {
class ITemplateProvider;
class TemplatesModel;
}

class AppWizardDialog;

class ProjectSelectionPage : public AppWizardPageWidget {
Q_OBJECT
public:
    explicit ProjectSelectionPage(KDevelop::ITemplateProvider& templateProvider, AppWizardDialog* wizardDialog);
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

private:
    inline QByteArray encodedProjectName();
    [[nodiscard]] ProjectTemplatesViewHelper viewHelper();
    inline QStandardItem* currentItem() const;

    /**
     * Select the first template in @a ui->listView.
     *
     * This function is used as a fallback when selecting a more relevant template fails.
     */
    void makeFirstTemplateCurrent();

    const std::unique_ptr<KDevelop::TemplatesModel> m_templatesModel;
    Ui::ProjectSelectionPage *ui;
    AppWizardDialog *m_wizardDialog;
};

#endif
