/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <qwhatsthis.h>
#include <qdom.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qvbox.h>

#include <kdeversion.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kstdaction.h>
#include <kaction.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"

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

typedef KGenericFactory<FileCreatePart> FileCreateFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevfilecreate, FileCreateFactory( "kdevfilecreate" ) );

using namespace FileCreate;

FileCreatePart::FileCreatePart(QObject *parent, const char *name, const QStringList & )
    : KDevCreateFile("FileCreate", "filecreate", parent, name ? name : "FileCreatePart"), m_selectedWidget(-1), m_useSideTab(true)
{
  setInstance(FileCreateFactory::instance());
  setXMLFile("kdevpart_filecreate.rc");

  connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
  connect( core(), SIGNAL(projectClosed()), this, SLOT(slotProjectClosed()) );
  connect( core(), SIGNAL(configWidget(KDialogBase*)), this, SLOT(configWidget(KDialogBase*)));
  connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)), this, SLOT(projectConfigWidget(KDialogBase*)));


  KAction * newAction = KStdAction::openNew(this, SLOT(slotNewFile()), actionCollection(), "file_new");
  newAction->setStatusText( i18n("Creates a new file") );
  newAction->setText( i18n("Create a new file", "New...") );
  newAction->setWhatsThis( i18n("Use this to create a new file within your project.") );
  newAction->setToolTip( i18n("New file") );

  m_filetypes.setAutoDelete(true);

  m_availableWidgets[0] = new FriendlyWidget(this);
  m_availableWidgets[1] = new ListWidget(this);
  m_numWidgets = 2;

  // TODO: provide a way of choosing your preferred widget without
  // having to change the source, as this is not considered 'user-friendly'
  // these days, I'm led to believe.
  selectWidget(1);


}


FileCreatePart::~FileCreatePart()
{
  for(int c=0;c<m_numWidgets;c++)
    if (m_availableWidgets[c]) delete m_availableWidgets[c];
}

void FileCreatePart::configWidget(KDialogBase *dlg)
{
  QVBox *vbox = dlg->addVBoxPage(i18n("New File Wizard Options"));
  FCConfigWidget *w = new FCConfigWidget(this, true, vbox, "filecreate config widget");
  connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
}

void FileCreatePart::projectConfigWidget( KDialogBase* dlg )
{
  QVBox* vbox = dlg->addVBoxPage(i18n("New File Wizard Options"));
  FCConfigWidget* w = new FCConfigWidget( this, false, vbox, "filecreate config widget" );
  connect( dlg, SIGNAL( okClicked( ) ), w, SLOT( accept( ) ) );
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
    mainWindow()->embedSelectView(as_widget, i18n("New File"), i18n("file creation"));
  }

  return true;
}

void FileCreatePart::refresh() {
  if (typeChooserWidget()) typeChooserWidget()->refresh();
}

void FileCreatePart::slotNewFile() {
  KDevCreateFile::CreatedFile createdFile = createNewFile();
  openCreatedFile(createdFile);
}

void FileCreatePart::slotProjectOpened() {

  // read in global template information
  QString globalXMLFile = ::locate("data", "kdevfilecreate/template-info.xml");
  kdDebug(9034) << "Found global template info info " << globalXMLFile << endl;
  QDomDocument globalDom;
  if (!globalXMLFile.isNull() &&
      DomUtil::openDOMFile(globalDom,globalXMLFile)) {
    kdDebug(9034) << "Reading global template info..." << endl;
    readTypes(globalDom, m_filetypes, false);

    // use side tab or not?
    // TODO: this is a very Bad Way to do this. Must remember to move this setting to user's gideonrc config file
    QDomElement useSideTab = DomUtil::elementByPath(globalDom,"/kdevfilecreate/sidetab");
    if (!useSideTab.isNull() && useSideTab.attribute("active")=="no") {
        m_useSideTab = false;
    }
  }


  // read in which global templates are to be used for this project
  QDomElement useGlobalTypes =
    DomUtil::elementByPath(*projectDom(),"/kdevfilecreate/useglobaltypes");
  for(QDomNode node = useGlobalTypes.firstChild();
      !node.isNull();node=node.nextSibling()) {
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
  if ( readTypes( *projectDom(), m_filetypes, true )==0  ) {
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
    else { // it was probably an imported project
      // KLUDGE: we need a better way to determine file types
      // the current method looks a bit too restrictive
      addFileType( "cpp" );
      addFileType( "h" );
    }
  }

  setShowSideTab(m_useSideTab);

  // refresh view
  refresh();
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
}

void FileCreatePart::slotFiletypeSelected(const FileType * filetype) {

  KDevCreateFile::CreatedFile createdFile = createNewFile(filetype->ext(),
                                                          QString::null,
                                                          QString::null,
                                                          filetype->subtypeRef());

  openCreatedFile(createdFile);

  mainWindow()->lowerView( typeChooserWidgetAsQWidget() );
}

void FileCreatePart::openCreatedFile(const KDevCreateFile::CreatedFile & createdFile) {
  if (createdFile.status == KDevCreateFile::CreatedFile::STATUS_OK) {
    KURL uu(project()->projectDirectory() + createdFile.dir + "/" + createdFile.filename );
    partController()->editDocument ( uu );
  }
}

int FileCreatePart::readTypes(const QDomDocument & dom, QPtrList<FileType> &m_filetypes, bool enable) {
  int numRead = 0;
  QDomElement fileTypes = DomUtil::elementByPath(dom,"/kdevfilecreate/filetypes");
  if (!fileTypes.isNull()) {
    for(QDomNode node = fileTypes.firstChild();!node.isNull();node=node.nextSibling()) {
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

  QPtrList<FileType> filetypes = getFileTypes();
  for(FileType * filetype = filetypes.first();
      filetype;
      filetype=filetypes.next()) {
    if (filetype->ext()==ext) {
      if (subtypeRef.isNull()) return filetype;
      QPtrList<FileType> subtypes = filetype->subtypes();
      for(FileType * subtype = subtypes.first();
          subtype;
          subtype=subtypes.next()) {
        if (subtypeRef==subtype->subtypeRef()) return subtype;
      }
    }
  }
  return NULL;
}

// KDevFileCreate interface

// This is the old way -- to be removed if everyone's OK with the new way!
#if 0
KDevCreateFile::CreatedFile FileCreatePart::createNewFile(QString ext, QString dir, QString name, QString subtype) {

  KDevCreateFile::CreatedFile result;
  KURL projectURL( project()->projectDirectory() );
  KURL selectedURL;

  KDialogBase * dialogBase = NULL;
  FriendlyWidget * filetypeWidget = NULL;

  FileDialog * fileDialogWidget = NULL;

  // If the file type (extension) is unknown, we're going to need to ask
  if (ext.isNull()) {
    m_filedialogFiletype = NULL;
    filetypeWidget = new FriendlyWidget(this);
    connect( filetypeWidget->signaller(), SIGNAL(filetypeSelected(const FileType *) ) ,
             this, SLOT(slotNoteFiletype(const FileType *)) );
    filetypeWidget->refresh();
  }


  // If the directory was supplied, it was relative to the project dir
  if (!dir.isNull()) dir = project()->projectDirectory() + "/" + dir;

  // If the file name or path is unknown, we're going to need to ask
  if (dir.isNull() || name.isNull()) {

    // if no path is known, start at project root
    if (dir.isNull())
      dir=project()->projectDirectory();

    fileDialogWidget = new FileDialog(dir, "*." + ext, 0, "New file", true, filetypeWidget);

    dialogBase = fileDialogWidget;

  }

  // if no dialog has been created but there's a widget to be displayed,
  // create a holding dialog and swallow the widget
  if (!dialogBase && filetypeWidget) {
    dialogBase = new KDialogBase(KDialogBase::Swallow, QString("New file"), KDialogBase::Ok|KDialogBase::Cancel,
                                   KDialogBase::Ok, 0, "New file", true);
    dialogBase->setMainWidget(filetypeWidget);
  }


  // if there's a dialog display it
  if (dialogBase) {

    kdDebug(9034) << "Calling dialog..." << endl;
    int fdResult = dialogBase->exec();
    kdDebug(9034) << "Exited dialog..." << endl;

    // deal with the results...

    // if cancel was pressed, abort
    if (fdResult==QDialog::Rejected) {
      result.status = KDevCreateFile::CreatedFile::STATUS_NOTCREATED;
      delete dialogBase;
      return result;
    }

    // if there was a file path to be selected, store it
    if (fileDialogWidget) {
      selectedURL = fileDialogWidget->selectedURL();
      if (!projectURL.isParentOf(selectedURL)) {
        delete dialogBase;
        result.status = KDevCreateFile::CreatedFile::STATUS_NOTWITHINPROJECT;
        return result;
      }
    }

    // if there was a type to be chosen, store it
    if (filetypeWidget) {
      filetypeWidget = NULL; // deleted already as reparented in KFileDialog or KDialogBase
      if (m_filedialogFiletype) {
        ext = m_filedialogFiletype->ext();
        subtype = m_filedialogFiletype->subtypeRef();
      }
    }

    delete dialogBase; dialogBase=NULL;

  } else {
    // we know the directory and filename
    selectedURL = dir + "/" + name;
  }

  // work out the path relative to the project directory
  QString relToProj = URLUtil::relativePath(projectURL, selectedURL, URLUtil::SLASH_PREFIX );

  // add appropriate extension, if not already there
  if (!relToProj.endsWith("." + ext)) relToProj+="." + ext;

  QString filename = URLUtil::filename(relToProj);

  kdDebug(9034) << "relative to proj dir = " << relToProj << endl;
  kdDebug(9034) << "filename = " << filename << endl;

  // add in subtype, if specified
  if (!subtype.isEmpty())
    ext += "-" + subtype;

  // create file from template, and add it to the project
  if (FileTemplate::exists(this, ext)) {
    if (FileTemplate::copy(this, ext, project()->projectDirectory() + relToProj))
      project()->addFile(relToProj.mid(1));
  }


  // tell the caller what we did
  result.filename = filename;
  result.dir = URLUtil::directory(relToProj);
  result.status = KDevCreateFile::CreatedFile::STATUS_OK;
  return result;

}
#endif

KDevCreateFile::CreatedFile FileCreatePart::createNewFile(QString ext, QString dir, QString name, QString subtype)
{
  KDevCreateFile::CreatedFile result;
  KURL projectURL( project()->projectDirectory() );
  KURL selectedURL;

  NewFileChooser dialog;
  dialog.setFileTypes(m_filetypes);
  const FileType *filetype = getType(ext,subtype);
  kdDebug(9034) << "Looking for filetype pointer for " << ext << "/" << subtype << endl;
  if (filetype) {
    kdDebug(9034) << "found filetype" << endl;
  } else {
    kdDebug(9034) << "could not find filetype" << endl;
  }

  if (!dir.isNull())
    dialog.setDirectory(dir);
  else
    dialog.setDirectory( project()->projectDirectory() + "/" + project()->activeDirectory() );
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


  // work out the path relative to the project directory
  QString relToProj = URLUtil::relativePath(projectURL, selectedURL, URLUtil::SLASH_PREFIX );

  // add appropriate extension, if not already there
  if (!ext.isNull() & ext!="" & !relToProj.endsWith("." + ext)) relToProj+="." + ext;

  QString filename = URLUtil::filename(relToProj);

  kdDebug(9034) << "relative to proj dir = " << relToProj << endl;
  kdDebug(9034) << "filename = " << filename << endl;

  // add in subtype, if specified
  if (!subtype.isEmpty())
    ext += "-" + subtype;

  // create file from template, and add it to the project
  if (!FileTemplate::exists(this, ext) ||
      !FileTemplate::copy(this, ext, project()->projectDirectory() + relToProj) ) {
    // no template, create a blank file instead
    QFile f(project()->projectDirectory() + relToProj);
    f.open( IO_WriteOnly );
    f.close();
  }

  if (result.addToProject)
    project()->addFile(relToProj.mid(1));




  // tell the caller what we did
  result.filename = filename;
  result.dir = URLUtil::directory(relToProj);
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

#include "filecreate_part.moc"
