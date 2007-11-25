/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2007 by Hamish Rodda                                    *
 *   rodda@kde.org                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "importproject.h"

#include <qcombobox.h>
#include <qdir.h>
#include <qfile.h>
#include <qlabel.h>
#include <klineedit.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kcursor.h>
#include <kfile.h>
#include <kurlrequester.h>
#include <ktemporaryfile.h>

#include <kio/netaccess.h>

#include "icore.h"
#include "iprojectcontroller.h"

#include "appwizardfactory.h"
#include "appwizardplugin.h"


ImportProject::ImportProject(AppWizardPlugin *plugin, QWidget *parent)
    : QDialog(parent)
    , m_plugin(plugin)
{
    setupUi(this);

    directory->setMode(KFile::Directory|KFile::ExistingOnly);

    //connect( fetchModuleButton, SIGNAL(clicked()), SLOT(slotFetchModulesFromRepository()) );
    connect(directory, SIGNAL(urlSelected(const QString& )), SLOT(dirChanged()));
    connect(directory, SIGNAL(returnPressed()), SLOT(dirChanged()));
    directory->installEventFilter(this);
}


ImportProject::~ImportProject()
{}

bool ImportProject::eventFilter ( QObject * watched, QEvent * event )
{
    if (watched == directory)
        if (event->type() == QEvent::FocusOut)
            dirChanged();
    return false;
}

void ImportProject::accept()
{
    KUrl projectUrl = directory->url();

    if (projectUrl.isEmpty() || !projectUrl.isValid() || !KIO::NetAccess::exists(projectUrl, KIO::NetAccess::DestinationSide, QApplication::activeWindow())) {
        KMessageBox::sorry(this, i18n("Please chose an existing directory."));
        return;
    }

    if (projectName->text().isEmpty()) {
        KMessageBox::sorry(this, i18n("Please choose a project name."));
        return;
    }

    projectUrl.setFileName(projectName->text() + ".kdev4");

    if (projectUrl.isLocalFile()) {
        QFile projectFile(projectUrl.toLocalFile());
        if (!projectFile.open(QIODevice::WriteOnly)) {
            KMessageBox::error(this, i18n("Project file already exists."), i18n("Project import error"));
            return;
        }
        importProject(projectFile);

    } else {
        KTemporaryFile tmp;
        if (!tmp.open()) {
            KMessageBox::error(this, i18n("Could not create temporary file to upload."), i18n("Project import error"));
            return;
        }
        importProject(tmp);
    }

    QDialog::accept();

    kDebug(9010) << "OPENING PROJECT: " << projectUrl;

    m_plugin->core()->projectController()->openProject(projectUrl);
}

void ImportProject::importProject(QFile& file)
{
    QTextStream ts(&file);
    ts << "[Project]" << endl;
    ts << "Name=" << projectName->text() << endl;
    ts << "Manager=" << manager() << endl;
}

QString ImportProject::manager() const
{
    switch (buildSystemCombo->currentIndex()) {
        case 0:
            return "KDevCMakeManager";
        case 1:
            return "KDevQMakeManager";
        case 2:
            return "KDevAutotoolsImporter";
        case 3:
            return "KDevCustomMakeManager";
        default:
            return "";
    }
}

// Checks if the directory dir and all of its subdirectories
// (one level recursion) have files that follow patterns
// patterns is comma-separated
static bool dirHasFiles(QDir &dir, const QString &patterns)
{
    QStringList patternList = patterns.split(",");
    if (!dir.entryList(patternList, QDir::Files).isEmpty()) {
        return true;
    }

    QStringList subdirList = dir.entryList(QDir::Dirs);
    foreach (const QString subdirectory, subdirList) {
        QDir subdir(dir);
        subdir.cd(subdirectory);
        if (!subdir.entryList(patternList, QDir::Files).isEmpty())
            return true;
    }

    return false;
}


void ImportProject::dirChanged()
{
    kDebug(9010) << "ImportProject::dirChanged";
    KUrl projectUrl = directory->url();

    if (!projectUrl.isValid() || !KIO::NetAccess::exists(projectUrl, KIO::NetAccess::SourceSide, QApplication::activeWindow()))
        return;

    // Set the project name
    projectName->setText(projectUrl.fileName());

    if (projectUrl.isLocalFile()) {
        QDir dir = projectUrl.path();

        // Automake based?
//         if ( dir.exists("configure.in.in")|| dir.exists("configure.ac")|| dir.exists("configure.in")) {
//             buildSystemCombo->setCurrentIndex(2);
//         }

        // QMake based?
        if (!dir.entryList(QStringList() << "*.pro").isEmpty()) {
            buildSystemCombo->setCurrentIndex(1);
        }

        // CMake based?
        if (!dir.entryList(QStringList() << "CMakeLists.txt").isEmpty()) {
            buildSystemCombo->setCurrentIndex(0);
        }

    }

}

/*
void ImportProject::scanAvailableVCS()
{
//    vcsCombo->insertStringList( m_plugin->registeredVersionControls() );
    int i = 0;
    KTrader::OfferList offers = KTrader::self()->query("KDevelop/VersionControl");
    KTrader::OfferList::const_iterator it = offers.begin();
    while( it != offers.end() )
    {
        vcsCombo->insertItem( (*it)->genericName(), i++ );
        ++it;
    }
}
*/
/*
void ImportProject::slotFinishedCheckout( QString destinationDir )
{
    directory->setURL( destinationDir );

    setCursor( KCursor::arrowCursor() );
//    setEnabled( true );
}
*/
/*
void ImportProject::slotFetchModulesFromRepository()
{

    KDevVersionControl *vcs = m_plugin->versionControlByName( vcsCombo->currentText() );
    if (!vcs)
        return;

    setCursor( KCursor::waitCursor() );
//    setEnabled( false );

    connect( vcs, SIGNAL(finishedFetching(QString)),
        this, SLOT(slotFinishedCheckout(QString)) );

    //restore cursor if we can't fetch repository
    if ( !vcs->fetchFromRepository() )
        setCursor( KCursor::arrowCursor() );

}
*/
