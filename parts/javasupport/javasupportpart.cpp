#include <qfileinfo.h>
#include <qtimer.h>


#include <kgenericfactory.h>
#include <kapp.h>
#include <kdebug.h>


#include "kdevcore.h"
#include "kdevproject.h"
#include "classstore.h"


#include "javaclassparser.h"
#include "addclass.h"


#include "javasupportpart.h"


typedef KGenericFactory<JavaSupportPart> JavaSupportPartFactory;

K_EXPORT_COMPONENT_FACTORY(libkdevjavasupport, JavaSupportPartFactory("kdevjavasupport"));

JavaSupportPart::JavaSupportPart(QObject *parent, const char *name, const QStringList &)
  : KDevLanguageSupport(parent, name), m_parser(0)
{
  setInstance(JavaSupportPartFactory::instance());

  setXMLFile("javasupportpart.rc");

  connect(core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()));
  connect(core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()));
}


JavaSupportPart::~JavaSupportPart()
{
}


KDevLanguageSupport::Features JavaSupportPart::features()
{
  return KDevLanguageSupport::Features(Classes | NewClass);
}


QStringList JavaSupportPart::fileFilters()
{
  QStringList r;
  r << "*.java";
  return r;
}


void JavaSupportPart::projectOpened()
{
  connect(project(), SIGNAL(addedFileToProject(const QString &)), this, SLOT(addedFileToProject(const QString &)));
  connect(project(), SIGNAL(removedFileFromProject(const QString &)), this, SLOT(removedFileFromProject(const QString &)));
	
  m_parser = new JavaClassParser(classStore());

  QTimer::singleShot(0, this, SLOT(initialParse()));
}


void JavaSupportPart::projectClosed()
{
  delete m_parser;
  m_parser = 0;
}


void JavaSupportPart::initialParse()
{
  kdDebug() << "initialParse()" << endl;

  if (project()) 
  {
    kapp->setOverrideCursor(waitCursor);
   
    // TODO: Progress indicator!

    QStringList files = project()->allFiles();
    for (QStringList::Iterator it = files.begin(); it != files.end() ;++it) 
      maybeParse(*it);
  
    emit updatedSourceInfo();
    
    kapp->restoreOverrideCursor();
  } 
}


void JavaSupportPart::maybeParse(const QString &fileName)
{
  kdDebug() << "Maybe parse: " << fileName << endl;

  QFileInfo fi(fileName);
  if (fi.extension() == "java")
  {
    classStore()->removeWithReferences(fileName);
    parse(fileName);
  }
}


void JavaSupportPart::addedFileToProject(const QString &fileName)
{
  kdDebug() << "ADDED: " << project()->projectDirectory() + "/" + fileName << endl;
  
  maybeParse(project()->projectDirectory() + "/" + fileName);

  emit updatedSourceInfo();
}


void JavaSupportPart::removedFileFromProject(const QString &fileName)
{
  kdDebug() << "REMOVED: " << project()->projectDirectory() + "/" + fileName << endl;
	
  if (m_parser)
    m_parser->removeWithReferences(project()->projectDirectory() + "/" + fileName);

  emit updatedSourceInfo();

  m_parser->removeWithReferences("/home/mhk/platform/Services/TimeStamp/src/org/kde/koala/TestClass.java");
  classStore()->out();
}


void JavaSupportPart::parse(const QString &fileName)
{
  kdDebug() << "PARSE: " << fileName << endl;

  if (m_parser)
    m_parser->parse(fileName);
}


void JavaSupportPart::addClass()
{
  static AddClassInfo info;
  kdDebug() << "New JAVA class" << endl;

  AddClass ac;

  info.className = "org.kde.koala.TestClass";
  info.projectDir = project()->projectDirectory();
  info.sourceDir = "src";

  ac.setInfo(info);
  
  QStringList baseClasses = classStore()->getSortedClassNameList();
  baseClasses.prepend("java.lang.Object");
  ac.setBaseClasses(baseClasses);
  
  if (ac.showDialog())
  {
    info = ac.info();

    if (ac.generate())
      project()->addFile(info.javaFileName());
  }
}


#include "javasupportpart.moc"
