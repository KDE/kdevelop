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
#include <QSignalMapper>

#include <KListWidget>
#include <KLineEdit>
#include <KEMailSettings>
#include <KDebug>
#include <KMessageBox>

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
#include <kurlrequester.h>
#include <knuminput.h>

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
        removePage(d->licensePage);
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
    bool valid = false;

    if (currentPage() == d->classIdentifierPage) {
        valid = validateClassIdentifierPage();
    } else if (currentPage() == d->overridesPage) {
        valid = validateOverridesPage();
    } else if (currentPage() == d->licensePage) {
        valid = validateLicensePage();
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

struct ClassGeneratorPrivate
{
    QString name; ///< The name for the class to be generated (does not include namespace if relevant)
    QString license;
    QList<DeclarationPointer> inheritedClasses;   ///< Represent *ALL* of the inherited classes

    QMap<QString, KUrl> fileUrls;
    QMap<QString, SimpleCursor> filePositions;
    
    QString headerFileType;
    QString implementationFileType;
};

ClassGenerator::ClassGenerator()
: d(new ClassGeneratorPrivate)
{
    d->headerFileType = i18n("Header");
    d->implementationFileType = i18n("Implementation");
}

ClassGenerator::~ClassGenerator()
{
    delete d;
}

QString ClassGenerator::name() const
{
    return d->name;
}

void ClassGenerator::setName(const QString& newName)
{
    d->name = newName;
}

QString ClassGenerator::identifier() const
{
    return name();
}

void ClassGenerator::setIdentifier(const QString& identifier)
{
    setName(identifier);
}

void ClassGenerator::addDeclaration(const DeclarationPointer& newDeclaration)
{
    m_declarations << newDeclaration;
}

QList<DeclarationPointer> ClassGenerator::declarations() const
{
    return m_declarations;
}

QList<DeclarationPointer> ClassGenerator::addBaseClass(const QString&  newBaseClass)
{
    DUChainReadLocker lock;

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

QList<DeclarationPointer> ClassGenerator::inheritanceList() const
{
    return d->inheritedClasses;
}

QList<DeclarationPointer> ClassGenerator::directInheritanceList() const
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

QMap< QString, KUrl > ClassGenerator::fileUrlsFromBase (const KUrl& baseUrl, bool toLower)
{
    QMap<QString, KUrl> map;
    map.insert(d->headerFileType, headerUrlFromBase(baseUrl, toLower));
    map.insert(d->implementationFileType, implementationUrlFromBase(baseUrl, toLower));
    return map;
}

KUrl ClassGenerator::headerUrlFromBase(const KUrl& baseUrl, bool toLower)
{
    Q_UNUSED(baseUrl);
    Q_UNUSED(toLower);
    KUrl url;
    url.addPath(d->name);
    return url;
}

KUrl ClassGenerator::implementationUrlFromBase(const KUrl& baseUrl, bool toLower)
{
    Q_UNUSED(baseUrl);
    Q_UNUSED(toLower);
    return KUrl();
}

void ClassGenerator::setFilePosition (const QString& fileType, const SimpleCursor& position)
{
    d->filePositions[fileType] = position;
}


void ClassGenerator::setHeaderPosition(const SimpleCursor& position)
{
    setFilePosition(d->headerFileType, position);
}

void ClassGenerator::setImplementationPosition(const SimpleCursor& position)
{
    setFilePosition(d->implementationFileType, position);
}

void ClassGenerator::setFileUrl (const QString& fileType, const KUrl url)
{
    d->fileUrls[fileType] = url;
}

void ClassGenerator::setHeaderUrl(const KUrl& header)
{
    kDebug() << "Header for the generated class: " << header;
    setFileUrl(d->headerFileType, header);
}

void ClassGenerator::setImplementationUrl(const KUrl& implementation)
{
    kDebug() << "Implementation for the generated class: " << implementation;
    setFileUrl(d->implementationFileType, implementation);
}

SimpleCursor ClassGenerator::headerPosition()
{
    return d->filePositions[d->headerFileType];
}

SimpleCursor ClassGenerator::implementationPosition()
{
    return d->filePositions[d->implementationFileType];
}

KUrl ClassGenerator::headerUrl()
{
    return d->fileUrls[d->headerFileType];
}

KUrl ClassGenerator::implementationUrl()
{
    return d->fileUrls[d->implementationFileType];
}

/// Specify license for this class
void ClassGenerator::setLicense(const QString& license)
{
    kDebug() << "New Class: " << d->name << "Set license: " << d->license;
    d->license = license;
}

/// Get the license specified for this classes
QString ClassGenerator::license() const
{
    return d->license;
}

void ClassGenerator::fetchSuperClasses(const DeclarationPointer& derivedClass)
{
    DUChainReadLocker lock;

    //Prevent duplicity
    if(d->inheritedClasses.contains(derivedClass)) {
        return;
    }

    d->inheritedClasses.append(derivedClass);

    DUContext* context = derivedClass->internalContext();
    if (context) {
        foreach (const DUContext::Import& import, context->importedParentContexts()) {
            if (DUContext * parentContext = import.context(context->topContext())) {
                if (parentContext->type() == DUContext::Class) {
                    fetchSuperClasses( DeclarationPointer(parentContext->owner()) );
                }
            }
        }
    }
}

struct ClassIdentifierPagePrivate
{
    ClassIdentifierPagePrivate()
        : classid(0)
    {
    }

    Ui::NewClassDialog* classid;
};

ClassIdentifierPage::ClassIdentifierPage(QWidget* parent)
: QWidget(parent)
, d(new ClassIdentifierPagePrivate())
{
    d->classid = new Ui::NewClassDialog;
    d->classid->setupUi(this);
    d->classid->addInheritancePushButton->setIcon(KIcon("list-add"));
    d->classid->removeInheritancePushButton->setIcon(KIcon("list-remove"));
    d->classid->moveDownPushButton->setIcon(KIcon("go-down"));
    d->classid->moveUpPushButton->setIcon(KIcon("go-up"));

    connect(d->classid->identifierLineEdit, SIGNAL(textChanged(QString)), this, SLOT(checkIdentifier()));
    connect(d->classid->addInheritancePushButton, SIGNAL(pressed()), this, SLOT(addInheritance()));
    connect(d->classid->removeInheritancePushButton, SIGNAL(pressed()), this, SLOT(removeInheritance()));
    connect(d->classid->moveUpPushButton, SIGNAL(pressed()), this, SLOT(moveUpInheritance()));
    connect(d->classid->moveDownPushButton, SIGNAL(pressed()), this, SLOT(moveDownInheritance()));
    connect(d->classid->inheritanceList, SIGNAL(currentRowChanged(int)), this, SLOT(checkMoveButtonState()));

    emit isValid(false);
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

void ClassIdentifierPage::checkIdentifier()
{
    emit isValid(!identifierLineEdit()->userText().isEmpty());
}

void ClassIdentifierPage::addInheritance()
{
    d->classid->inheritanceList->addItem(d->classid->inheritanceLineEdit->text());

    d->classid->inheritanceLineEdit->clear();

    d->classid->removeInheritancePushButton->setEnabled(true);

    if (d->classid->inheritanceList->count() > 1) {
        checkMoveButtonState();
    }

    emit inheritanceChanged();
}

void ClassIdentifierPage::removeInheritance()
{
    delete d->classid->inheritanceList->takeItem(d->classid->inheritanceList->currentRow());

    if (d->classid->inheritanceList->count() == 0) {
        d->classid->removeInheritancePushButton->setEnabled(false);
    }

    checkMoveButtonState();

    emit inheritanceChanged();
}

void ClassIdentifierPage::moveUpInheritance()
{
    int currentRow = d->classid->inheritanceList->currentRow();

    Q_ASSERT(currentRow > 0);
    if (currentRow <= 0) {
        return;
    }

    QListWidgetItem* item = d->classid->inheritanceList->takeItem(currentRow);
    d->classid->inheritanceList->insertItem(currentRow - 1, item);
    d->classid->inheritanceList->setCurrentItem(item);

    emit inheritanceChanged();
}

void ClassIdentifierPage::moveDownInheritance()
{
    int currentRow = d->classid->inheritanceList->currentRow();

    Q_ASSERT(currentRow != -1 && currentRow < d->classid->inheritanceList->count() - 1);
    if (currentRow == -1 || currentRow >= d->classid->inheritanceList->count() - 1) {
        return;
    }

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

struct LicensePagePrivate
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

LicensePage::LicensePage(QWidget* parent)
: QWidget(parent)
, d(new LicensePagePrivate)
{
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

KTextEdit* LicensePage::licenseTextEdit()
{
    return d->license->licenseTextEdit;
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
QString& LicensePage::readLicense(int licenseIndex)
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

struct OutputPagePrivate
{
    OutputPagePrivate()
        : output(0)
    {
    }

    Ui::OutputLocationDialog* output;
    CreateClassAssistant* parent;
    QSignalMapper urlChangedMapper;
    QMap<QString, KUrlRequester*> outputFiles;
    QMap<QString, KIntNumInput*> outputLines;
    QMap<QString, KIntNumInput*> outputColumns;

    void updateRanges(KIntNumInput * line, KIntNumInput * column, bool enable);
};

void OutputPagePrivate::updateRanges(KIntNumInput * line, KIntNumInput * column, bool enable)
{
    kDebug() << "Updating Ranges, file exists: " << enable;
    line->setEnabled(enable);
    column->setEnabled(enable);
}

OutputPage::OutputPage(CreateClassAssistant* parent)
: QWidget(parent)
, d(new OutputPagePrivate)
{
    d->parent = parent;

    d->output = new Ui::OutputLocationDialog;
    d->output->setupUi(this);
    
    QMap<QString, KUrl> urls = parent->generator()->fileUrlsFromBase(parent->baseUrl(), d->output->lowerFilenameCheckBox->isChecked());
    for (QMap<QString, KUrl>::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it)
    {
        QLabel* label = new QLabel(it.key(), this);
        KUrlRequester* requester = new KUrlRequester(it.value(), this);
        requester->setMode( KFile::File | KFile::LocalOnly );
        requester->fileDialog()->setOperationMode( KFileDialog::Saving );
        
        d->urlChangedMapper.setMapping(requester, it.key());
        connect(requester, SIGNAL(textChanged(QString)), &d->urlChangedMapper, SLOT(map()));
        
        d->output->urlFormLayout->addRow(label, requester);
        d->outputFiles.insert(it.key(), requester);
        
        label = new QLabel(it.key(), this);
        QHBoxLayout* layout = new QHBoxLayout(this);
        
        KIntNumInput* line = new KIntNumInput(this);
        line->setPrefix(i18n("Line: "));
        line->setValue(0);
        line->setMinimum(0);
        layout->addWidget(line);
       
        KIntNumInput* column = new KIntNumInput(this);
        column->setPrefix(i18n("Column: "));
        column->setValue(0);
        column->setMinimum(0);
        layout->addWidget(column);
        
        d->output->positionFormLayout->addRow(label, layout);
        d->outputLines.insert(it.key(), line);
        d->outputColumns.insert(it.key(), column);
    }
    
    connect(&d->urlChangedMapper, SIGNAL(mapped(QString)), SLOT(updateFileRange(QString)));
    connect(d->output->lowerFilenameCheckBox, SIGNAL(stateChanged(int)), this, SLOT(updateFileNames()));
}

void OutputPage::initializePage()
{
     //Read the setting for lower case filenames
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup codegenGroup( config, "CodeGeneration" );
    bool lower = codegenGroup.readEntry( "LowerCaseFilenames", true );
    d->output->lowerFilenameCheckBox->setChecked(lower);

    updateFileNames();
}

void OutputPage::updateFileNames()
{
    QMap<QString, KUrl> urls = d->parent->generator()->fileUrlsFromBase(d->parent->baseUrl(), d->output->lowerFilenameCheckBox->isChecked());
    
    for (QMap<QString, KUrlRequester*>::const_iterator it = d->outputFiles.constBegin(); it != d->outputFiles.constEnd(); ++it)
    {
        if (urls.contains(it.key()))
        {
            it.value()->setUrl(urls[it.key()]);
        }
    }

    //Save the setting for next time
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup codegenGroup( config, "CodeGeneration" );
    codegenGroup.writeEntry( "LowerCaseFilenames", d->output->lowerFilenameCheckBox->isChecked() );

    emit isValid(isComplete());
}

void OutputPage::updateFileRange (const QString& field)
{
    if (!d->outputFiles.contains(field))
    {
        return;
    }
    
    QString url = d->outputFiles[field]->url().toLocalFile();
    QFileInfo info(url);
    
    d->updateRanges(d->outputLines[field], d->outputColumns[field], info.exists() && !info.isDir());
    
    emit isValid(isComplete());
}

bool OutputPage::isComplete() const
{
    foreach (KUrlRequester* requester, d->outputFiles)
    {
        if (!requester->url().isValid())
        {
            return false;
        }
    }
    return true;
}

bool OutputPage::validatePage()
{
    for (QMap<QString,KUrlRequester*>::const_iterator it = d->outputFiles.constBegin(); it != d->outputFiles.constEnd(); ++it)
    {
        d->parent->generator()->setFileUrl(it.key(), it.value()->url());
        
        int line = d->outputLines[it.key()]->value();
        int column = d->outputColumns[it.key()]->value();
        d->parent->generator()->setFilePosition(it.key(), SimpleCursor(line, column));
    }
    return true;
}

OutputPage::~OutputPage()
{
    delete d;
}

}

Q_DECLARE_TYPEINFO(KDevelop::LicensePagePrivate::LicenseInfo, Q_MOVABLE_TYPE);

#include "createclass.moc"
