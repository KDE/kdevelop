/*
* $Id$
*  Copyright (C) 2003 Ian Reinhart Geiser <geiseri@kde.org>
*/
#include <qwhatsthis.h>

#include <qfileinfo.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <kapplication.h>
#include <kregexp.h>

#include "classstore.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kaction.h>
#include <kparts/part.h>
#include <kdialogbase.h>


#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevlanguagesupport.h"
#include "kdevpartcontroller.h"
#include "kdevproject.h"
#include "kdevappfrontend.h"

#include "bashsupport_part.h"
#include "domutil.h"

typedef KGenericFactory<BashSupportPart> BashSupportFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevbashsupport, BashSupportFactory( "kdevbashsupport" ) );

BashSupportPart::BashSupportPart(QObject *parent, const char *name, const QStringList& )
: KDevLanguageSupport ("KDevPart", "kdevpart", parent, name ? name : "BashSupportPart" )
{
	setInstance(BashSupportFactory::instance());
	setXMLFile("kdevbashsupport.rc");

	KAction *action;
	action = new KAction( i18n("&Run"), "exec",Key_F9,this, SLOT(slotRun()),actionCollection(), "build_execute" );

	kdDebug() << "Creating BashSupportPart" << endl;

	connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
		this, SLOT(projectConfigWidget(KDialogBase*)) );
	connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
	connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
	connect( partController(), SIGNAL(savedFile(const QString&)), this, SLOT(savedFile(const QString&)) );

}


BashSupportPart::~BashSupportPart()
{

}


void BashSupportPart::projectConfigWidget(KDialogBase *dlg)
{
//	QVBox *vbox = dlg->addVBoxPage(i18n("Bash"));
//	RubyConfigWidget *w = new RubyConfigWidget(*projectDom(), (QWidget *)vbox, "Bash config widget");
//	connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

void BashSupportPart::projectOpened()
{
	kdDebug(9014) << "projectOpened()" << endl;

	connect( project(), SIGNAL(addedFilesToProject(const QStringList &)),
		this, SLOT(addedFilesToProject(const QStringList &)) );
	connect( project(), SIGNAL(removedFilesFromProject(const QStringList &)),
		this, SLOT(removedFilesFromProject(const QStringList &)) );

	// We want to parse only after all components have been
	// properly initialized
	QTimer::singleShot(0, this, SLOT(parse()));
}


void BashSupportPart::projectClosed()
{
}

void BashSupportPart::slotRun ()
{
	QString file;
	KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(partController()->activePart());
	if(ro_part)
		file = ro_part->url().path();

	QString cmd = interpreter() + " " + file;
	startApplication(cmd);
}

QString BashSupportPart::interpreter()
{
	QString prog = DomUtil::readEntry(*projectDom(), "/kdevrbashsupport/run/interpreter");
	if (prog.isEmpty())
		prog = "bash";
	return prog;
}

void BashSupportPart::parse()
{
	kdDebug(9014) << "initialParse()" << endl;

	if (project())
	{
		kapp->setOverrideCursor(waitCursor);
		QStringList files = project()->allFiles();
		for (QStringList::Iterator it = files.begin(); it != files.end() ;++it)
		{
			kdDebug(9014) << "maybe parse " << project()->projectDirectory() + "/" + (*it) << endl;
			parse(project()->projectDirectory() + "/" + *it);
		}
		emit updatedSourceInfo();
		kapp->restoreOverrideCursor();
	} else {
		kdDebug(9014) << "No project" << endl;
	}
}

void BashSupportPart::addedFilesToProject(const QStringList &fileList)
{
kdDebug(9014) << "addedFilesToProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		parse(project()->projectDirectory() + "/" + ( *it ) );
	}

emit updatedSourceInfo();
}


void BashSupportPart::removedFilesFromProject(const QStringList &fileList)
{
	kdDebug(9014) << "removedFilesFromProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		classStore()->removeWithReferences(project()->projectDirectory() + "/" + ( *it ) );
	}

	emit updatedSourceInfo();
}

void BashSupportPart::savedFile(const QString &fileName)
{
	kdDebug(9014) << "savedFile()" << endl;

	if (project()->allFiles().contains(fileName.mid ( project()->projectDirectory().length() + 1 )))
	{
		parse(fileName);
		emit updatedSourceInfo();
	}
}

void BashSupportPart::startApplication(const QString &program)
{
	kdDebug() << "starting application" << program << endl;
	appFrontend()->startAppCommand(QString::QString(), program, TRUE);
}


KDevLanguageSupport::Features BashSupportPart::features()
{
	return Features(Classes | Functions);
}


QStringList BashSupportPart::fileFilters()
{
	QStringList l;
	l << "*.sh";
	return l;
}

void BashSupportPart::parse(const QString &fileName)
{
	QFileInfo fi(fileName);
	if (fi.extension() == "sh")
	{
		classStore()->removeWithReferences(fileName);

		QFile f(QFile::encodeName(fileName));
		if (!f.open(IO_ReadOnly))
			return;
		QString rawline;
		QString line;
		uint lineNo = 0;
		KRegExp methodre("\\b[\\d\\w]+[\\s]*\\([\\s]*\\)");
		QTextStream stream(&f);
		while (!stream.atEnd())
		{
			 rawline = stream.readLine();
       			 line = rawline.stripWhiteSpace().local8Bit();
			 if (methodre.match(line))
			 {
				ParsedMethod *method = new ParsedMethod;
				method->setName(methodre.group(0));
				method->setDefinedInFile(fileName);
				method->setDefinedOnLine(lineNo);

				ParsedMethod *old = classStore()->globalScope()->getMethod(method);
				kdDebug(9014) << "Add global method " << method->name() << endl;
				if( old )
				{
					delete( method );
					method = old;
				}
				else
				{
					classStore()->globalScope()->addMethod(method);
				}
			}
			++lineNo;
		}
		f.close();
	}
}
#include "bashsupport_part.moc"
