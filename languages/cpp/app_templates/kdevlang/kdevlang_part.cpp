%{CPP_TEMPLATE}

#include <qwhatsthis.h>
#include <qtimer.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kdevelop/kdevcore.h>
#include <kdevelop/kdevpartcontroller.h>
#include <kdevelop/kdevproject.h>
#include <kaction.h>
#include <kdebug.h>
#include <kapplication.h>

//#include "%{APPNAMELC}_widget.h"
#include "%{APPNAMELC}_part.h"

typedef KGenericFactory<%{APPNAME}Part> %{APPNAME}Factory;
K_EXPORT_COMPONENT_FACTORY( libkdev%{APPNAMELC}, %{APPNAME}Factory( "kdev%{APPNAMELC}" ) );

%{APPNAME}Part::%{APPNAME}Part(QObject *parent, const char *name, const QStringList& )
: KDevLanguageSupport("KDevPart", "kdevpart", parent, name ? name : "%{APPNAME}Part" )
{
	setInstance(%{APPNAME}Factory::instance());
	setXMLFile("kdevlang_%{APPNAMELC}.rc");


	m_build = new KAction( i18n("&Run"), "exec",Key_F9,this, SLOT(slotRun()),actionCollection(), "build_execute" );

	kdDebug() << "Creating %{APPNAMELC} Part" << endl;

	connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
		this, SLOT(projectConfigWidget(KDialogBase*)) );
	connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
	connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
	connect( partController(), SIGNAL(savedFile(const QString&)), this, SLOT(savedFile(const QString&)) );
	connect(partController(), SIGNAL(activePartChanged(KParts::Part*)),
		this, SLOT(slotActivePartChanged(KParts::Part *)));
}


%{APPNAME}Part::~%{APPNAME}Part()
{
	delete m_build;
}

KDevLanguageSupport::Features %{APPNAME}Part::features()
{
	return Features(Variables | Functions);
}
KMimeType::List %{APPNAME}Part::mimeTypes()
{
    KMimeType::List list;

    KMimeType::Ptr mime = KMimeType::mimeType( "application/x-shellscript" );
    if( mime )
	list << mime;

    return list;
}
void %{APPNAME}Part::slotRun()
{
	// Execute the application here.
}
void %{APPNAME}Part::projectConfigWidget(KDialogBase *dlg)
{
	Q_UNUSED( dlg );
	// Create your config dialog here.
}
void %{APPNAME}Part::projectOpened()
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
void %{APPNAME}Part::projectClosed()
{

}
void %{APPNAME}Part::savedFile(const QString &fileName)
{


	if (project()->allFiles().contains(fileName.mid ( project()->projectDirectory().length() + 1 )))
	{
		kdDebug(9014) << "parse file " << fileName << endl;
		emit addedSourceInfo( fileName );
	}
}
void %{APPNAME}Part::addedFilesToProject(const QStringList &fileList)
{
	kdDebug(9014) << "addedFilesToProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		kdDebug(9014) << "maybe parse " << project()->projectDirectory() + "/" + ( *it ) << endl;
	}

	emit updatedSourceInfo();
}
void %{APPNAME}Part::removedFilesFromProject(const QStringList &fileList)
{


	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		QString fileName = project()->projectDirectory() + "/" + ( *it );
		if( codeModel()->hasFile(fileName) )
		{
			kdDebug(9014) << "removed " << fileName << endl;
			emit aboutToRemoveSourceInfo( fileName );
			codeModel()->removeFile( codeModel()->fileByName(fileName) );
		}
	}

}
void %{APPNAME}Part::parse()
{
	kdDebug(9014) << "initialParse()" << endl;

	if (project())
	{
		kapp->setOverrideCursor(waitCursor);
		QStringList files = project()->allFiles();
		for (QStringList::Iterator it = files.begin(); it != files.end() ;++it)
		{
			kdDebug(9014) << "maybe parse " << project()->projectDirectory() + "/" + (*it) << endl;
		}
		emit updatedSourceInfo();
		kapp->restoreOverrideCursor();
	} else {
		kdDebug(9014) << "No project" << endl;
	}
}
void %{APPNAME}Part::slotActivePartChanged(KParts::Part *part)
{
	kdDebug() << "Changeing active part..." << endl;
}

#include "%{APPNAMELC}_part.moc"
