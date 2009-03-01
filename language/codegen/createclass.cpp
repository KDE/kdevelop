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

#include <KListWidget>
#include <KLineEdit>

#include "ui_newclass.h"
#include "ui_licensechooser.h"
#include "ui_outputlocation.h"

#include "overridespage.h"

using namespace KDevelop;

namespace KDevelop {

struct CreateClassPrivate {
    KUrl baseUrl;
    OutputPage* output;
};
}

CreateClass::CreateClass(QWidget* parent, KUrl baseUrl)
    : QWizard(parent)
    , d(new CreateClassPrivate)
{
    d->baseUrl = baseUrl;
    setDefaultProperty("KUrlRequester", "url", SIGNAL(textChanged(QString)));
    setDefaultProperty("KTextEdit", "plainText", SIGNAL(textChanged()));
}

CreateClass::~CreateClass()
{
    delete d;
}

void CreateClass::setup()
{
    setWindowTitle(i18n("Create New Class"));

    if (QWizardPage* page = newIdentifierPage())
        addPage(page);

    if (QWizardPage* page = newOverridesPage())
        addPage(page);

    addPage(new LicensePage(this));
    addPage(d->output = new OutputPage(this));
}

KUrl CreateClass::headerUrlFromBase(QString className, KUrl baseUrl)
{
    Q_UNUSED(baseUrl);
    KUrl url;
    url.addPath(className);
    return url;
}

KUrl CreateClass::implementationUrlFromBase(QString className, KUrl baseUrl)
{
    Q_UNUSED(baseUrl);
    Q_UNUSED(className);
    return KUrl();
}


void CreateClass::accept()
{
    QWizard::accept();

    generate();
}

void CreateClass::done ( int r )
{
    QWizard::done(r);

    //generate();
}

ClassIdentifierPage* CreateClass::newIdentifierPage()
{
    return new ClassIdentifierPage(this);
}

OverridesPage* CreateClass::newOverridesPage()
{
    return new OverridesPage(this);
}

class KDevelop::ClassIdentifierPagePrivate
{
public:
    ClassIdentifierPagePrivate()
        : classid(0)
    {
    }

    Ui::NewClassDialog* classid;
};

ClassIdentifierPage::ClassIdentifierPage(QWizard* parent)
    : QWizardPage(parent)
    , d(new ClassIdentifierPagePrivate)
{
    setTitle(i18n("Class Basics"));
    setSubTitle( i18n("Identify the class and any classes from which it is to inherit.") );

    d->classid = new Ui::NewClassDialog;
    d->classid->setupUi(this);

    connect(d->classid->addInheritancePushButton, SIGNAL(pressed()), this, SLOT(addInheritance()));
    connect(d->classid->removeInheritancePushButton, SIGNAL(pressed()), this, SLOT(removeInheritance()));
    connect(d->classid->moveUpPushButton, SIGNAL(pressed()), this, SLOT(moveUpInheritance()));
    connect(d->classid->moveDownPushButton, SIGNAL(pressed()), this, SLOT(moveDownInheritance()));

    registerField("classIdentifier*", d->classid->identifierLineEdit);
    registerField("classInheritance", this, "inheritance", SIGNAL(inheritanceChanged()));
}

ClassIdentifierPage::~ClassIdentifierPage()
{
    delete d;
}

KLineEdit* ClassIdentifierPage::identifierLineEdit() const
{
    return d->classid->identifierLineEdit;
}

KLineEdit* ClassIdentifierPage::inheritanceLineEdit() const
{
    return d->classid->inheritanceLineEdit;
}

void ClassIdentifierPage::addInheritance()
{
    d->classid->inheritanceList->addItem(d->classid->inheritanceLineEdit->text());

    d->classid->inheritanceLineEdit->clear();

    d->classid->removeInheritancePushButton->setEnabled(true);

    if (d->classid->inheritanceList->count() > 1)
        checkMoveButtonState();

    emit inheritanceChanged();
}

void ClassIdentifierPage::removeInheritance()
{
    delete d->classid->inheritanceList->takeItem(d->classid->inheritanceList->currentRow());

    if (d->classid->inheritanceList->count() == 0)
        d->classid->removeInheritancePushButton->setEnabled(false);

    emit inheritanceChanged();
}

void ClassIdentifierPage::moveUpInheritance()
{
    int currentRow = d->classid->inheritanceList->currentRow();

    Q_ASSERT(currentRow > 0);
    if (currentRow <= 0)
        return;

    QListWidgetItem* item = d->classid->inheritanceList->takeItem(currentRow);
    d->classid->inheritanceList->insertItem(currentRow - 1, item);

    emit inheritanceChanged();
}

void ClassIdentifierPage::moveDownInheritance()
{
    int currentRow = d->classid->inheritanceList->currentRow();

    Q_ASSERT(currentRow != -1 && currentRow < d->classid->inheritanceList->count() - 1);
    if (currentRow == -1 || currentRow >= d->classid->inheritanceList->count() - 1)
        return;

    QListWidgetItem* item = d->classid->inheritanceList->takeItem(currentRow);
    d->classid->inheritanceList->insertItem(currentRow + 1, item);

    emit inheritanceChanged();
}

void ClassIdentifierPage::checkMoveButtonState()
{
    int currentRow = d->classid->inheritanceList->currentRow();
    d->classid->moveUpPushButton->setEnabled(currentRow > 0);
    d->classid->moveDownPushButton->setEnabled(currentRow >= 0 && currentRow < d->classid->inheritanceList->count() - 1);
}

QStringList ClassIdentifierPage::inheritanceList() const
{
    QStringList ret;
    for (int i = 0; i < d->classid->inheritanceList->count(); ++i)
        ret << d->classid->inheritanceList->item(i)->text();

    return ret;
}

class KDevelop::LicensePagePrivate
{
public:
    LicensePagePrivate()
        : license(0)
    {
    }

    Ui::LicenseChooserDialog* license;
};

static const char* s_lgpl =
"   This library is free software; you can redistribute it and/or\n"
"   modify it under the terms of the GNU Library General Public\n"
"   License version 2 as published by the Free Software Foundation.\n\n"
"   This library is distributed in the hope that it will be useful,\n"
"   but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
"   Library General Public License for more details.\n\n"
"   You should have received a copy of the GNU Library General Public License\n"
"   along with this library; see the file COPYING.LIB.  If not, write to\n"
"   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,\n"
"   Boston, MA 02110-1301, USA.\n";

LicensePage::LicensePage(QWizard* parent)
    : QWizardPage(parent)
    , d(new LicensePagePrivate)
{
    setTitle(i18n("License"));
    setSubTitle( i18n("Choose the license under which to place the new class.") );

    d->license = new Ui::LicenseChooserDialog;
    d->license->setupUi(this);

    connect(d->license->licenseComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(licenseComboChanged(int)));

    d->license->licenseTextEdit->setText(s_lgpl);

    registerField("license", d->license->licenseTextEdit);
}

LicensePage::~LicensePage()
{
    delete d;
}

void LicensePage::licenseComboChanged(int license)
{
    //TODO
    Q_UNUSED(license);
}

class KDevelop::OutputPagePrivate
{
public:
    OutputPagePrivate()
        : output(0)
    {
    }

    Ui::OutputLocationDialog* output;
    CreateClass* parent;
};

OutputPage::OutputPage(CreateClass* parent)
    : QWizardPage(parent)
    , d(new OutputPagePrivate)
{
    d->parent = parent;
    setTitle(i18n("Output"));
    setSubTitle( i18n("Choose where to save the new class.") );

    d->output = new Ui::OutputLocationDialog;
    d->output->setupUi(this);

    registerField("headerUrl*", d->output->headerUrl);
    registerField("implementationUrl*", d->output->implementationUrl);
}

void OutputPage::showEvent(QShowEvent*) {
    d->output->headerUrl->setUrl(d->parent->headerUrlFromBase(d->parent->field("classIdentifier").toString(), d->parent->d->baseUrl));
    d->output->implementationUrl->setUrl(d->parent->implementationUrlFromBase(d->parent->field("classIdentifier").toString(), d->parent->d->baseUrl));
}

OutputPage::~OutputPage()
{
    delete d;
}

#include "createclass.moc"
