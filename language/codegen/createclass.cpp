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
#include <KEMailSettings>
#include <kdebug.h>
#include <kmessagebox.h>

#include "ui_newclass.h"
#include "ui_licensechooser.h"
#include "ui_outputlocation.h"

#include "duchain/persistentsymboltable.h"
#include "duchain/duchainlock.h"
#include "duchain/duchain.h"
#include "duchain/types/structuretype.h"

#include "codegen/documentchangeset.h"

#include "overridespage.h"
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <kconfig.h>
#include <KSharedConfig>

namespace KDevelop {

class CreateClassWizardPrivate {
public:

   KUrl baseUrl;
    OutputPage* output;
    ClassGenerator * generator;
};


CreateClassWizard::CreateClassWizard(QWidget* parent, ClassGenerator * generator, KUrl baseUrl)
    : QWizard(parent)
    , d(new CreateClassWizardPrivate)
{
    d->baseUrl = baseUrl;
    Q_ASSERT(generator);
    d->generator = generator;
    
    setDefaultProperty("KUrlRequester", "url", SIGNAL(textChanged(QString)));
    setDefaultProperty("KTextEdit", "plainText", SIGNAL(textChanged()));
}

CreateClassWizard::~CreateClassWizard()
{
    delete d;
}

void CreateClassWizard::setup()
{
    setWindowTitle(i18n("Create New Class in %1", d->baseUrl.prettyUrl()));

    if (QWizardPage* page = newIdentifierPage())
        addPage(page);

    if (QWizardPage* page = newOverridesPage())
        addPage(page);

    addPage(new LicensePage(this));
    addPage(d->output = new OutputPage(this));
}

void CreateClassWizard::accept()
{
    QWizard::accept();
    
    //Transmit all the final information to the generator
    d->generator->setLicense(field("license").toString());
    kDebug() << "Header Url: " << field("headerUrl").toString();
    /*
    d->generator->setHeaderUrl(field("headerUrl").toString());
    d->generator->setImplementationUrl(field("implementationUrl").toString());
    d->generator->setHeaderPosition(SimpleCursor(field("headerLine").toInt(), field("headerColumn").toInt()));
    d->generator->setHeaderPosition(SimpleCursor(field("implementationLine").toInt(), field("implementationColumn").toInt()));
    */
    DocumentChangeSet changes = d->generator->generate();
    
    changes.setReplacementPolicy(DocumentChangeSet::WarnOnFailedChange);
    changes.setActivationPolicy(KDevelop::DocumentChangeSet::Activate);
    changes.applyAllChanges();
}

ClassGenerator * CreateClassWizard::generator()
{
    return d->generator;
}

ClassIdentifierPage* CreateClassWizard::newIdentifierPage()
{
    return new ClassIdentifierPage(this);
}

OverridesPage* CreateClassWizard::newOverridesPage()
{
    return new OverridesPage(d->generator, this);
}

struct ClassGeneratorPrivate
{
    QString name; ///< The name for the class to be generated (does not include namespace if relevant)
    QString license;
    QList<DeclarationPointer> inheritedClasses;   ///< Represent *ALL* of the inherited classes
    SimpleCursor headerPosition;
    SimpleCursor implementationPosition;
    
    KUrl headerUrl;
    KUrl implementationUrl;
};

ClassGenerator::ClassGenerator() :
                             d(new ClassGeneratorPrivate)
{
}

ClassGenerator::~ClassGenerator()
{
    delete d;
}

const QString & ClassGenerator::name() const
{
    return d->name;
}

void ClassGenerator::setName(const QString & newName)
{
    d->name = newName;
}

QString ClassGenerator::identifier() const
{
    return name();
}

void ClassGenerator::setIdentifier(const QString & identifier)
{
    setName(identifier);
}

void ClassGenerator::addDeclaration(DeclarationPointer newDeclaration)
{
    m_declarations << newDeclaration;
}

const QList<DeclarationPointer> ClassGenerator::declarations() const
{
    return m_declarations;
}

const QList<DeclarationPointer> & ClassGenerator::addBaseClass(const QString &  newBaseClass)
{
    DUChainReadLocker lock(DUChain::lock());
    
    bool added = false;
    PersistentSymbolTable::Declarations decl = PersistentSymbolTable::self().getDeclarations(IndexedQualifiedIdentifier(QualifiedIdentifier(newBaseClass)));
    
    //Search for all super classes
    for(PersistentSymbolTable::Declarations::Iterator it = decl.iterator(); it; ++it)
    {
        DeclarationPointer declaration = DeclarationPointer(it->declaration());
        if(declaration->isForwardDeclaration())
            continue;

        // Check if it's a class/struct/etc
        if(declaration->type<StructureType>())
        {
            fetchSuperClasses(declaration);
            m_baseClasses << declaration;
            added = true;
            break;
        }
    }
    
    if(!added)
        m_baseClasses << DeclarationPointer(); //Some entities expect that there is always an item added to the list, so just add zero
    
    return m_baseClasses;
}

const QList<DeclarationPointer> & ClassGenerator::inheritanceList() const
{
    return d->inheritedClasses;
}

const QList< DeclarationPointer >& ClassGenerator::directInheritanceList() const
{
    return m_baseClasses;
}

void ClassGenerator::clearInheritance()
{
    m_baseClasses.clear();
    d->inheritedClasses.clear();
}

void ClassGenerator::clearDeclarations()
{
    m_declarations.clear();
}

KUrl ClassGenerator::headerUrlFromBase(KUrl baseUrl, bool toLower)
{
    Q_UNUSED(baseUrl);
    Q_UNUSED(toLower);
    KUrl url;
    url.addPath(d->name);
    return url;
}

KUrl ClassGenerator::implementationUrlFromBase(KUrl baseUrl, bool toLower)
{
    Q_UNUSED(baseUrl);
    Q_UNUSED(toLower);
    return KUrl();
}

void ClassGenerator::setHeaderPosition ( SimpleCursor position )
{
    d->headerPosition = position;
}

void ClassGenerator::setImplementationPosition ( SimpleCursor position )
{
    d->implementationPosition = position;
}

void ClassGenerator::setHeaderUrl ( KUrl header )
{
    d->headerUrl = header;
    kDebug() << "Header for the generated class: " << header;
}

void ClassGenerator::setImplementationUrl ( KUrl implementation )
{
    d->implementationUrl = implementation;
    kDebug() << "Implementation for the generated class: " << implementation;
}

SimpleCursor ClassGenerator::headerPosition()
{
    return d->headerPosition;
}

SimpleCursor ClassGenerator::implementationPosition()
{
    return d->implementationPosition;
}

KUrl & ClassGenerator::headerUrl()
{
    return d->headerUrl;
}

KUrl & ClassGenerator::implementationUrl()
{
    return d->implementationUrl;
}

/// Specify license for this class
void ClassGenerator::setLicense(const QString & license)
{
    kDebug() << "New Class: " << d->name << "Set license: " << d->license;
    d->license = license;
}

/// Get the license specified for this classes
const QString & ClassGenerator::license() const
{
    return d->license;
}

void ClassGenerator::fetchSuperClasses(DeclarationPointer derivedClass)
{
    DUChainReadLocker lock(DUChain::lock());
    
    //Prevent duplicity
    if(d->inheritedClasses.contains(derivedClass))
        return;
    
    d->inheritedClasses.append(derivedClass);

    DUContext* context = derivedClass->internalContext();
    foreach (const DUContext::Import& import, context->importedParentContexts())
        if (DUContext * parentContext = import.context(context->topContext()))
            if (parentContext->type() == DUContext::Class)
                fetchSuperClasses( DeclarationPointer(parentContext->owner()) );
}

class ClassIdentifierPagePrivate
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
    d->classid->addInheritancePushButton->setIcon(KIcon("list-add"));
    d->classid->removeInheritancePushButton->setIcon(KIcon("list-remove"));
    d->classid->moveDownPushButton->setIcon(KIcon("go-down"));
    d->classid->moveUpPushButton->setIcon(KIcon("go-up"));
    
    connect(d->classid->addInheritancePushButton, SIGNAL(pressed()), this, SLOT(addInheritance()));
    connect(d->classid->removeInheritancePushButton, SIGNAL(pressed()), this, SLOT(removeInheritance()));
    connect(d->classid->moveUpPushButton, SIGNAL(pressed()), this, SLOT(moveUpInheritance()));
    connect(d->classid->moveDownPushButton, SIGNAL(pressed()), this, SLOT(moveDownInheritance()));
    connect(d->classid->inheritanceList, SIGNAL(currentRowChanged(int)), this, SLOT(checkMoveButtonState()));

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

    checkMoveButtonState();

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
    d->classid->inheritanceList->setCurrentItem(item);

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
    d->classid->inheritanceList->setCurrentItem(item);

    emit inheritanceChanged();
}

QualifiedIdentifier ClassIdentifierPage::parseParentClassId(const QString& inheritedObject)
{
    return QualifiedIdentifier(inheritedObject);
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

bool ClassIdentifierPage::validatePage ( void )
{
    //save the information in the generator
    ClassGenerator * generator = dynamic_cast<CreateClassWizard *>(wizard())->generator();
    generator->setIdentifier(field("classIdentifier").toString());
    
    //Remove old base classes, and add the new ones
    generator->clearInheritance();
    foreach (const QString & inherited, field("classInheritance").toStringList())
        generator->addBaseClass(inherited);
    
    return true;
}


class LicensePagePrivate
{
public:

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
    KConfigGroup config(KGlobal::config()->group("CodeGeneration"));
    d->license->licenseComboBox->setCurrentIndex(config.readEntry( "LastSelectedLicense", 0 ));
    //Needed to avoid a bug where licenseComboChanged doesn't get called by QComboBox if the past selection was 0
    licenseComboChanged(d->license->licenseComboBox->currentIndex());

    registerField("license", d->license->licenseTextEdit);
}

LicensePage::~LicensePage()
{
    KConfigGroup config(KGlobal::config()->group("CodeGeneration"));
    //Do not save invalid license numbers'
    int index = d->license->licenseComboBox->currentIndex();
    if( index >= 0 || index < d->availableLicenses.size() )
    {
        config.writeEntry("LastSelectedLicense", index);
        config.config()->sync();
    }
    else
        kWarning() << "Attempted to save an invalid license number: " << index << ". Number of licenses:" << d->availableLicenses.size();
    
    delete d;
}

// If the user entered a custom license that they want to save, save it
bool LicensePage::validatePage()
{
    if(d->license->licenseComboBox->currentIndex() == (d->availableLicenses.size() - 1) && 
        d->license->saveLicense->isChecked())
        return saveLicense();
    else
        return true;
}

//! Read all the license files in the global and local config dirs
void LicensePage::initializeLicenses()
{
    kDebug() << "Searching for available licenses";
    KStandardDirs * dirs = KGlobal::dirs();
    QStringList licenseDirs = dirs->findDirs("data", "kdevcodegen/licenses");
    
    //Iterate through the possible directories that contain licenses, and load their names
    foreach(const QString& currentDir, licenseDirs)
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
    if(d->availableLicenses[licenseIndex].contents.isEmpty())
    {
        QString licenseText("");
        //If we are dealing with the last option "other" just return a new empty string
        if(licenseIndex != (d->availableLicenses.size() - 1))
        {
            kDebug() << "Reading license: " << d->availableLicenses[licenseIndex].name ;
            QFile newLicense(d->availableLicenses[licenseIndex].path);
            
            if(newLicense.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QTextStream newLicenseText(&newLicense);
                newLicenseText.setAutoDetectUnicode(true);
                licenseText = newLicenseText.readAll();
                newLicense.close();
            }
            else
                licenseText = "Error, could not open license file.\n Was it deleted?";
        }

        /* Add date, name and email to license text */
        licenseText.replace("<year>", QDate::currentDate().toString("yyyy"));
        QString developer("%1 <%2>");
        KEMailSettings* emailSettings = new KEMailSettings();
        QString name = emailSettings->getSetting(KEMailSettings::RealName);
        if (name.isEmpty())
        {
            name = "<copyright holder>";
        }
        developer = developer.arg(name);
        QString email = emailSettings->getSetting(KEMailSettings::EmailAddress);
        if (email.isEmpty())
        {
            email = "email"; //no < > as they are already through the email field
        }
        developer = developer.arg(email);
        licenseText.replace("<copyright holder>", developer);

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
    
    if(selectedLicense < 0 || selectedLicense >= d->availableLicenses.size())
        d->license->licenseTextEdit->setText(i18n("Could not load previous license"));
    else
        d->license->licenseTextEdit->setText(readLicense(selectedLicense));
}

bool LicensePage::saveLicense()
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

class OutputPagePrivate
{
public:
    OutputPagePrivate()
        : output(0)
    {
    }

    Ui::OutputLocationDialog* output;
    CreateClassWizard* parent;
    
    void updateRanges(KIntNumInput * line, KIntNumInput * column, bool enable);
};

void OutputPagePrivate::updateRanges(KIntNumInput * line, KIntNumInput * column, bool enable)
{
    kDebug() << "Updating Ranges, file exists: " << enable;
    line->setEnabled(enable);
    column->setEnabled(enable);
}

OutputPage::OutputPage(CreateClassWizard* parent)
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
    
    
    connect(d->output->lowerFilenameCheckBox, SIGNAL(stateChanged(int)), this, SLOT(updateFileNames()));
    connect(d->output->headerUrl, SIGNAL(textChanged(const QString &)), this, SLOT(updateHeaderRanges(const QString &)));
    connect(d->output->implementationUrl, SIGNAL(textChanged(const QString &)), this, SLOT(updateImplementationRanges(const QString &)));
    
    registerField("headerUrl", d->output->headerUrl);
    registerField("implementationUrl", d->output->implementationUrl);
}

void OutputPage::initializePage()
{
     //Read the setting for lower case filenames
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup codegenGroup( config, "CodeGeneration" );
    bool lower = codegenGroup.readEntry( "LowerCaseFilenames", true );
    d->parent->d->baseUrl, d->output->lowerFilenameCheckBox->setChecked(lower);

    updateFileNames();
    QWizardPage::initializePage();
}

void OutputPage::updateFileNames() {
    d->output->headerUrl->setUrl(d->parent->generator()->headerUrlFromBase(d->parent->d->baseUrl, d->output->lowerFilenameCheckBox->isChecked()));
    d->output->implementationUrl->setUrl(d->parent->generator()->implementationUrlFromBase(d->parent->d->baseUrl, d->output->lowerFilenameCheckBox->isChecked()));

    //Save the setting for next time
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup codegenGroup( config, "CodeGeneration" );
    codegenGroup.writeEntry( "LowerCaseFilenames", d->output->lowerFilenameCheckBox->isChecked() );
}

void OutputPage::updateHeaderRanges(const QString & url)
{
    QFileInfo info(url);
    d->updateRanges(d->output->headerLineNumber, d->output->headerColumnNumber, info.exists() && !info.isDir());
}

void OutputPage::updateImplementationRanges(const QString & url)
{
    QFileInfo info(url);
    d->updateRanges(d->output->implementationLineNumber, d->output->implementationColumnNumber, info.exists() && !info.isDir());
}

bool OutputPage::isComplete() const
{
    return !d->output->headerUrl->url().url().isEmpty() && !d->output->implementationUrl->url().url().isEmpty();
}

bool OutputPage::validatePage()
{
    d->parent->generator()->setHeaderUrl(d->output->headerUrl->text());
    d->parent->generator()->setImplementationUrl(d->output->implementationUrl->text());
    d->parent->generator()->setHeaderPosition(SimpleCursor(d->output->headerLineNumber->value(), d->output->headerColumnNumber->value()));
    d->parent->generator()->setImplementationPosition(SimpleCursor(d->output->implementationLineNumber->value(), d->output->implementationColumnNumber->value()));
    return true;
}

OutputPage::~OutputPage()
{
    delete d;
}

}

#include "createclass.moc"
