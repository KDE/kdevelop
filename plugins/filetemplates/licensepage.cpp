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

#include "licensepage.h"
#include <KConfigCore/ksharedconfig.h>
#include "ui_licensechooser.h"

#include <KLocalizedString>
#include <KDebug>
#include <KStandardDirs>
#include <KEMailSettings>
#include <KComponentData>
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
    typedef QList<LicenseInfo> LicenseList;


    LicensePagePrivate(LicensePage* page_)
    : license(0)
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
    kDebug() << "Searching for available licenses";
    QStringList licenseDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "kdevcodegen/licenses", QStandardPaths::LocateDirectory);

    //Iterate through the possible directories that contain licenses, and load their names
    foreach(const QString& currentDir, licenseDirs)
    {
        QDirIterator it(currentDir, QDir::Files | QDir::Readable);
        while(it.hasNext())
        {
            LicenseInfo newLicense;
            newLicense.path = it.next();
            newLicense.name = it.fileName();

            kDebug() << "Found License: " << newLicense.name;

            availableLicenses.push_back(newLicense);
        }
    }

    qSort(availableLicenses);

    foreach(const LicenseInfo& info, availableLicenses) {
        license->licenseComboBox->addItem(info.name);
    }
    //Finally add the option other for user specified licenses
    LicenseInfo otherLicense;
    availableLicenses.push_back(otherLicense);
    license->licenseComboBox->addItem(i18n("Other"));
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
            kDebug() << "Reading license: " << availableLicenses[licenseIndex].name ;
            QFile newLicense(availableLicenses[licenseIndex].path);

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
    kDebug() << "Attempting to save custom license: " << license->licenseName->text();

    QString localDataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+"/kdevcodegen/licenses/";
    QFile newFile(localDataDir + license->licenseName->text());

    if(newFile.exists())
    {
        KMessageBox::sorry(page, i18n("The specified license already exists. "
                                      "Please provide a different name."));
        return false;
    }

    newFile.open(QIODevice::WriteOnly);
    qint64 result = newFile.write(license->licenseTextEdit->toPlainText().toUtf8());
    newFile.close();

    if(result == -1)
    {
        KMessageBox::sorry(page, i18n("There was an error writing the file."));
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

    connect(d->license->licenseComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(licenseComboChanged(int)));
    connect(d->license->saveLicense, SIGNAL(clicked(bool)),
            d->license->licenseName, SLOT(setEnabled(bool)));

    // Read all the available licenses from the standard dirs
    d->initializeLicenses();

    //Set the license selection to the previous one
    KConfigGroup config(KSharedConfig::openConfig()->group("CodeGeneration"));
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
    KConfigGroup config(KSharedConfig::openConfig()->group("CodeGeneration"));
    //Do not save invalid license numbers'
    int index = d->license->licenseComboBox->currentIndex();
    if( index >= 0 || index < d->availableLicenses.size() )
    {
        config.writeEntry("LastSelectedLicense", index);
        config.config()->sync();
    }
    else
    {
        kWarning() << "Attempted to save an invalid license number: " << index
                   << ". Number of licenses:" << d->availableLicenses.size();
    }

    delete d->license;
    delete d;
}

QString LicensePage::license() const
{
    QString licenseText = d->license->licenseTextEdit->document()->toPlainText();
    /* Add date, name and email to license text */
    licenseText.replace("<year>", QDate::currentDate().toString("yyyy"));
    QString developer("%1 <%2>");
    KEMailSettings emailSettings;
    QString name = emailSettings.getSetting(KEMailSettings::RealName);
    if (name.isEmpty())
    {
        name = "<copyright holder>";
    }
    developer = developer.arg(name);
    QString email = emailSettings.getSetting(KEMailSettings::EmailAddress);
    if (email.isEmpty())
    {
        email = "email"; //no < > as they are already through the email field
    }
    developer = developer.arg(email);
    licenseText.replace("<copyright holder>", developer);

    return licenseText;
}

}

Q_DECLARE_TYPEINFO(KDevelop::LicensePagePrivate::LicenseInfo, Q_MOVABLE_TYPE);

#include "moc_licensepage.cpp"
