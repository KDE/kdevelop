/***************************************************************************
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qdom.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qvaluestack.h>
#include <qregexp.h>
#include <qvbox.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kaction.h>
#include <kgenericfactory.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kmessagebox.h>
#include <klibloader.h>
#include <kservice.h>
#include <kconfig.h>

#include "domutil.h"
#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"
#include "kdevpartcontroller.h"
#include "kdevlanguagesupport.h"
#include "kdevcompileroptions.h"

#include "pascalproject_widget.h"
#include "pascalproject_part.h"
#include "pascalprojectoptionsdlg.h"
#include "pascalglobaloptionsdlg.h"

typedef KGenericFactory<PascalProjectPart> PascalProjectFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevpascalproject, PascalProjectFactory( "kdevpascalproject" ) );

PascalProjectPart::PascalProjectPart(QObject *parent, const char *name, const QStringList& )
    :KDevProject("PascalProject", "pascalproject", parent, name ? name : "PascalProjectPart" )
{
    setInstance(PascalProjectFactory::instance());
    setXMLFile("kdevpascalproject.rc");

    KAction *action;
    action = new KAction( i18n("&Build Project"), "make_kdevelop", Key_F8,
                          this, SLOT(slotBuild()),
                          actionCollection(), "build_build" );
    action = new KAction( i18n("Execute Program"), "exec", 0,
                          this, SLOT(slotExecute()),
                          actionCollection(), "build_execute" );

    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );

    connect( core(), SIGNAL(configWidget(KDialogBase*)),
             this, SLOT(configWidget(KDialogBase*)) );

//  m_widget = new PascalProjectWidget(this);

//  QWhatsThis::add(m_widget, i18n("WHAT DOES THIS PART DO?"));

  // now you decide what should happen to the widget. Take a look at kdevcore.h
  // or at other plugins how to embed it.

  // if you want to embed your widget as an outputview, simply uncomment
  // the following line.

  // mainWindow()->embedOutputView( m_widget, "name that should appear", "enter a tooltip" );

}

PascalProjectPart::~PascalProjectPart()
{
//  delete m_widget;
}

/**
 * This should really be merged with FileTreeWidget::matchesHidePattern()
 * and put in its own class.
 */
static bool matchesPattern(const QString &fileName, const QStringList &patternList)
{
    QStringList::ConstIterator it;
    for (it = patternList.begin(); it != patternList.end(); ++it) {
        QRegExp re(*it, true, true);
        if (re.search(fileName) == 0 && re.matchedLength() == (int)fileName.length())
            return true;
    }

    return false;
}

void PascalProjectPart::openProject(const QString &dirName, const QString &projectName)
{
    m_buildDir = dirName;
    m_projectDir = dirName;
    m_projectName = projectName;


    loadProjectConfig();

    // Put all files from all subdirectories into file list
    QValueStack<QString> s;
    int prefixlen = m_projectDir.length()+1;
    s.push(m_projectDir);

    QStringList includepatternList;
    if ( languageSupport() )
    {
        kdDebug(9033) << languageSupport()->fileFilters().join(",") << endl;
        includepatternList = languageSupport()->fileFilters();
    }
    QString excludepatterns = "*~";
    QStringList excludepatternList = QStringList::split(",", excludepatterns);

    QDir dir;
    do {
        dir.setPath(s.pop());
        kdDebug(9033) << "Examining: " << dir.path() << endl;
        const QFileInfoList *dirEntries = dir.entryInfoList();
        QPtrListIterator<QFileInfo> it(*dirEntries);
        for (; it.current(); ++it) {
            QString fileName = it.current()->fileName();
            if (fileName == "." || fileName == "..")
                continue;
            QString path = it.current()->absFilePath();
            if (it.current()->isDir()) {
                kdDebug(9033) << "Pushing: " << path << endl;
                s.push(path);
            }
            else {
                if (matchesPattern(path, includepatternList)
                    && !matchesPattern(path, excludepatternList)) {
                    kdDebug(9033) << "Adding: " << path << endl;
                    m_sourceFiles.append(path.mid(prefixlen));
                } else {
                    kdDebug(9033) << "Ignoring: " << path << endl;
                }
            }
        }
    } while (!s.isEmpty());
}

void PascalProjectPart::closeProject()
{
}

QString PascalProjectPart::mainProgram()
{
    QFileInfo fi(mainSource());
    return buildDirectory() + "/" + fi.baseName();
}

QString PascalProjectPart::mainSource()
{
    return projectDirectory() + "/" + m_mainSource;
}

void PascalProjectPart::setMainSource(QString fullPath)
{
    m_mainSource = fullPath.remove(QString(projectDirectory() + QString("/")));
}

QString PascalProjectPart::projectDirectory()
{
    return m_projectDir;
}

QString PascalProjectPart::projectName()
{
    return m_projectName;
}

QString PascalProjectPart::activeDirectory()
{
    QFileInfo fi(mainSource());
    return fi.dirPath(true).remove(projectDirectory());
}

QString PascalProjectPart::buildDirectory()
{
    QFileInfo fi(mainSource());
    return fi.dirPath(true);
}

void PascalProjectPart::listOfFiles(QStringList &result, QString path)
{
    QDir d(path);
    if (!d.exists())
        return;
    QFileInfoList *entries = d.entryInfoList(QDir::Dirs | QDir::Files | QDir::Hidden);
    for (QFileInfo *it = entries->first(); it; it = entries->next())
    {
        if ((it->isDir()) && (!it->filePath() == path))
        {
            qWarning("entering dir %s", it->dirPath().latin1());
            listOfFiles(result, it->dirPath());
        }
        else
        {
            qWarning("adding to result: %s", it->filePath().latin1());
            result << it->filePath();
        }
    }
}

QStringList PascalProjectPart::allFiles()
{
//    QStringList files;

//    listOfFiles(files, projectDirectory());

//    return files;
    return m_sourceFiles;
}

void PascalProjectPart::addFile(const QString &fileName)
{
}

void PascalProjectPart::addFiles(const QStringList &fileList)
{
}

void PascalProjectPart::removeFile(const QString &fileName)
{
}

void PascalProjectPart::removeFiles(const QStringList &fileList)
{
}

void PascalProjectPart::slotBuild()
{
    partController()->saveAllFiles();

    QString cmdline = m_compilerExec + " " + m_compilerOpts + " ";

    if (cmdline.isEmpty())
    {
        KMessageBox::sorry(0, i18n("Could not find pascal compiler.\nCheck if your compiler settings are correct."));
        return;
    }

    QFileInfo fi(mainSource());
    cmdline += fi.fileName();

    QString dircmd = "cd ";
    dircmd += buildDirectory();
    dircmd += " && ";

    makeFrontend()->queueCommand(buildDirectory(), dircmd + cmdline);
}

void PascalProjectPart::slotExecute()
{
    partController()->saveAllFiles();
    QString program = "./";
    appFrontend()->startAppCommand(buildDirectory(), mainProgram(), true);
}

void PascalProjectPart::changedFiles( const QStringList & fileList )
{
    KDevProject::changedFiles(fileList);
}

void PascalProjectPart::changedFile( const QString & fileName )
{
    KDevProject::changedFile(fileName);
}

void PascalProjectPart::projectConfigWidget( KDialogBase * dlg )
{
    QVBox *vbox;
    vbox = dlg->addVBoxPage(i18n("Pascal Compiler"));
    PascalProjectOptionsDlg *w = new PascalProjectOptionsDlg(this, vbox);
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
    connect( dlg, SIGNAL(okClicked()), this, SLOT(loadProjectConfig()) );
}

void PascalProjectPart::loadProjectConfig( )
{
    QDomDocument &dom = *(projectDom());

    QString config = DomUtil::readEntry(dom, "/kdevpascalproject/general/useconfiguration", "default");
    m_mainSource = DomUtil::readEntry(dom, QString("/kdevpascalproject/configurations/") + config + QString("/mainsource") );
    m_compilerOpts = DomUtil::readEntry(dom, QString("/kdevpascalproject/configurations/") + config + QString("/compileroptions"));
    m_compilerExec = DomUtil::readEntry(dom, QString("/kdevpascalproject/configurations/") + config + QString("/compilerexec"));

    if (m_compilerExec.isEmpty())
    {
        KTrader::OfferList offers = KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'Pascal'");
        QValueList<KService::Ptr>::ConstIterator it;
        for (it = offers.begin(); it != offers.end(); ++it) {
            if ((*it)->property("X-KDevelop-Default").toBool()) {
                m_compilerExec = (*it)->exec();
                break;
            }
        }
    }
}

void PascalProjectPart::configWidget( KDialogBase * dlg )
{
    QVBox *vbox;
    vbox = dlg->addVBoxPage(i18n("Pascal Compiler"));
    PascalGlobalOptionsDlg *w = new PascalGlobalOptionsDlg(this, vbox);
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

KDevCompilerOptions *PascalProjectPart::createCompilerOptions(const QString &name)
{
    KService::Ptr service = KService::serviceByName(name);
    if (!service) {
        kdDebug() << "Can't find service " << name;
        return 0;
    }

    KLibFactory *factory = KLibLoader::self()->factory(QFile::encodeName(service->library()));
    if (!factory) {
        QString errorMessage = KLibLoader::self()->lastErrorMessage();
        KMessageBox::error(0, i18n("There was an error loading the module %1.\n"
                                   "The diagnostics is:\n%2").arg(service->name()).arg(errorMessage));
        exit(1);
    }

    QStringList args;
    QVariant prop = service->property("X-KDevelop-Args");
    if (prop.isValid())
        args = QStringList::split(" ", prop.toString());

    QObject *obj = factory->create(this, service->name().latin1(),
                                   "KDevCompilerOptions", args);

    if (!obj->inherits("KDevCompilerOptions")) {
        kdDebug() << "Component does not inherit KDevCompilerOptions" << endl;
        return 0;
    }
    KDevCompilerOptions *dlg = (KDevCompilerOptions*) obj;

    return dlg;
}

QString PascalProjectPart::defaultOptions( const QString compiler )
{
    KConfig *config = KGlobal::config();
    config->setGroup("Pascal Compiler");
    return config->readEntry(compiler);
}

#include "pascalproject_part.moc"
