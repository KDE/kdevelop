/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "appwizarddialog.h"

#include <QPushButton>

#include <KLocalizedString>

#include <interfaces/iplugincontroller.h>
#include <vcs/vcslocation.h>

#include "projectselectionpage.h"
#include "projectvcspage.h"

AppWizardDialog::AppWizardDialog(KDevelop::IPluginController* pluginController,
                                 KDevelop::ITemplateProvider& templateProvider, QWidget* parent)
    : KAssistantDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Create New Project"));

    // KAssistantDialog creates a help button by default, no option to prevent that
    QPushButton *helpButton = button(QDialogButtonBox::Help);
    if (helpButton) {
        buttonBox()->removeButton(helpButton);
        delete helpButton;
    }

    m_selectionPage = new ProjectSelectionPage(templateProvider, this);
    m_vcsPage = new ProjectVcsPage( pluginController, this );
    m_vcsPage->setSourceLocation( m_selectionPage->location() );
    connect( m_selectionPage, &ProjectSelectionPage::locationChanged,
             m_vcsPage, &ProjectVcsPage::setSourceLocation );
    m_pageItems[m_selectionPage] = addPage(m_selectionPage, i18nc("@title:tab Page for general configuration options", "General"));

    m_pageItems[m_vcsPage] = addPage(m_vcsPage, i18nc("@title:tab Page for version control options", "Version Control") );

    setValid( m_pageItems[m_selectionPage], false );

    connect(m_selectionPage, &ProjectSelectionPage::invalid, this, [this]() { pageInValid(m_selectionPage); });
    connect(m_vcsPage, &ProjectVcsPage::invalid, this, [this]() { pageInValid(m_vcsPage); });
    connect(m_selectionPage, &ProjectSelectionPage::valid, this, [this]() { pageValid(m_selectionPage); });
    connect(m_vcsPage, &ProjectVcsPage::valid, this, [this]() { pageValid(m_vcsPage); });
}

ApplicationInfo AppWizardDialog::appInfo() const
{
    ApplicationInfo a;
    a.name = m_selectionPage->projectName();
    a.location = m_selectionPage->location();
    a.appTemplate = m_selectionPage->selectedTemplate();
    a.vcsPluginName = m_vcsPage->pluginName();

    if( !m_vcsPage->pluginName().isEmpty() )
    {
        a.repository = m_vcsPage->destination();
        a.sourceLocation = m_vcsPage->source();
        a.importCommitMessage = m_vcsPage->commitMessage();
    }
    else
    {
        a.repository = KDevelop::VcsLocation();
        a.sourceLocation.clear();
        a.importCommitMessage.clear();
    }
    return a;
}


void AppWizardDialog::pageValid( QWidget* w )
{
    const auto pageItemId = m_pageItems.constFind(w);
    if (pageItemId != m_pageItems.constEnd())
        setValid(*pageItemId, true);
}


void AppWizardDialog::pageInValid( QWidget* w )
{
    const auto pageItemId =  m_pageItems.constFind(w);
    if (pageItemId != m_pageItems.constEnd())
        setValid(*pageItemId, false);
}

void AppWizardDialog::next()
{
    auto* w = qobject_cast<AppWizardPageWidget*>(currentPage()->widget());
    if (!w || w->shouldContinue()) {
        KAssistantDialog::next();
    }
}

#include "moc_appwizarddialog.cpp"
