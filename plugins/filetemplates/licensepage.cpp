/*
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "licensepage.h"

#include "ui_licensechooser.h"
#include "debug.h"

#include <KSharedConfig>
#include <KLocalizedString>
#include <KEMailSettings>
#include <KMessageBox>
#include <KConfigGroup>

#include <QDirIterator>
#include <QStandardPaths>

namespace KDevelop {

struct LicensePagePrivate
{
    struct LicenseInfo
    {
        QString name;
        QString path;
        QString contents;
        bool operator< (const LicenseInfo& o) const
        {
            return name.localeAwareCompare(o.name) < 0;
        }
    };
    using LicenseList = QVector<LicenseInfo>;


    explicit LicensePagePrivate(LicensePage* page_)
    : license(nullptr)
    , page(page_)
    {
    }

    // methods
    void initializeLicenses();
    QString readLicense(int licenseIndex);
    bool saveLicense();
    // slots
    void licenseComboChanged(int license);

    Ui::LicenseChooserDialog* license;
    LicenseList availableLicenses;
    LicensePage* page;
};

//! Read all the license files in the global and local config dirs
void LicensePagePrivate::initializeLicenses()
{
    qCDebug(PLUGIN_FILETEMPLATES) << "Searching for available licenses";
    const QStringList licenseDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("kdevcodegen/licenses"), QStandardPaths::LocateDirectory);

    //Iterate through the possible directories that contain licenses, and load their names
    for (const QString& currentDir : licenseDirs) {
        QDirIterator it(currentDir, QDir::Files | QDir::Readable);
        while(it.hasNext())
        {
            LicenseInfo newLicense;
            newLicense.path = it.next();
            newLicense.name = it.fileName();

            qCDebug(PLUGIN_FILETEMPLATES) << "Found License: " << newLicense.name;

            availableLicenses.push_back(newLicense);
        }
    }

    std::sort(availableLicenses.begin(), availableLicenses.end());

    for (const LicenseInfo& info : std::as_const(availableLicenses)) {
        license->licenseComboBox->addItem(info.name);
    }
    //Finally add the option other for user specified licenses
    LicenseInfo otherLicense;
    availableLicenses.push_back(otherLicense);
    license->licenseComboBox->addItem(i18nc("@item:inlistbox other license", "Other"));
}

// Read a license index, if it is not loaded, open it from the file
QString LicensePagePrivate::readLicense(int licenseIndex)
{
    //If the license is not loaded into memory, read it in
    if(availableLicenses[licenseIndex].contents.isEmpty())
    {
        QString licenseText;
        //If we are dealing with the last option "other" just return a new empty string
        if(licenseIndex != (availableLicenses.size() - 1))
        {
            qCDebug(PLUGIN_FILETEMPLATES) << "Reading license: " << availableLicenses[licenseIndex].name ;
            QFile newLicense(availableLicenses[licenseIndex].path);

            if(newLicense.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QTextStream newLicenseText(&newLicense);
                newLicenseText.setAutoDetectUnicode(true);
                licenseText = newLicenseText.readAll();
                newLicense.close();

                // license text files are stored with a trailing linebreak,
                // as otherwise some tools complain about it
                // so deal with that and remove any potential trailing linebreak
                // which otherwise would result in a trailing empty line
                QRegularExpression anyLinebreakAtEnd(QStringLiteral("(\n|\r\n)$"));
                licenseText.remove(anyLinebreakAtEnd);
            }
            else
                licenseText = QStringLiteral("Error, could not open license file.\n Was it deleted?");
        }

        availableLicenses[licenseIndex].contents = licenseText;
    }

    return availableLicenses[licenseIndex].contents;
}

// ---Slots---

void LicensePagePrivate::licenseComboChanged(int selectedLicense)
{
    //If the last slot is selected enable the save license combobox
    if(selectedLicense == (availableLicenses.size() - 1))
    {
        license->licenseTextEdit->clear();
        license->licenseTextEdit->setReadOnly(false);
        license->saveLicense->setEnabled(true);
    }
    else
    {
        license->saveLicense->setEnabled(false);
        license->licenseTextEdit->setReadOnly(true);
    }

    if(selectedLicense < 0 || selectedLicense >= availableLicenses.size())
        license->licenseTextEdit->setText(i18n("Could not load previous license"));
    else
        license->licenseTextEdit->setText(readLicense(selectedLicense));
}

bool LicensePagePrivate::saveLicense()
{
    qCDebug(PLUGIN_FILETEMPLATES) << "Attempting to save custom license: " << license->licenseName->text();

    QString localDataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+QLatin1String("/kdevcodegen/licenses/");
    QString fullPath = localDataDir + license->licenseName->text();
    QFile newFile(fullPath);

    if(newFile.exists())
    {
        KMessageBox::error(page, i18n("The specified license already exists. "
                                      "Please provide a different name."));
        return false;
    }

    QDir().mkpath(localDataDir);
    newFile.open(QIODevice::WriteOnly);
    qint64 result = newFile.write(license->licenseTextEdit->toPlainText().toUtf8());
    newFile.close();

    if(result == -1)
    {
        KMessageBox::error(page, i18n("Failed to write custom license template to file %1.", fullPath));
        return false;
    }

    // also add to our data structures, this esp. needed for proper saving
    // of the license index so it can be restored the next time we show up
    LicenseInfo info;
    info.name = license->licenseName->text();
    info.path = localDataDir;
    availableLicenses << info;
    // find index of the new the license, omitting the very last item ('Other')
    int idx = availableLicenses.count() - 1;
    for(int i = 0; i < availableLicenses.size() - 1; ++i) {
        if (info < availableLicenses.at(i)) {
            idx = i;
            break;
        }
    }
    availableLicenses.insert(idx, info);
    license->licenseComboBox->insertItem(idx, info.name);
    license->licenseComboBox->setCurrentIndex(idx);


    return true;
}

LicensePage::LicensePage(QWidget* parent)
: QWidget(parent)
, d(new LicensePagePrivate(this))
{
    d->license = new Ui::LicenseChooserDialog;
    d->license->setupUi(this);

    connect(d->license->licenseComboBox, QOverload<int>::of(&KComboBox::currentIndexChanged),
            this, [&] (int selectedLicense) { d->licenseComboChanged(selectedLicense); });
    connect(d->license->saveLicense, &QCheckBox::clicked,
            d->license->licenseName, &QLineEdit::setEnabled);

    // Read all the available licenses from the standard dirs
    d->initializeLicenses();

    //Set the license selection to the previous one
    KConfigGroup config(KSharedConfig::openConfig()->group(QStringLiteral("CodeGeneration")));
    d->license->licenseComboBox->setCurrentIndex(config.readEntry( "LastSelectedLicense", 0 ));
    // Needed to avoid a bug where licenseComboChanged doesn't get
    // called by QComboBox if the past selection was 0
    d->licenseComboChanged(d->license->licenseComboBox->currentIndex());
}

LicensePage::~LicensePage()
{
    if (d->license->saveLicense->isChecked() && !d->license->licenseName->text().isEmpty()) {
        d->saveLicense();
    }
    KConfigGroup config(KSharedConfig::openConfig()->group(QStringLiteral("CodeGeneration")));
    //Do not save invalid license numbers'
    int index = d->license->licenseComboBox->currentIndex();
    if( index >= 0 || index < d->availableLicenses.size() )
    {
        config.writeEntry("LastSelectedLicense", index);
        config.config()->sync();
    }
    else
    {
        qCWarning(PLUGIN_FILETEMPLATES) << "Attempted to save an invalid license number: " << index
                                        << ". Number of licenses:" << d->availableLicenses.size();
    }

    delete d->license;
    delete d;
}

QString LicensePage::license() const
{
    QString licenseText = d->license->licenseTextEdit->document()->toPlainText();
    /* Add date, name and email to license text */
    licenseText.replace(QLatin1String("<year>"), QDate::currentDate().toString(QStringLiteral("yyyy")));
    licenseText.replace(QLatin1String("<month>"), QDate::currentDate().toString(QStringLiteral("MM")));
    licenseText.replace(QLatin1String("<day>"), QDate::currentDate().toString(QStringLiteral("dd")));
    QString developer(QStringLiteral("%1 <%2>"));
    KEMailSettings emailSettings;
    QString name = emailSettings.getSetting(KEMailSettings::RealName);
    if (name.isEmpty())
    {
        name = QStringLiteral("<copyright holder>");
    }
    developer = developer.arg(name);
    QString email = emailSettings.getSetting(KEMailSettings::EmailAddress);
    if (email.isEmpty())
    {
        email = QStringLiteral("email"); //no < > as they are already through the email field
    }
    developer = developer.arg(email);
    licenseText.replace(QLatin1String("<copyright holder>"), developer);

    return licenseText;
}

void LicensePage::setFocusToFirstEditWidget()
{
    d->license->licenseComboBox->setFocus();
}

}

Q_DECLARE_TYPEINFO(KDevelop::LicensePagePrivate::LicenseInfo, Q_MOVABLE_TYPE);

#include "moc_licensepage.cpp"
