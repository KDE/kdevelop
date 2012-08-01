/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "templateclassassistant.h"
#include "templateselectionpage.h"
#include "templateoptionspage.h"
#include "classmemberspage.h"
#include "templateclassgenerator.h"
#include "sourcefiletemplate.h"
#include "defaultcreateclasshelper.h"
#include "classidentifierpage.h"
#include "overridespage.h"
#include "licensepage.h"
#include "outputpage.h"
#include "documentchangeset.h"

#include "interfaces/icore.h"
#include "interfaces/ilanguagecontroller.h"
#include "interfaces/ilanguagesupport.h"
#include "interfaces/ilanguage.h"
#include "interfaces/idocumentcontroller.h"
#include "interfaces/icreateclasshelper.h"

#include <KLocalizedString>
#include <KConfig>

using namespace KDevelop;

class KDevelop::TemplateClassAssistantPrivate
{
public:
    KPageWidgetItem* templateSelectionPage;
    KPageWidgetItem* templateOptionsPage;
    KPageWidgetItem* membersPage;
    KPageWidgetItem* classIdentifierPage;
    KPageWidgetItem* overridesPage;
    KPageWidgetItem* licensePage;
    KPageWidgetItem* outputPage;

    TemplateSelectionPage* templateSelectionPageWidget;
    TemplateOptionsPage* templateOptionsPageWidget;
    ClassMembersPage* membersPageWidget;
    OverridesPage* overridesPageWidget;
    LicensePage* licensePageWidget;
    OutputPage* outputPageWidget;

    SourceFileTemplate fileTemplate;
    ICreateClassHelper* helper;
    TemplateClassGenerator* generator;
    ClassIdentifierPage* classIdentifierPageWidget;
    KUrl baseUrl;
};

TemplateClassAssistant::TemplateClassAssistant (QWidget* parent, const KUrl& baseUrl)
: KAssistantDialog (parent)
, d(new TemplateClassAssistantPrivate)
{
    d->baseUrl = baseUrl;
    setup();
}

TemplateClassAssistant::~TemplateClassAssistant()
{
    delete d;
}

void TemplateClassAssistant::setup()
{
    setWindowTitle(i18n("Create New Class in %1", baseUrl().prettyUrl()));

    d->templateSelectionPageWidget = new TemplateSelectionPage(this);
    connect (this, SIGNAL(accepted()), d->templateSelectionPageWidget, SLOT(saveConfig()));
    d->templateSelectionPage = addPage(d->templateSelectionPageWidget, i18n("Language and Template"));
    d->templateSelectionPage->setIcon(KIcon("project-development-new-template"));

    d->classIdentifierPageWidget = new ClassIdentifierPage(this);
    d->classIdentifierPage = addPage(d->classIdentifierPageWidget, i18n("Class Basics"));
    d->classIdentifierPage->setIcon(KIcon("classnew"));

    d->overridesPageWidget = new OverridesPage(this);
    d->overridesPage = addPage(d->overridesPageWidget, i18n("Override Methods"));
    d->overridesPage->setIcon(KIcon("code-class"));

    d->membersPageWidget = new ClassMembersPage(this);
    d->membersPage = addPage(d->membersPageWidget, i18n("Class Members"));
    d->membersPage->setIcon(KIcon("field"));

    d->licensePageWidget = new LicensePage(this);
    d->licensePage = addPage(d->licensePageWidget, i18n("License"));
    d->licensePage->setIcon(KIcon("text-x-copying"));

    d->outputPageWidget = new OutputPage(this);
    d->outputPage = addPage(d->outputPageWidget, i18n("Output"));
    d->outputPage->setIcon(KIcon("document-save"));

    QWidgetList pages;
    pages << d->templateSelectionPageWidget
          << d->classIdentifierPageWidget
          << d->overridesPageWidget
          << d->membersPageWidget
          << d->licensePageWidget
          << d->outputPageWidget;

    foreach (QWidget* page, pages)
    {
        connect (page, SIGNAL(isValid(bool)), this, SLOT(setCurrentPageValid(bool)));
    }

    setValid(d->classIdentifierPage, false);
    setValid(d->overridesPage, true);
    setValid(d->licensePage, true);
    setValid(d->outputPage, false);
    showButton(KDialog::Help, false);

    setCurrentPage(d->templateSelectionPage);
}

void TemplateClassAssistant::next()
{
    if (currentPage() == d->templateSelectionPage)
    {
        // We have chosen the template
        // Depending on the template's language, we can now create a helper
        QString description = currentPage()->widget()->property("selectedTemplate").toString();
        d->fileTemplate.setTemplateDescription(description);
        d->helper = 0;

        QString languageName = d->fileTemplate.languageName();
        ILanguage* language = ICore::self()->languageController()->language(languageName);
        if (language && language->languageSupport())
        {
            d->helper = language->languageSupport()->createClassHelper(this);
        }

        if (!d->helper)
        {
            kDebug() << "No class creation helper for language" << languageName;
            d->helper = new DefaultCreateClassHelper(this);
        }

        d->generator = d->helper->generator();
        if (!d->generator)
        {
            kDebug() << "No generator for language" << languageName;
            return;
        }

        d->generator->setTemplateDescription(description);

        if (d->generator->sourceFileTemplate()->hasCustomOptions())
        {
            kDebug() << "Class generator has custom options";
            d->templateOptionsPageWidget = new TemplateOptionsPage(this);
            d->templateOptionsPage = insertPage(d->outputPage, d->templateOptionsPageWidget, i18n("Template Options"));
        }
        else
        {
            d->templateOptionsPage = 0;
            d->templateOptionsPageWidget = 0;
        }

        d->outputPageWidget->prepareForm(d->fileTemplate);
    }
    else if (currentPage() == d->classIdentifierPage)
    {
        d->generator->setIdentifier(d->classIdentifierPageWidget->identifier());
        foreach (const QString& base, d->classIdentifierPageWidget->inheritanceList())
        {
            d->generator->addBaseClass(base);
        }
    }
    else if (currentPage() == d->overridesPage)
    {
        ClassDescription desc = d->generator->description();
        foreach (const DeclarationPointer& declaration, d->overridesPageWidget->selectedOverrides())
        {
            desc.methods << FunctionDescription(declaration);
        }
        d->generator->setDescription(desc);
    }
    else if (currentPage() == d->membersPage)
    {
        ClassDescription desc = d->generator->description();
        desc.members << d->membersPageWidget->members();
        d->generator->setDescription(desc);
    }
    else if (currentPage() == d->licensePage)
    {
        d->generator->setLicense(d->licensePageWidget->license());
    }
    else if (d->templateOptionsPage && (currentPage() == d->templateOptionsPage))
    {
        d->generator->addVariables(d->templateOptionsPageWidget->templateOptions());
    }

    KAssistantDialog::next();

    if (currentPage() == d->membersPage)
    {
        d->membersPage->widget()->setProperty("members", QVariant::fromValue(d->generator->description().members));
    }
    else if (currentPage() == d->overridesPage)
    {
        // TODO: Again separate direct and indirect bases
        d->overridesPageWidget->populateOverrideTree(d->generator->baseClasses(), d->generator->baseClasses());
        d->overridesPageWidget->addCustomDeclarations(i18n("Default"), d->helper->defaultMethods(d->generator->name()));
    }
    else if (d->templateOptionsPage && (currentPage() == d->templateOptionsPage))
    {
        d->templateOptionsPageWidget->load(d->fileTemplate, d->generator->renderer());
    }
    else if (currentPage() == d->outputPage)
    {
        d->outputPageWidget->loadFileTemplate(d->fileTemplate, d->baseUrl, d->generator->renderer());
    }
}

void TemplateClassAssistant::back()
{
    KAssistantDialog::back();
    if (currentPage() == d->templateSelectionPage)
    {
        delete d->helper;
        delete d->generator;
        d->helper = 0;
        d->generator = 0;
    }
}

void TemplateClassAssistant::accept()
{
    DocumentChangeSet changes = d->generator->generate();
    changes.applyAllChanges();

    foreach (const KUrl& url, d->generator->fileUrls())
    {
        ICore::self()->documentController()->openDocument(url);
    }

    KAssistantDialog::accept();
}

void TemplateClassAssistant::setCurrentPageValid (bool valid)
{
    setValid(currentPage(), valid);
}

KUrl TemplateClassAssistant::baseUrl() const
{
    return d->baseUrl;
}
