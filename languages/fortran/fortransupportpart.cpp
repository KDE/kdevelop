/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   The tooltips for ftnchek contained in this source file are taken      *
 *   from the ftnchek man page. ftnchek is written by Robert Moniot and    *
 *   others.                                                               *
 *                                                                         *
 ***************************************************************************/

#include "fortransupportpart.h"
#include "ftnchekconfigwidget.h"
#include "fixedformparser.h"

#include <qdir.h>
#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qvbox.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kregexp.h>
#include <kdevgenericfactory.h>
#include <kaction.h>

#include <kdevcore.h>
#include <kdevproject.h>
#include <kdevmakefrontend.h>
#include <kdevpartcontroller.h>
#include <domutil.h>
#include <codemodel.h>



typedef KDevGenericFactory<FortranSupportPart> FortranSupportFactory;
static const KAboutData data("kdevfortransupport", I18N_NOOP("Language"), "1.0");
K_EXPORT_COMPONENT_FACTORY( libkdevfortransupport, FortranSupportFactory( &data ) )

FortranSupportPart::FortranSupportPart(QObject *parent, const char *name, const QStringList &)
    : KDevLanguageSupport("FortranSupport", "fortran", parent, name ? name : "FortranSupportPart")
{
    setInstance(FortranSupportFactory::instance());

    setXMLFile("kdevfortransupport.rc");

    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );
    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( partController(), SIGNAL(savedFile(const KURL&)),
             this, SLOT(savedFile(const KURL&)) );

    KAction *action;

    action = new KAction( i18n("&Ftnchek"), 0,
                          this, SLOT(slotFtnchek()),
                          actionCollection(), "project_ftnchek" );
    action->setToolTip(i18n("Run ftnchek"));
    action->setWhatsThis(i18n("<b>Run ftnchek</b><p>Runs <b>ftnchek</b> to check fortran programs for semantic errors. Configure ftnchek options in project settings dialog, <b>Ftnchek</b> tab."));

    parser = 0;
}


FortranSupportPart::~FortranSupportPart()
{}


void FortranSupportPart::slotFtnchek()
{
   // Do something smarter here...
    if (makeFrontend()->isRunning()) {
        KMessageBox::sorry(0, i18n("There is currently a job running."));
        return;
    }

    partController()->saveAllFiles();

    QDomDocument &dom = *projectDom();

    QString cmdline = "cd ";
    cmdline += KProcess::quote(project()->projectDirectory());
    cmdline += "&& ftnchek -nonovice ";

    if (DomUtil::readBoolEntry(dom, "/kdevfortransupport/ftnchek/division"))
        cmdline += "-division ";
    if (DomUtil::readBoolEntry(dom, "/kdevfortransupport/ftnchek/extern"))
        cmdline += "-extern ";
    if (DomUtil::readBoolEntry(dom, "/kdevfortransupport/ftnchek/declare"))
        cmdline += "-declare ";
    if (DomUtil::readBoolEntry(dom, "/kdevfortransupport/ftnchek/pure"))
        cmdline += "-pure ";

    cmdline += "-arguments=";
    if (DomUtil::readBoolEntry(dom, "/kdevfortransupport/ftnchek/argumentsall"))
        cmdline += "all ";
    else
        cmdline += DomUtil::readEntry(dom, "/kdevfortransupport/ftnchek/argumentsonly") + " ";

    cmdline += "-common=";
    if (DomUtil::readBoolEntry(dom, "/kdevfortransupport/ftnchek/commonall"))
        cmdline += "all ";
    else
        cmdline += DomUtil::readEntry(dom, "/kdevfortransupport/ftnchek/commononly") + " ";

    cmdline += "-truncation=";
    if (DomUtil::readBoolEntry(dom, "/kdevfortransupport/ftnchek/truncationall"))
        cmdline += "all ";
    else
        cmdline += DomUtil::readEntry(dom, "/kdevfortransupport/ftnchek/truncationonly") + " ";

    cmdline += "-usage=";
    if (DomUtil::readBoolEntry(dom, "/kdevfortransupport/ftnchek/usageall"))
        cmdline += "all ";
    else
        cmdline += DomUtil::readEntry(dom, "/kdevfortransupport/ftnchek/usageonly") + " ";

    cmdline += "-f77=";
    if (DomUtil::readBoolEntry(dom, "/kdevfortransupport/ftnchek/f77all"))
        cmdline += "all ";
    else
        cmdline += DomUtil::readEntry(dom, "/kdevfortransupport/ftnchek/f77only") + " ";

    cmdline += "-portability=";
    if (DomUtil::readBoolEntry(dom, "/kdevfortransupport/ftnchek/portabilityall"))
        cmdline += "all ";
    else
        cmdline += DomUtil::readEntry(dom, "/kdevfortransupport/ftnchek/portabilityonly") + " ";

    QStringList list = project()->allFiles();
    QStringList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        QFileInfo fi(*it);
        QString extension = fi.extension();
        if (extension == "f77" || extension == "f" || extension == "for"
            || extension == "ftn") {
            cmdline += *it + " ";
        }
    }

    makeFrontend()->queueCommand(QString::null, cmdline);
}


void FortranSupportPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox = dlg->addVBoxPage(i18n("Ftnchek"));
    FtnchekConfigWidget *w = new FtnchekConfigWidget(*projectDom(), vbox, "ftnchek config widget");
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}


void FortranSupportPart::projectOpened()
{
    kdDebug(9019) << "projectOpened()" << endl;

    connect( project(), SIGNAL(addedFilesToProject(const QStringList &)),
             this, SLOT(addedFilesToProject(const QStringList &)) );
    connect( project(), SIGNAL(removedFilesFromProject(const QStringList &)),
             this, SLOT(removedFilesFromProject(const QStringList &)) );

    // We want to parse only after all components have been
    // properly initialized
    parser = new FixedFormParser(codeModel());

    QTimer::singleShot(0, this, SLOT(initialParse()));
}


void FortranSupportPart::projectClosed()
{
    delete parser;
    parser = 0;
}


void FortranSupportPart::maybeParse(const QString fileName)
{
    QFileInfo fi(fileName);
    QString extension = fi.extension();
    if (extension == "f77" || extension == "f" || extension == "for" || extension == "ftn") {

        if( codeModel()->hasFile(fileName) ){
            emit aboutToRemoveSourceInfo( fileName );
	    codeModel()->removeFile( codeModel()->fileByName(fileName) );
	}

        parser->parse(fileName);
    }
}


void FortranSupportPart::initialParse()
{
    kdDebug(9019) << "initialParse()" << endl;

    if (project()) {
        kapp->setOverrideCursor(waitCursor);
        QStringList files = project()->allFiles();
        for (QStringList::Iterator it = files.begin(); it != files.end() ;++it) {
	    QFileInfo fileInfo( project()->projectDirectory(), *it );
            kdDebug(9019) << "maybe parse " << fileInfo.absFilePath() << endl;
            maybeParse( fileInfo.absFilePath() );
        }

        emit updatedSourceInfo();
        kapp->restoreOverrideCursor();
    } else {
        kdDebug(9019) << "No project" << endl;
    }
}


void FortranSupportPart::addedFilesToProject(const QStringList &fileList)
{
    kdDebug(9019) << "addedFilesToProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
	        QFileInfo fileInfo( project()->projectDirectory(), *it );
		QString path = fileInfo.absFilePath();
		maybeParse( path );
		emit addedSourceInfo( path );
	}

    //emit updatedSourceInfo();
}


void FortranSupportPart::removedFilesFromProject(const QStringList &fileList)
{
    kdDebug(9019) << "removedFilesFromProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		QFileInfo fileInfo( project()->projectDirectory(), *it );
		QString path = fileInfo.absFilePath();

		if( codeModel()->hasFile(path) ){
		    emit aboutToRemoveSourceInfo( path );
		    codeModel()->removeFile( codeModel()->fileByName(path) );
		}
	}

    //emit updatedSourceInfo();
}


void FortranSupportPart::savedFile(const KURL &fileName)
{
    kdDebug(9019) << "savedFile()" << endl;

    if (project()->allFiles().contains(fileName.path().mid ( project()->projectDirectory().length() + 1 ))) {
        maybeParse(fileName.path());
        emit addedSourceInfo( fileName.path() );
    }
}


KDevLanguageSupport::Features FortranSupportPart::features()
{
    return Features(Functions);
}

#include "fortransupportpart.moc"
