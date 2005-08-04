/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <q3whatsthis.h>
#include <qdom.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <q3vbox.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3PtrList>

#include <kdeversion.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kstdaction.h>
#include <kaction.h>
#include <kapplication.h>
#include <kactionclasses.h>
#include <kpopupmenu.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"
#include "configwidgetproxy.h"

#include "filetemplate.h"
#include "domutil.h"
#include "urlutil.h"

#include "filecreate_widget2.h"
#include "filecreate_widget3.h"
#include "filecreate_part.h"
#include "filecreate_filetype.h"
#include "filecreate_filedialog.h"
#include "filecreate_newfile.h"
#include "fcconfigwidget.h"

#define PROJECTSETTINGSPAGE 1
#define GLOBALSETTINGSPAGE 2

#include "kdevplugininfo.h"

static const KDevPluginInfo data("kdevfilecreate");

typedef KDevGenericFactory<FileCreatePart> FileCreateFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevfilecreate, FileCreateFactory( data ) )

using namespace FileCreate;

FileCreatePart::FileCreatePart(QObject *parent, const char *name, const QStringList & )
    : KDevCreateFile(&data, parent, name ? name : "FileCreatePart"), m_selectedWidget(-1), m_useSideTab(true), m_subPopups(0)
{
  setInstance(FileCreateFactory::instance());
  setXMLFile("kdevpart_filecreate.rc");

  connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
  connect( core(), SIGNAL(projectClosed()), this, SLOT(slotProjectClosed()) );
  
	_configProxy = new ConfigWidgetProxy( core() );
	_configProxy->createProjectConfigPage( i18n("File Templates"), PROJECTSETTINGSPAGE, info()->icon() );
	_configProxy->createGlobalConfigPage( i18n("File Templates"), GLOBALSETTINGSPAGE, info()->icon() );
	connect( _configProxy, SIGNAL(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )), 
		this, SLOT(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )) );


  KToolBarPopupAction * newAction = new KToolBarPopupAction( i18n("&New"), "filenew", CTRL+Qt::Key_N, this, SLOT(slotNewFile()), actionCollection(), "file_new");
  newAction->setWhatsThis( i18n("<b>New file</b><p>Creates a new file. Also adds it the project if the <b>Add to project</b> checkbox is turned on.") );
  newAction->setToolTip( i18n("Create a new file") );
  m_newPopupMenu = newAction->popupMenu();
  connect(m_newPopupMenu, SIGNAL(aboutToShow()), this, SLOT(slotAboutToShowNewPopupMenu()));
  
//  m_filetypes.setAutoDelete(true);

  m_availableWidgets[0] = new FriendlyWidget(this);
  m_availableWidgets[1] = new ListWidget(this);
  m_numWidgets = 2;

  /// @todo provide a way of choosing your preferred widget without
  /// having to change the source, as this is not considered 'user-friendly'
  /// these days, I'm led to believe.
  selectWidget(1);
  
  QTimer::singleShot( 0, this, SLOT(slotGlobalInitialize()) );
}


FileCreatePart::~FileCreatePart()
{
/*
  for(int c=0;c<m_numWidgets;c++){
    if (TypeChooser* chooser = m_availableWidgets[c]) {

       if( QWidget* as_widget = dynamic_cast<QWidget*>(chooser) )
           mainWindow()->removeView( as_widget );

       delete chooser;
    }
  }
*/  
  delete _configProxy;
  
  m_newPopupMenu->clear();
  delete m_subPopups;
}

void FileCreatePart::insertConfigWidget( const KDialogBase * dlg, QWidget * page, unsigned int pagenumber )
{
	kdDebug() << k_funcinfo << endl;
	
	switch( pagenumber )
	{
		case PROJECTSETTINGSPAGE:
		{
			FCConfigWidget* w = new FCConfigWidget( this, false, page, "filecreate config widget" );
			connect( dlg, SIGNAL( okClicked( ) ), w, SLOT( accept( ) ) );
		}
		break;
		
		case GLOBALSETTINGSPAGE:
		{
			FCConfigWidget *w = new FCConfigWidget( this, true, page, "filecreate config widget" );
			connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
		}
		break;
	}
}

void FileCreatePart::selectWidget(int widgetNumber) {
  if (m_selectedWidget==widgetNumber) return;
  if (widgetNumber<-1 || widgetNumber>=m_numWidgets) return;
  if (setWidget(widgetNumber==-1 ? NULL : m_availableWidgets[widgetNumber]))
    m_selectedWidget = widgetNumber;
}


bool FileCreatePart::setWidget(TypeChooser * widg) {

  QWidget *as_widget = widg ? dynamic_cast<QWidget*>(widg) : NULL;

  // remove the existing widget
  TypeChooser *tc = typeChooserWidget();
  if (tc) {
    disconnect( tc->signaller(), SIGNAL(filetypeSelected(const FileType *)), this, SLOT(slotFiletypeSelected(const FileType *)) );
    QWidget *as_widget2 = dynamic_cast<QWidget*>(tc);
    if (as_widget2) {
        kdDebug(9034) << "filecreate_part: Removing as_widget2" << endl;
      mainWindow()->removeView(as_widget2);
    } else
      kdWarning(9034) << "WARNING: could not cast to as_widget2" << endl;

  }

  if (widg && as_widget) {
    connect( widg->signaller(), SIGNAL(filetypeSelected(const FileType *)), this, SLOT(slotFiletypeSelected(const FileType *)) );
    mainWindow()->embedSelectView(as_widget, i18n("New File"), i18n("File creation"));
  }

  return true;
}

void FileCreatePart::refresh() {
  if (typeChooserWidget()) typeChooserWidget()->refresh();
}

void FileCreatePart::slotAboutToShowNewPopupMenu()
{
	KIconLoader * m_iconLoader = KGlobal::iconLoader();
	m_newPopupMenu->clear();
	delete m_subPopups;
	m_subPopups = NULL;
	int id = 0;
	FileType * filetype = m_filetypes.first();
	for(; filetype; filetype=m_filetypes.next())
	{
		if (filetype->enabled())
		{
			if (filetype->subtypes().count()==0)
			{
				QPixmap iconPix = m_iconLoader->loadIcon(
					filetype->icon(), KIcon::Desktop, KIcon::SizeSmall,
					KIcon::DefaultState, NULL, true);
				m_newPopupMenu->insertItem(iconPix, filetype->name(), this,
					SLOT(slotNewFilePopup(int)), 0, ++id );
				m_newPopupMenu->setItemParameter( id, (long)filetype );
			} else
			{
				KPopupMenu* subMenu = NULL;
				Q3PtrList<FileType> subtypes = filetype->subtypes();
				for(FileType * subtype = subtypes.first(); subtype; subtype=subtypes.next())
				{
					if (subtype->enabled()){
						if( !subMenu )
							subMenu = new KPopupMenu(0,0);
						QPixmap iconPix = m_iconLoader->loadIcon(
							subtype->icon(), KIcon::Desktop, KIcon::SizeSmall,
							KIcon::DefaultState, NULL, true);
						subMenu->insertItem(iconPix, subtype->name(), this,
							SLOT(slotNewFilePopup(int)), 0, ++id );
						subMenu->setItemParameter( id, (long)subtype );
					}
				}
				if( subMenu )
				{
					if( !m_subPopups )
					{
						m_subPopups = new Q3PtrList<KPopupMenu>;
						m_subPopups->setAutoDelete(true);
					}
					m_subPopups->append( subMenu );
					m_newPopupMenu->insertItem( filetype->name(), subMenu );
				}
			}

		}

	}
}

void FileCreatePart::slotNewFilePopup( int pFileType )
{
	const FileType* filetype = (const FileType*) pFileType;
	slotFiletypeSelected( filetype );
}

void FileCreatePart::slotNewFile() {
  KDevCreateFile::CreatedFile createdFile = createNewFile();
  openCreatedFile(createdFile);
}

void FileCreatePart::slotProjectOpened() {
    QTimer::singleShot( 0, this, SLOT(slotInitialize()) );
}

void FileCreatePart::addFileType(const QString & filename) {
  FileType * filetype = getType(filename);
  if (!filetype) {
    filetype = new FileType;
    filetype->setName( filename + " files" );
    filetype->setExt( filename );
    filetype->setCreateMethod("template");
    m_filetypes.append(filetype);
  }
  filetype->setEnabled(true);
}

void FileCreatePart::slotProjectClosed() {
  m_filetypes.clear();
  refresh();
  QTimer::singleShot( 0, this, SLOT(slotGlobalInitialize()) );
}

void FileCreatePart::slotFiletypeSelected(const FileType * filetype) {

  KDevCreateFile::CreatedFile createdFile = createNewFile(filetype->ext(),
                                                          QString::null,
                                                          QString::null,
                                                          filetype->subtypeRef());

  if (project())
    openCreatedFile(createdFile);

//  mainWindow()->lowerView( typeChooserWidgetAsQWidget() );
}

void FileCreatePart::openCreatedFile(const KDevCreateFile::CreatedFile & createdFile) {
	if (createdFile.status == KDevCreateFile::CreatedFile::STATUS_OK && project() ) {
    KURL uu(project()->projectDirectory() + createdFile.dir + "/" + createdFile.filename );
    partController()->editDocument ( uu );
  }
}

int FileCreatePart::readTypes(const QDomDocument & dom, Q3PtrList<FileType> &m_filetypes, bool enable) {
  int numRead = 0;
  QDomElement fileTypes = DomUtil::elementByPath(dom,"/kdevfilecreate/filetypes");
  if (!fileTypes.isNull()) {
    for(QDomNode node = fileTypes.firstChild();!node.isNull();node=node.nextSibling()) {
//      kapp->processEvents();

      if (node.isElement() && node.nodeName()=="type") {
        QDomElement element = node.toElement();
        FileType * filetype = new FileType;
        filetype->setName( element.attribute("name") );
        filetype->setExt( element.attribute("ext") );
        filetype->setCreateMethod( element.attribute("create") );

        filetype->setIcon( element.attribute("icon") );
        filetype->setDescr( (DomUtil::namedChildElement(element, "descr")).text() );
        filetype->setEnabled(enable || (filetype->ext()==""));
        m_filetypes.append(filetype);
        numRead++;

        kdDebug(9034) << "node: " << filetype->name().latin1() << endl;

        if (node.hasChildNodes()) {
          for(QDomNode subnode = node.firstChild();!subnode.isNull();subnode=subnode.nextSibling()) {
            kdDebug(9034) << "subnode: " << subnode.nodeName().latin1() << endl;
//            kapp->processEvents();
            if (subnode.isElement() && subnode.nodeName()=="subtype") {
              QDomElement subelement = subnode.toElement();
              FileType * subtype = new FileType;
              subtype->setExt( filetype->ext() );
              subtype->setCreateMethod( filetype->createMethod() );
              subtype->setSubtypeRef( subelement.attribute("ref") );
              subtype->setIcon( subelement.attribute("icon") );
              subtype->setName( subelement.attribute("name") );
              subtype->setDescr( (DomUtil::namedChildElement(subelement, "descr")).text() );
              subtype->setEnabled(enable);
              filetype->addSubtype(subtype);
            }
          }
        }
      }
    }
  }
  return numRead;
}

FileType * FileCreatePart::getType(const QString & ex, const QString subtRef) {

  QString subtypeRef = subtRef;
  QString ext = ex;
  int dashPos = ext.find('-');
  if (dashPos>-1 && subtRef.isNull()) {
    ext = ex.left(dashPos);
    subtypeRef = ex.mid(dashPos+1);
  }

  Q3PtrList<FileType> filetypes = getFileTypes();
  for(FileType * filetype = filetypes.first();
      filetype;
      filetype=filetypes.next()) {
    if (filetype->ext()==ext) {
      if (subtypeRef.isNull()) return filetype;
      Q3PtrList<FileType> subtypes = filetype->subtypes();
      for(FileType * subtype = subtypes.first();
          subtype;
          subtype=subtypes.next()) {
        if (subtypeRef==subtype->subtypeRef()) return subtype;
      }
    }
  }
  return NULL;
}

FileType * FileCreatePart::getEnabledType(const QString & ex, const QString subtRef) {

  QString subtypeRef = subtRef;
  QString ext = ex;
  int dashPos = ext.find('-');
  if (dashPos>-1 && subtRef.isNull()) {
    ext = ex.left(dashPos);
    subtypeRef = ex.mid(dashPos+1);
  }

  Q3PtrList<FileType> filetypes = getFileTypes();
  for(FileType * filetype = filetypes.first();
      filetype;
      filetype=filetypes.next()) {
    if (filetype->ext()==ext) {
      if ( (subtypeRef.isNull()) && (filetype->enabled()) ) return filetype;
      Q3PtrList<FileType> subtypes = filetype->subtypes();
      for(FileType * subtype = subtypes.first();
          subtype;
          subtype=subtypes.next()) {
        if ( (subtypeRef==subtype->subtypeRef()) && (filetype->enabled()) ) return subtype;
      }
    }
  }
  return NULL;
}

// KDevFileCreate interface

KDevCreateFile::CreatedFile FileCreatePart::createNewFile(QString ext, QString dir, QString name, QString subtype)
{
  KDevCreateFile::CreatedFile result;
  
  KURL projectURL;
  if ( !project() )
  {
    //result.status = KDevCreateFile::CreatedFile::STATUS_NOTCREATED;
    //return result;
  }
  else
  {
    projectURL = project()->projectDirectory();
  }

  KURL selectedURL;

  NewFileChooser dialog;
  dialog.setFileTypes(m_filetypes);
  const FileType *filetype = getEnabledType(ext,subtype);
  kdDebug(9034) << "Looking for filetype pointer for " << ext << "/" << subtype << endl;
  if (filetype) {
    kdDebug(9034) << "found filetype" << endl;
  } else {
    kdDebug(9034) << "could not find filetype" << endl;
  }
  if (!project())
    dialog.setInProjectMode(false);

  if (!dir.isNull())
    dialog.setDirectory(dir);
  else if (!project())
    dialog.setDirectory(QDir::currentDirPath());
  else
  {
    QString activeDir = project()->activeDirectory();
    dialog.setDirectory( project()->projectDirectory() +
        ( activeDir[0] == '/' ? "" : "/" )
        + activeDir );
  }
  if (!name.isNull()) dialog.setName(name);
  if (filetype) dialog.setCurrent(filetype);

  int dialogResult = dialog.exec();

  if (dialogResult == KDialogBase::Rejected) {
    result.status = KDevCreateFile::CreatedFile::STATUS_NOTCREATED;
    return result;
  }

  // OK was pressed

  result.addToProject = dialog.addToProject();
  selectedURL = dialog.url();
  const FileType *selectedFileType = dialog.selectedType();

  if (dialog.addToProject() && !projectURL.isParentOf(selectedURL)) {
    result.status = KDevCreateFile::CreatedFile::STATUS_NOTWITHINPROJECT;
    return result;
  }

  if (selectedFileType) {
    ext = selectedFileType->ext();
    subtype = selectedFileType->subtypeRef();
  }

  QString fullPath = selectedURL.path();
  // add appropriate extension, if not already there
  if (!ext.isNull() & ext!="" & !fullPath.endsWith("." + ext)) fullPath+="." + ext;

  QString filename = URLUtil::filename(fullPath);
  kdDebug(9034) << "full path = " << fullPath << endl;

  // add in subtype, if specified
  if (!subtype.isEmpty())
      ext += "-" + subtype;

  // create file from template
  if (!FileTemplate::exists(this, ext) ||
      !FileTemplate::copy(this, ext, fullPath) ) {
      // no template, create a blank file instead
      QFile f(fullPath);
      f.open( QIODevice::WriteOnly );
      f.close();
  }
  if (dialog.addToProject())
  {
    // work out the path relative to the project directory
//    QString relToProj = URLUtil::relativePath(projectURL, selectedURL, URLUtil::SLASH_PREFIX );
    QString relToProj = URLUtil::relativePath(projectURL.path(), fullPath, URLUtil::SLASH_PREFIX );
    project()->addFile(relToProj.mid(1));
  }
  else
  {
    KURL url;
    url.setPath(fullPath);
    partController()->editDocument(url);
  }
  
  QString fileName = URLUtil::filename(fullPath);
  kdDebug(9034) << "file name = " << filename << endl;
  
  result.filename = fileName;
  result.dir = URLUtil::directory(fullPath);
  result.status = KDevCreateFile::CreatedFile::STATUS_OK;

  return result;
}

void FileCreatePart::setShowSideTab(bool on) {
  selectWidget(on ? 1 : -1 );
}

void FileCreatePart::slotNoteFiletype(const FileType * filetype) {
  kdDebug(9034) << "Noting file type: " << (filetype ? filetype->ext() : QString::fromLatin1("Null") ) << endl;
  m_filedialogFiletype = filetype;
}

void FileCreatePart::slotInitialize( )
{
  m_filetypes.clear();
  refresh();
  
  //read global configuration
  slotGlobalInitialize();

  // read in which global templates are to be used for this project
  QDomElement useGlobalTypes =
    DomUtil::elementByPath(*projectDom(),"/kdevfilecreate/useglobaltypes");
  for(QDomNode node = useGlobalTypes.firstChild();
      !node.isNull();node=node.nextSibling()) {

//    kapp->processEvents();

    if (node.isElement() && node.nodeName()=="type") {
      QDomElement element = node.toElement();
      QString ext = element.attribute("ext");
      QString subtyperef = element.attribute("subtyperef");
      // if an extension has been specified as enabled, ensure it
      // and all its subtypes are enabled
      if (subtyperef.isNull()) {
        FileType * filetype = getType(ext);
        if (filetype) {
          filetype->setEnabled(true);
          if (filetype->subtypes().count())
            filetype->setSubtypesEnabled(true);
        }
      } else {
        // if an extension + subtype have been specified, enable
        // the subtype and the extension (the 'parent')
        FileType * filetype = getType(ext);
        FileType * subtype = getType(ext,subtyperef);
        if (filetype && subtype) {
          filetype->setEnabled(true);
          subtype->setEnabled(true);
        }
      }
    }
  }

  // read in the list of file types for this project
  if ( project() && readTypes( *projectDom(), m_filetypes, true )==0  ) {
    // default by scanning the templates directory if no template info
    // found in project file
    QDir templDir( project()->projectDirectory() + "/templates/" );
    if (templDir.exists()) {
      templDir.setFilter( QDir::Files );
      const QFileInfoList * list = templDir.entryInfoList();
      if( list ){
        QFileInfoListIterator it( *list );
        QFileInfo *fi;
        while ( (fi = it.current()) != 0 ) {
          addFileType(fi->fileName());
          ++it;
        }
      }
    }
/*    else { // it was probably an imported project
      // KLUDGE: we need a better way to determine file types
      // the current method looks a bit too restrictive
      addFileType( "cpp" );
      addFileType( "h" );
    }*/
  }

  setShowSideTab(m_useSideTab);

  // refresh view
  refresh();
}

void FileCreatePart::slotGlobalInitialize( )
{
  // read in global template information
  QString globalXMLFile = ::locate("data", "kdevfilecreate/template-info.xml");
  kdDebug(9034) << "Found global template info info " << globalXMLFile << endl;
  QDomDocument globalDom;
  if (!globalXMLFile.isNull() &&
      DomUtil::openDOMFile(globalDom,globalXMLFile)) {
    kdDebug(9034) << "Reading global template info..." << endl;
//    kapp->processEvents();
    readTypes(globalDom, m_filetypes, false);

    // use side tab or not?
    /// @todo this is a very Bad Way to do this. Must remember to move this setting to user's kdeveloprc config file
    QDomElement useSideTab = DomUtil::elementByPath(globalDom,"/kdevfilecreate/sidetab");
    if (!useSideTab.isNull() && useSideTab.attribute("active")=="no") {
        m_useSideTab = false;

		setShowSideTab(m_useSideTab); // not the cleanest thing to do.. but c'mon, look at the rest of this code.. ;)
    }
  }
  
  // refresh view
  refresh();
}

#include "filecreate_part.moc"

	// kate: indent-width 4; replace-tabs off; tab-width 4; space-indent off;
