#include <qwhatsthis.h>
#include <qtimer.h>
#include <qfileinfo.h>


#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kaction.h>
#include <kparts/part.h>
#include <kdialogbase.h>
#include <kapplication.h>
#include <qregexp.h>


#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevlanguagesupport.h"
#include "kdevpartcontroller.h"
#include "kdevproject.h"
#include "kdevappfrontend.h"

#include "rubysupport_part.h"
#include "rubyconfigwidget.h"
#include "domutil.h"

typedef KDevGenericFactory<RubySupportPart> RubySupportFactory;
static const KAboutData data("kdevrubysupport", I18N_NOOP("Language"), "1.0");
K_EXPORT_COMPONENT_FACTORY( libkdevrubysupport, RubySupportFactory( &data ) )

RubySupportPart::RubySupportPart(QObject *parent, const char *name, const QStringList& )
  : KDevLanguageSupport ("KDevPart", "kdevpart", parent, name ? name : "RubySupportPart" )
{
  setInstance(RubySupportFactory::instance());
  setXMLFile("kdevrubysupport.rc");

  KAction *action;
  action = new KAction( i18n("&Run"), "exec",Key_F9,this, SLOT(slotRun()),actionCollection(), "build_execute" );
  action->setToolTip(i18n("Run"));
  action->setWhatsThis(i18n("<b>Run</b><p>Starts an application."));

  kdDebug() << "Creating RubySupportPart" << endl;

  connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
  connect( partController(), SIGNAL(savedFile(const KURL&)),
  	this, SLOT(savedFile(const KURL&)) );
  connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );
}


RubySupportPart::~RubySupportPart() {
}


void RubySupportPart::projectConfigWidget(KDialogBase *dlg) {
    QVBox *vbox = dlg->addVBoxPage(i18n("Ruby"));
    RubyConfigWidget *w = new RubyConfigWidget(*projectDom(), (QWidget *)vbox, "ruby config widget");
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

void RubySupportPart::projectOpened()
{
  kdDebug() << "projectOpened()" << endl;

  connect( project(), SIGNAL(addedFilesToProject(const QStringList &)),
  	this, SLOT(addedFilesToProject(const QStringList &)) );
  connect( project(), SIGNAL(removedFilesFromProject(const QStringList &)),
  	this, SLOT(removedFilesFromProject(const QStringList &)) );

  // We want to parse only after all components have been
  // properly initialized
  QTimer::singleShot(0, this, SLOT(initialParse()));
}

void RubySupportPart::maybeParse(const QString fileName)
{
  QFileInfo fi(fileName);

  if (fi.extension() == "rb") {
    if (codeModel()->hasFile(fileName)) {
      emit aboutToRemoveSourceInfo(fileName);
      codeModel()->removeFile(codeModel()->fileByName(fileName));
    }

    parse(fileName);
  }
}

void RubySupportPart::initialParse()
{
  kdDebug() << "initialParse()" << endl;

  if (project()) {
    kapp->setOverrideCursor(waitCursor);
    QStringList files = project()->allFiles();
    for (QStringList::Iterator it = files.begin(); it != files.end() ;++it) {
      kdDebug() << "maybe parse " << project()->projectDirectory() + "/" + (*it) << endl;
      maybeParse(project()->projectDirectory() + "/" + *it);
    }

    emit updatedSourceInfo();
    kapp->restoreOverrideCursor();
  } else {
    kdDebug() << "No project" << endl;
  }
}

void RubySupportPart::addedFilesToProject(const QStringList &fileList)
{
  kdDebug() << "addedFilesToProject()" << endl;

  QStringList::ConstIterator it;

  for ( it = fileList.begin(); it != fileList.end(); ++it )
  {
    QString fileName = project()->projectDirectory() + "/" + ( *it );
    maybeParse( fileName );
    emit addedSourceInfo( fileName );
  }
}

void RubySupportPart::removedFilesFromProject(const QStringList &fileList)
{
  kdDebug() << "removedFilesFromProject()" << endl;

  QStringList::ConstIterator it;

  for ( it = fileList.begin(); it != fileList.end(); ++it )
  {
    QString fileName = project()->projectDirectory() + "/" + ( *it );

    if( codeModel()->hasFile(fileName) ){
      emit aboutToRemoveSourceInfo( fileName );
      codeModel()->removeFile( codeModel()->fileByName(fileName) );
    }
  }
}

void RubySupportPart::savedFile(const KURL &fileName)
{
  kdDebug() << "savedFile()" << endl;

  if (project()->allFiles().contains(fileName.path().mid ( project()->projectDirectory().length() + 1 ))) {
    maybeParse(fileName.path());
    emit addedSourceInfo( fileName.path() );
  }
}

KDevLanguageSupport::Features RubySupportPart::features()
{
  return Features(Classes | Functions);
}

void RubySupportPart::parse(const QString &fileName)
{
  QFile f(QFile::encodeName(fileName));
  if (!f.open(IO_ReadOnly))
    return;
  QTextStream stream(&f);

  QRegExp classre("^\\s*class\\s+([A-Z][A-Za-z0-9_]+)\\s*(<\\s*([A-Z][A-Za-z0-9_:]+))?$");
  QRegExp methodre("^\\s*def\\s+([A-Za-z0-9_]+).*$");

  FileDom m_file = codeModel()->create<FileModel>();
  m_file->setName(fileName);

  ClassDom lastClass;
  QString rawline;
  QCString line;
  int lineNo = 0;

  while (!stream.atEnd()) {
    rawline = stream.readLine();
    line = rawline.stripWhiteSpace().local8Bit();
    if (classre.search(line) != -1) {
      lastClass = codeModel()->create<ClassModel>();
      lastClass->setName(classre.cap(1));
      lastClass->setFileName( fileName );
      lastClass->setStartPosition( lineNo, 0 );

      QString parent = classre.cap(3);//.stripWhiteSpace()?
      if (!parent.isEmpty())
      {
        kdDebug() << "Add parent " << parent << endl;
        lastClass->addBaseClass( parent );
      }

      if (m_file->hasClass(lastClass->name())) {
        ClassDom old = m_file->classByName( lastClass->name() )[ 0 ];
        old->setFileName( lastClass->fileName() );

        int line, col;
        lastClass->getStartPosition( &line, &col );
        old->setStartPosition( line, col );

        lastClass = old;
      } else {
          kdDebug() << "Add class " << lastClass->name() << endl;
          m_file->addClass( lastClass );
      }
    } else if (methodre.search(line) != -1) {
      FunctionDom method = codeModel()->create<FunctionModel>();
      method->setName(methodre.cap(1));
      kdDebug() << "Add method: " << method->name() << endl;
      method->setFileName( fileName );
      method->setStartPosition( lineNo, 0 );

      if (lastClass && rawline.left(3) != "def") {
        if( !lastClass->hasFunction(method->name()) ) 
          lastClass->addFunction( method );
      } else if( !m_file->hasFunction(method->name()) ){
        m_file->addFunction( method );
        lastClass = 0;
      }
    }
    ++lineNo;
  }

  f.close();

  codeModel()->addFile( m_file );
}


void RubySupportPart::slotRun () {
  QString file;
KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(partController()->activePart());
  if(ro_part) file = ro_part->url().path();
//	file = project()->mainProgram();

  QString cmd = interpreter() + " " + file;
  startApplication(cmd);
}

QString RubySupportPart::interpreter() {
    QString prog = DomUtil::readEntry(*projectDom(), "/kdevrubysupport/run/interpreter");
    if (prog.isEmpty()) prog = "ruby";
    return prog;
}


void RubySupportPart::startApplication(const QString &program) {
		bool inTerminal = DomUtil::readBoolEntry(*projectDom(), "/kdevrubysupport/run/terminal");
    appFrontend()->startAppCommand(project()->projectDirectory(), program, inTerminal);
}


KMimeType::List RubySupportPart::mimeTypes( )
{
    KMimeType::List list;
    KMimeType::Ptr mime = KMimeType::mimeType( "text/x-ruby" );
    if( mime )
	list << mime;
    return list;
}

#include "rubysupport_part.moc"
