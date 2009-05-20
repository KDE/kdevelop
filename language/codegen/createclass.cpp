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

#include <QDirIterator>
#include <QFile>

#include <KListWidget>
#include <KLineEdit>
#include <kdebug.h>
#include <kmessagebox.h>

#include "ui_newclass.h"
#include "ui_licensechooser.h"
#include "ui_outputlocation.h"

#include "overridespage.h"
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <kconfig.h>

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

struct KDevelop::LicensePagePrivate
{
    struct LicenseInfo
    {
        QString name;
        QString path;
        QString contents;
    };
    typedef QList<LicenseInfo> LicenseList;


    LicensePagePrivate()
        : license(0)
    {
    }

    Ui::LicenseChooserDialog* license;
    LicenseList availableLicenses;
};

LicensePage::LicensePage(QWizard* parent)
    : QWizardPage(parent)
    , d(new LicensePagePrivate)
{
    setTitle(i18n("License"));
    setSubTitle( i18n("Choose the license under which to place the new class.") );

    d->license = new Ui::LicenseChooserDialog;
    d->license->setupUi(this);

    connect(d->license->licenseComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(licenseComboChanged(int)));
    connect(d->license->saveLicense, SIGNAL(clicked(bool)), d->license->licenseName, SLOT(setEnabled(bool)));

    // Read all the available licenses from the standard dirs
    initializeLicenses();
    
    //Set the license selection to the previous one
    KConfigGroup config(KSharedConfig::openConfig(), "CodeGeneration");
    d->license->licenseComboBox->setCurrentIndex(config.readEntry( "LastSelectedLicense", 0 ));
    //Needed to avoid a bug where licenseComboChanged doesn't get called by QComboBox if the past selection was 0
    licenseComboChanged(d->license->licenseComboBox->currentIndex());

    registerField("license", d->license->licenseTextEdit);
}

LicensePage::~LicensePage(void)
{
    KConfigGroup config(KSharedConfig::openConfig(), "CodeGeneration");
    config.writeEntry("LastSelectedLicense", d->license->licenseComboBox->currentIndex());
    config.config()->sync();
    
    delete d;
}

// If the user entered a custom license that they want to save, save it
bool LicensePage::validatePage(void)
{
    if(d->license->licenseComboBox->currentIndex() == (d->availableLicenses.size() - 1) && 
        d->license->saveLicense->isChecked())
        return saveLicense();
    else
        return true;
}

//! Read all the license files in the global and local config dirs
void LicensePage::initializeLicenses(void)
{
    kDebug() << "Searching for available licenses";
    KStandardDirs * dirs = KGlobal::dirs();
    QStringList licenseDirs = dirs->findDirs("data", "kdevcodegen/licenses");
    
    //Iterate through the possible directories that contain licenses, and load their names
    foreach(QString currentDir, licenseDirs)
    {
        QDirIterator it(currentDir, QDir::Files | QDir::Readable);
        while(it.hasNext())
        {
            LicensePagePrivate::LicenseInfo newLicense;
            newLicense.path = it.next();
            newLicense.name = it.fileName();
            
            kDebug() << "Found License: " << newLicense.name;
            
            d->availableLicenses.push_back(newLicense);
            d->license->licenseComboBox->addItem(newLicense.name);
        }
    }
    
    //Finally add the option other for user specified licenses
    LicensePagePrivate::LicenseInfo license;
    d->availableLicenses.push_back(license);
    d->license->licenseComboBox->addItem("Other");
}

// Read a license index, if it is not loaded, open it from the file
QString & LicensePage::readLicense(int licenseIndex)
{
    //If the license is not loaded into memory, read it in
    if(d->availableLicenses[licenseIndex].contents.isNull())
    {
        QString licenseText("");
        //If we are dealing with the last option "other" just return a new empty string
        if(licenseIndex != (d->availableLicenses.size() - 1))
        {
            kDebug() << "Reading license: " << d->availableLicenses[licenseIndex].name ;
            QFile newLicense(d->availableLicenses[licenseIndex].path);
            
            if(newLicense.open(QIODevice::ReadOnly))
            {
                licenseText = newLicense.readAll();
                newLicense.close();
            }
            else
                licenseText = "Error, could not open license file.\n Was it deleted?";
        }
        
        d->availableLicenses[licenseIndex].contents = licenseText;
    }
    
    return d->availableLicenses[licenseIndex].contents;
}

// ---Slots---

void LicensePage::licenseComboChanged(int selectedLicense)
{
    //If the last slot is selected enable the save license combobox
    if(selectedLicense == (d->availableLicenses.size() - 1))
    {
        d->license->licenseTextEdit->clear();
        d->license->licenseTextEdit->setReadOnly(false);
        d->license->saveLicense->setEnabled(true);
    }
    else
    {
        d->license->saveLicense->setEnabled(false);
        d->license->licenseTextEdit->setReadOnly(true);
    }
    
    d->license->licenseTextEdit->setText(readLicense(selectedLicense));
}

bool LicensePage::saveLicense(void)
{
    kDebug() << "Attempting to save custom license: " << d->license->licenseName->text();
    
    QString localDataDir = KStandardDirs::locateLocal("data", "kdevcodegen/licenses/", KGlobal::activeComponent());
    QFile newFile(localDataDir + d->license->licenseName->text());
    
    if(newFile.exists())
    {
        KMessageBox::sorry(this, i18n("The specified license already exists. Please provide a different name."));
        return false;
    }
    
    newFile.open(QIODevice::WriteOnly);
    qint64 result = newFile.write(d->license->licenseTextEdit->toPlainText().toUtf8());
    newFile.close();
    
    if(result == -1)
    {
        KMessageBox::sorry(this, i18n("There was an error writing the file."));
        return false;
    }

    return true;
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
    d->output->headerUrl->setMode( KFile::File | KFile::LocalOnly );
    d->output->headerUrl->fileDialog()->setOperationMode( KFileDialog::Saving );
    d->output->implementationUrl->setMode( KFile::File | KFile::LocalOnly );
    d->output->implementationUrl->fileDialog()->setOperationMode( KFileDialog::Saving );

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
