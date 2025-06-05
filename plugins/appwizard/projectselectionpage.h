/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _PROJECTSELECTIONPAGE_H_
#define _PROJECTSELECTIONPAGE_H_

#include "appwizardpagewidget.h"

#include <memory>

class QUrl;

namespace KDevelop {
class ITemplateProvider;
}

class AppWizardDialog;

class ProjectSelectionPagePrivate;

class ProjectSelectionPage : public AppWizardPageWidget {
Q_OBJECT
public:
    explicit ProjectSelectionPage(KDevelop::ITemplateProvider& templateProvider, AppWizardDialog* wizardDialog);
    ~ProjectSelectionPage() override;

    bool shouldContinue() override;

    [[nodiscard]] QString selectedTemplate() const;
    [[nodiscard]] QString projectName() const;
    [[nodiscard]] QUrl location() const;

Q_SIGNALS:
    void locationChanged( const QUrl& );
    void valid();
    void invalid();

private:
    const std::unique_ptr<ProjectSelectionPagePrivate> d_ptr;
    Q_DECLARE_PRIVATE(ProjectSelectionPage)
};

#endif
