/* This file is part of KDevelop
    Copyright 2008 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "createclass.h"
#include "classgenerator.h"
#include "overridespage.h"
#include "documentchangeset.h"

#include <KLocalizedString>
#include <KLineEdit>
#include <KTextEdit>

namespace KDevelop {

struct CreateClassAssistantPrivate
{
    CreateClassAssistantPrivate();

    KUrl baseUrl;
    ClassGenerator * generator;
    ClassIdentifierPage* classIdentifierPageWidget;
    OverridesPage *overridesPageWidget;
    LicensePage *licensePageWidget;
    OutputPage* outputPageWidget;
    KPageWidgetItem* classIdentifierPage;
    KPageWidgetItem* overridesPage;
    KPageWidgetItem* licensePage;
    KPageWidgetItem* outputPage;
};

CreateClassAssistantPrivate::CreateClassAssistantPrivate()
: classIdentifierPage(0)
, overridesPage(0)
, licensePage(0)
, outputPage(0)
{

}


CreateClassAssistant::CreateClassAssistant(QWidget* parent, ClassGenerator * generator, const KUrl& baseUrl)
: KAssistantDialog(parent)
, d(new CreateClassAssistantPrivate)
{
    d->baseUrl = baseUrl;
    Q_ASSERT(generator);
    d->generator = generator;
}

CreateClassAssistant::CreateClassAssistant (QWidget* parent, const KUrl& baseUrl)
: KAssistantDialog (parent)
, d(new CreateClassAssistantPrivate)
{
    d->baseUrl = baseUrl;
    d->generator = 0;
}

CreateClassAssistant::~CreateClassAssistant()
{
    delete d;
}

void CreateClassAssistant::setup()
{
    setWindowTitle(i18n("Create New Class in %1", d->baseUrl.prettyUrl()));

    if (d->classIdentifierPage)
    {
        removePage(d->classIdentifierPage);
    }
    d->classIdentifierPageWidget = newIdentifierPage();
    connect(d->classIdentifierPageWidget, SIGNAL(isValid(bool)), this, SLOT(checkClassIdentifierPage(bool)));
    d->classIdentifierPage = addPage(d->classIdentifierPageWidget, i18n("Class Basics"));
    d->classIdentifierPage->setIcon(KIcon("classnew"));

    if (d->overridesPage)
    {
        removePage(d->overridesPage);
    }
    d->overridesPageWidget = newOverridesPage();
    d->overridesPage = addPage(d->overridesPageWidget, i18n("Override Methods"));
    d->overridesPage->setIcon(KIcon("code-class"));

    if (d->licensePage)
    {
        removePage(d->licensePage);
    }
    d->licensePageWidget = new LicensePage(this);
    d->licensePage = addPage(d->licensePageWidget, i18n("License"));
    d->licensePage->setIcon(KIcon("text-x-copying"));

    if (d->outputPage)
    {
        removePage(d->outputPage);
    }
    d->outputPageWidget = new OutputPage(this);
    connect(d->outputPageWidget, SIGNAL(isValid(bool)), this, SLOT(checkOutputPage(bool)));
    d->outputPage = addPage(d->outputPageWidget, i18n("Output"));
    d->outputPage->setIcon(KIcon("document-save"));

    setValid(d->classIdentifierPage, false);
    setValid(d->overridesPage, true);
    setValid(d->licensePage, true);
    setValid(d->outputPage, false);
    showButton(KDialog::Help, false);

    setCurrentPage(d->classIdentifierPage);
}

void CreateClassAssistant::checkClassIdentifierPage(bool valid)
{
    setValid(d->classIdentifierPage, valid);
}

void CreateClassAssistant::checkOutputPage(bool valid)
{
    setValid(d->outputPage, valid);
}

bool CreateClassAssistant::validateClassIdentifierPage()
{
    generator()->setIdentifier(d->classIdentifierPageWidget->identifierLineEdit()->userText());

    //Remove old base classes, and add the new ones
    generator()->clearInheritance();
    foreach (const QString& inherited, d->classIdentifierPageWidget->inheritanceList()) {
        generator()->addBaseClass(inherited);
    }

    //Update the overrides page with the new inheritance list
    d->overridesPageWidget->updateOverrideTree();

    return true;
}

bool CreateClassAssistant::validateOverridesPage()
{
    d->overridesPageWidget->validateOverrideTree();
    return true;
}

bool CreateClassAssistant::validateLicensePage()
{
    bool valid = d->licensePageWidget->validatePage();

    if (valid) {
        generator()->setLicense(d->licensePageWidget->licenseTextEdit()->toPlainText());
        d->outputPageWidget->initializePage();
    }

    return valid;
}

bool CreateClassAssistant::validateOutputPage()
{
    d->outputPageWidget->validatePage();
    return true;
}

void CreateClassAssistant::next()
{
    bool valid = true;

    if (currentPage() == d->classIdentifierPage) {
        valid = validateClassIdentifierPage();
    } else if (currentPage() == d->overridesPage) {
        valid = validateOverridesPage();
    } else if (currentPage() == d->licensePage) {
        valid = validateLicensePage();
    } else if (currentPage() == d->outputPage) {
        valid = validateOutputPage();
    }

    if (valid) {
        KAssistantDialog::next();
    }
}

void CreateClassAssistant::accept()
{
    //Transmit all the final information to the generator
    validateOutputPage();

    DocumentChangeSet changes = d->generator->generate();
    changes.setReplacementPolicy(DocumentChangeSet::WarnOnFailedChange);
    changes.setActivationPolicy(KDevelop::DocumentChangeSet::Activate);
    changes.applyAllChanges();

    KAssistantDialog::accept();
}

ClassGenerator * CreateClassAssistant::generator()
{
    return d->generator;
}

void CreateClassAssistant::setGenerator (ClassGenerator* generator)
{
    d->generator = generator;
}


ClassIdentifierPage* CreateClassAssistant::newIdentifierPage()
{
    return new ClassIdentifierPage(this);
}

OverridesPage* CreateClassAssistant::newOverridesPage()
{
    return new OverridesPage(generator(), this);
}

KUrl CreateClassAssistant::baseUrl()
{
    return d->baseUrl;
}

}

#include "createclass.moc"
