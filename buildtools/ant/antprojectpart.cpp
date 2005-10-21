#include "antprojectpart.h"

#include <qapplication.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qvbox.h>
#include <qtable.h>
#include <qtextstream.h>
#include <qvaluestack.h>
#include <qdir.h>


#include <kdevgenericfactory.h>
#include <kdebug.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <kdialogbase.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <keditlistbox.h>
#include <kurlrequester.h>

#include <kdevplugininfo.h>

#include <kdevcore.h>
#include <kdevmakefrontend.h>
#include <urlutil.h>


#include "antoptionswidget.h"
#include "classpathwidget.h"




typedef KDevGenericFactory<AntProjectPart> AntProjectFactory;
static const KDevPluginInfo data("kdevantproject");
K_EXPORT_COMPONENT_FACTORY(libkdevantproject, AntProjectFactory( data ))


AntOptions::AntOptions()
  : m_buildXML("build.xml"), m_verbosity(AntOptions::Quiet)
{
}


AntProjectPart::AntProjectPart(QObject *parent, const char *name, const QStringList &)
  : KDevBuildTool(&data, parent, name ? name : "AntProjectPart")
{
  setInstance(AntProjectFactory::instance());

  setXMLFile("kdevantproject.rc");

  m_buildProjectAction = new KAction(i18n("&Build Project"), "make_kdevelop", Key_F8,
		                     this, SLOT(slotBuild()),
				     actionCollection(), "build_build" );
  m_buildProjectAction->setToolTip(i18n("Build project"));
  m_buildProjectAction->setWhatsThis(i18n("<b>Build project</b><p>Executes <b>ant dist</b> command to build the project."));

  KActionMenu *menu = new KActionMenu(i18n("Build &Target"),
                                      actionCollection(), "build_target" );
  menu->setToolTip(i18n("Build target"));
  menu->setWhatsThis(i18n("<b>Build target</b><p>Executes <b>ant target_name</b> command to build the specified target."));

  m_targetMenu = menu->popupMenu();

  connect(m_targetMenu, SIGNAL(activated(int)), this, SLOT(slotTargetMenuActivated(int)));
  connect(core(), SIGNAL(projectConfigWidget(KDialogBase*)), this, SLOT(projectConfigWidget(KDialogBase*)));
  connect(core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)), this, SLOT(contextMenu(QPopupMenu *, const Context *)));

  m_antOptionsWidget = 0;
}


AntProjectPart::~AntProjectPart()
{
}


void AntProjectPart::openProject(const QString &dirName, const QString &projectName)
{
  m_projectDirectory = dirName;
  m_projectName = projectName;

  QDomDocument &dom = *projectDom();
  // Set the default directory radio to "executable"
  /// \FIXME there is no kdevantproject so this will not work !
  if (DomUtil::readEntry(dom, "/kdevantproject/run/directoryradio") == "" ) {
    DomUtil::writeEntry(dom, "/kdevantproject/run/directoryradio", "executable");
  }

  /// @todo read alternative build file from properties
  m_antOptions.m_buildXML = "build.xml";

  parseBuildXML();

  fillMenu();

  QFile f(dirName + "/" + projectName + ".filelist");
  if (f.open(IO_ReadOnly)) 
  {
    QTextStream stream(&f);
    while (!stream.atEnd()) 
    {
      QString s = stream.readLine();
      if (!s.startsWith("#"))
        m_sourceFiles << s;
    }
  } 
  else 
    populateProject();
  
  KDevProject::openProject( dirName, projectName );  
}


void AntProjectPart::populateProject()
{
  QApplication::setOverrideCursor(Qt::waitCursor);

  QValueStack<QString> s;
  int prefixlen = m_projectDirectory.length()+1;
  s.push(m_projectDirectory);

  QDir dir;
  do 
  {
    dir.setPath(s.pop());
    kdDebug() << "Examining: " << dir.path() << endl;
    const QFileInfoList *dirEntries = dir.entryInfoList();
    QPtrListIterator<QFileInfo> it(*dirEntries);
    for (; it.current(); ++it) 
    {
      QString fileName = it.current()->fileName();
      if (fileName == "." || fileName == "..")
        continue;
      QString path = it.current()->absFilePath();
      if (it.current()->isDir()) 
      {
        kdDebug() << "Pushing: " << path << endl;
        s.push(path);
      }
      else 
      {
        kdDebug() << "Adding: " << path << endl;
        m_sourceFiles.append(path.mid(prefixlen));
      }
    }
  } 
  while (!s.isEmpty());

  QApplication::restoreOverrideCursor();
}


void AntProjectPart::closeProject()
{
  m_projectDirectory = "";
  m_projectName = "";
  m_buildProjectAction->setEnabled(false);

  m_targetMenu->clear();

  m_antOptions = AntOptions();
  
  QFile f(m_projectDirectory + "/" + m_projectName + ".filelist");
  if (!f.open(IO_WriteOnly))
    return;

  QTextStream stream(&f);
  stream << "# KDevelop Ant Project File List" << endl;

  QStringList::ConstIterator it;
  for (it = m_sourceFiles.begin(); it != m_sourceFiles.end(); ++it)
    stream << (*it) << endl;
  f.close();
}


QString AntProjectPart::projectDirectory() const
{
  return m_projectDirectory;
}


QString AntProjectPart::buildDirectory() const
{
  return m_projectDirectory;
}

QString AntProjectPart::projectName() const
{
  return m_projectName;
}


/** Retuns a PairList with the run environment variables */
DomUtil::PairList AntProjectPart::runEnvironmentVars() const
{
    /// \FIXME there is no kdevantproject so this will not work !
    return DomUtil::readPairListEntry(*projectDom(), "/kdevantproject/run/envvars", "envvar", "name", "value");
}


/** Retuns the currently selected run directory
  * The returned string can be:
  *   if run/directoryradio == executable
  *        The directory where the executable is
  *   if run/directoryradio == build
  *        The directory where the executable is relative to build directory
  *   if run/directoryradio == custom
  *        The custom directory absolute path
  */
QString AntProjectPart::runDirectory() const
{
    return buildDirectory();
    /// \FIXME put the code below into use!

    QDomDocument &dom = *projectDom();

    /// \FIXME there is no kdevantproject so this will not work !
    QString directoryRadioString = DomUtil::readEntry(dom, "/kdevantproject/run/directoryradio");
    QString DomMainProgram = DomUtil::readEntry(dom, "/kdevantproject/run/mainprogram");

    if ( directoryRadioString == "build" )
        return buildDirectory();

    if ( directoryRadioString == "custom" )
        return DomUtil::readEntry(dom, "/kdevantproject/run/customdirectory");

    int pos = DomMainProgram.findRev('/');
    if (pos != -1)
        return buildDirectory() + "/" + DomMainProgram.left(pos);

    return buildDirectory() + "/" + DomMainProgram;

}


/** Retuns the currently selected main program
  * The returned string can be:
  *   if run/directoryradio == executable
  *        The executable name
  *   if run/directoryradio == build
  *        The path to executable relative to build directory
  *   if run/directoryradio == custom or relative == false
  *        The absolute path to executable
  */
QString AntProjectPart::mainProgram(bool relative) const
{
	return QString::null;

    /// \FIXME put the code below into use!
    QDomDocument &dom = *projectDom();

    /// \FIXME there is no kdevantproject so this will not work !
    QString directoryRadioString = DomUtil::readEntry(dom, "/kdevantproject/run/directoryradio");
    QString DomMainProgram = DomUtil::readEntry(dom, "/kdevantproject/run/mainprogram");

    if ( directoryRadioString == "custom" )
        return DomMainProgram;

    if ( relative == false )
        return buildDirectory() + "/" + DomMainProgram;

    if ( directoryRadioString == "executable" ) {
        int pos = DomMainProgram.findRev('/');
        if (pos != -1)
            return DomMainProgram.mid(pos+1);
        return DomMainProgram;
    }
    else
        return DomMainProgram;
}


/** Retuns a QString with the run command line arguments */
QString AntProjectPart::runArguments() const
{
    /// \FIXME there is no kdevantproject so this will not work !
    return DomUtil::readEntry(*projectDom(), "/kdevantproject/run/programargs");
}


QString AntProjectPart::activeDirectory() const
{
  /// \FIXME
  
//  return m_projectDirectory;

	// returning m_projectDirectory is wrong, the path returned should be _relative_ to the project dir
	// returning an empty string until antproject supports the idea of an active directory
	return QString("");
}


QStringList AntProjectPart::allFiles() const
{
/* QStringList res;

  QStringList::ConstIterator it;
  for (it = m_sourceFiles.begin(); it != m_sourceFiles.end(); ++it) 
  {
    QString fileName = *it;
    if (!fileName.startsWith("/")) 
    {
      fileName.prepend("/");
      fileName.prepend(m_projectDirectory);
    }
    res += fileName;
  }

  return res;*/
  
	// return all files relative to the project directory!
	return m_sourceFiles;
}


void AntProjectPart::addFile(const QString &fileName)
{
	QStringList fileList;
	fileList.append ( fileName );
	
	this->addFiles ( fileList );
}

void AntProjectPart::addFiles ( const QStringList& fileList )
{
	QStringList::ConstIterator it;	
	
	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		m_sourceFiles.append (*it );
	}
	
	kdDebug() << "Emitting addedFilesToProject" << endl;
	emit addedFilesToProject(fileList);
}

void AntProjectPart::removeFile(const QString &fileName)
{
	QStringList fileList;
	fileList.append ( fileName );
	
	this->removeFiles ( fileList );
}

void AntProjectPart::removeFiles ( const QStringList& fileList )
{
	kdDebug() << "Emitting removedFilesFromProject" << endl;
	emit removedFilesFromProject(fileList);

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		m_sourceFiles.remove ( *it );
	}
}

void AntProjectPart::parseBuildXML()
{
  m_antOptions.m_targets.clear();
  m_antOptions.m_properties.clear();
  m_antOptions.m_defineProperties.clear();

  // open build file
  QFile bf(m_projectDirectory + "/" + m_antOptions.m_buildXML);
  if (!bf.open(IO_ReadOnly))
    return;

  // parse build file
  QDomDocument dom;
  if (!dom.setContent(&bf))
  {
    bf.close();
    return;
  }
  bf.close();

  m_projectName = dom.documentElement().attribute("name", m_projectName);
  m_antOptions.m_defaultTarget = dom.documentElement().attribute("default", "");

  QDomNode node = dom.documentElement().firstChild();
  while (!node.isNull())
  {
    if (node.toElement().tagName() == "target")
    {
      if (m_antOptions.m_defaultTarget.isEmpty())
        m_antOptions.m_defaultTarget = node.toElement().attribute("name");
      m_antOptions.m_targets.append(node.toElement().attribute("name"));
    }
    else if (node.toElement().tagName() == "property")
    {
      m_antOptions.m_properties.insert(node.toElement().attribute("name"), node.toElement().attribute("value"));
      m_antOptions.m_defineProperties.insert(node.toElement().attribute("name"), false);
    }

    /// @todo Handle property files
    /// @todo evaluate properties' values

    node = node.nextSibling();
  }
}


void AntProjectPart::fillMenu()
{
  m_buildProjectAction->setEnabled(!m_antOptions.m_defaultTarget.isEmpty());

  m_targetMenu->clear();
  int id = 0;
  QStringList::ConstIterator it;
  for (it = m_antOptions.m_targets.begin(); it != m_antOptions.m_targets.end(); ++it)
    m_targetMenu->insertItem(*it, id++);
}


void AntProjectPart::slotBuild()
{
  ant(m_antOptions.m_defaultTarget);
}


void AntProjectPart::slotTargetMenuActivated(int id)
{
  ant(m_antOptions.m_targets[id]);
}


void AntProjectPart::ant(const QString &target)
{
  QString cmd = "cd %0 && %1 ant %2 -buildfile %3 %4 %5";

  QString verb = "";
  switch (m_antOptions.m_verbosity)
  {
  case AntOptions::Quiet:
    verb = "-quiet";
    break;
  case AntOptions::Verbose:
    verb = "-verbose";
    break;
  default:
    verb = "-debug";
    break;
  }

  QString options = "";
  QMap<QString,QString>::Iterator it;
  for (it = m_antOptions.m_properties.begin(); it != m_antOptions.m_properties.end(); ++it)
    if (m_antOptions.m_defineProperties[it.key()])
      options += "-D" + it.key() + "=\"" + it.data() + "\" ";

  QString cp;
  if (!m_classPath.count() == 0)
    cp = "CLASSPATH="+m_classPath.join(":");
  
  makeFrontend()->queueCommand(m_projectDirectory, cmd.arg(m_projectDirectory).arg(cp).arg(target).arg(m_antOptions.m_buildXML).arg(verb).arg(options));
}


void AntProjectPart::projectConfigWidget(KDialogBase *dlg)
{
  QVBox *vbox = dlg->addVBoxPage(i18n("Ant Options"));
  m_antOptionsWidget = new AntOptionsWidget(vbox);

  m_antOptionsWidget->BuildXML->setURL(m_antOptions.m_buildXML);

  switch (m_antOptions.m_verbosity)
  {
  case AntOptions::Quiet:
    m_antOptionsWidget->Verbosity->setCurrentItem(0);
    break;
  case AntOptions::Verbose:
    m_antOptionsWidget->Verbosity->setCurrentItem(1);
    break;
  default:
    m_antOptionsWidget->Verbosity->setCurrentItem(2);
    break;
  }

  m_antOptionsWidget->Properties->setNumRows(m_antOptions.m_properties.count());
  m_antOptionsWidget->Properties->setNumCols(2);

  QMap<QString,QString>::Iterator it;
  int i=0;
  for (it = m_antOptions.m_properties.begin(); it != m_antOptions.m_properties.end(); ++it)
  {
    QCheckTableItem *citem = new QCheckTableItem(m_antOptionsWidget->Properties, it.key());
    citem->setChecked(m_antOptions.m_defineProperties[it.key()]);
    m_antOptionsWidget->Properties->setItem(i,0, citem);
    QTableItem *item = new QTableItem(m_antOptionsWidget->Properties, QTableItem::WhenCurrent, it.data());
    m_antOptionsWidget->Properties->setItem(i,1, item);
    ++i;
  }

  connect(dlg, SIGNAL(okClicked()), this, SLOT(optionsAccepted()));

  vbox = dlg->addVBoxPage(i18n("Classpath"));
  m_classPathWidget = new ClassPathWidget(vbox);

  m_classPathWidget->ClassPath->insertStringList(m_classPath);
}


void AntProjectPart::optionsAccepted()
{
  if (!m_antOptionsWidget || !m_classPathWidget)
    return;

  m_antOptions.m_buildXML = m_antOptionsWidget->BuildXML->url();

  switch (m_antOptionsWidget->Verbosity->currentItem())
  {
  case 1:
    m_antOptions.m_verbosity = AntOptions::Verbose;
    break;
  case 2:
    m_antOptions.m_verbosity = AntOptions::Debug;
    break;
  default:
    m_antOptions.m_verbosity = AntOptions::Quiet;
    break;
  }

  for (int i=0; i<m_antOptionsWidget->Properties->numRows(); ++i)
  {
    QString key = m_antOptionsWidget->Properties->text(i,0);
    m_antOptions.m_properties.replace(key, m_antOptionsWidget->Properties->text(i,1));
    kdDebug() << "PROP: " << key << "  = " << m_antOptionsWidget->Properties->text(i,1);

    QCheckTableItem *item =(QCheckTableItem*) m_antOptionsWidget->Properties->item(i,0);
    m_antOptions.m_defineProperties.replace(key, item->isChecked());
  }

  m_classPath = m_classPathWidget->ClassPath->items();

  m_antOptionsWidget = 0;
  m_classPathWidget = 0;
}


void AntProjectPart::contextMenu(QPopupMenu *popup, const Context *context)
{
  if (!context->hasType( Context::FileContext ))
    return;

  const FileContext *fcontext = static_cast<const FileContext*>(context);
  KURL url = fcontext->urls().first();
  if (URLUtil::isDirectory(url))
    return;

  m_contextFileName = url.fileName();
  bool inProject = project()->allFiles().contains(m_contextFileName.mid ( project()->projectDirectory().length() + 1 ) );
  QString popupstr = QFileInfo(m_contextFileName).fileName();
  if (m_contextFileName.startsWith(projectDirectory()+ "/"))
    m_contextFileName.remove(0, projectDirectory().length()+1);

  popup->insertSeparator();
  if (inProject)
  {
    int id = popup->insertItem( i18n("Remove %1 From Project").arg(popupstr),
                       this, SLOT(slotRemoveFromProject()) );
    popup->setWhatsThis(id, i18n("<b>Remove from project</b><p>Removes current file from the project."));
  }
  else
  {
    int id = popup->insertItem( i18n("Add %1 to Project").arg(popupstr),
                       this, SLOT(slotAddToProject()) );
    popup->setWhatsThis(id, i18n("<b>Add to project</b><p>Adds current file from the project."));
  }
}


void AntProjectPart::slotAddToProject()
{
	QStringList fileList;	
	fileList.append ( m_contextFileName );
	addFiles ( fileList );
}


void AntProjectPart::slotRemoveFromProject()
{
	QStringList fileList;	
	fileList.append ( m_contextFileName );
	removeFiles ( fileList );
}


#include "antprojectpart.moc"


/*!
    \fn AntProjectPart::distFiles() const
 */
QStringList AntProjectPart::distFiles() const
{
	QStringList sourceList = allFiles();
	// Scan current source directory for any .pro files.
	QString projectDir = projectDirectory();
	QDir dir(projectDir);
	QStringList files = dir.entryList( "build.xml");
	return sourceList + files;
}
