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

#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kstdaction.h>
#include <kaction.h>

#include "kdevcore.h"
#include "kdevtoplevel.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"

#include "filetemplate.h"
#include "domutil.h"
#include "urlutil.h"

#include "filecreate_widget.h"
#include "filecreate_widget2.h"
#include "filecreate_part.h"
#include "filecreate_filetype.h"

typedef KGenericFactory<FileCreatePart> FileCreateFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevfilecreate, FileCreateFactory( "kdevfilecreate" ) );

FileCreatePart::FileCreatePart(QObject *parent, const char *name, const QStringList& )
  : KDevPlugin(parent, name), m_selectedWidget(-1)
{
  setInstance(FileCreateFactory::instance());
  setXMLFile("kdevpart_filecreate.rc"); 

  connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
  connect( core(), SIGNAL(projectClosed()), this, SLOT(slotProjectClosed()) );

  KAction * newAction = KStdAction::openNew(this, SLOT(slotNewFile()), actionCollection(), "file_new");
  newAction->setStatusText( i18n("Creates a new file") );
  m_filetypes.setAutoDelete(true);

  m_availableWidgets[0] = new FileCreateWidget(this);
  m_availableWidgets[1] = new FileCreateWidget2(this);
  m_numWidgets = 2;
  
  //setWidget(new FileCreateWidget2(this));
  selectWidget(1);
  
  
}


FileCreatePart::~FileCreatePart()
{
  for(int c=0;c<m_numWidgets;c++)
    if (m_availableWidgets[c]) delete m_availableWidgets[c];
}

void FileCreatePart::selectWidget(int widgetNumber) {
  if (widgetNumber<0 || widgetNumber>=m_numWidgets) return;
  if (setWidget(m_availableWidgets[widgetNumber]))
    m_selectedWidget = widgetNumber;
}
  

bool FileCreatePart::setWidget(FileCreateTypeChooser * widg) {
  if (!widg) return false;
  // the type chooser must of course also be derived from QWidget
  QWidget * as_widget = dynamic_cast<QWidget*>(widg);
  if (as_widget) {
    // remove the existing widget
    if (widget()) {
      disconnect( widget()->signaller(), SIGNAL(filetypeSelected(const FileCreateFileType *)), this, SLOT(slotFiletypeSelected(const FileCreateFileType *)) );
      QWidget * as_widget2 = dynamic_cast<QWidget*>(widget());
      if (as_widget2)
        topLevel()->removeView(as_widget2);
    }
    connect( widg->signaller(), SIGNAL(filetypeSelected(const FileCreateFileType *)), this, SLOT(slotFiletypeSelected(const FileCreateFileType *)) );
    topLevel()->embedSelectView(as_widget, i18n("New File"), i18n("file creation"));
  }
  return true;
}


void FileCreatePart::refresh() {
  if (widget()) widget()->refresh();
}

void FileCreatePart::slotNewFile() {
  createNewFile();
}

void FileCreatePart::slotProjectOpened() {

  // read in global template information
  QString globalXMLFile = ::locate("data", "kdevfilecreate/template-info.xml");
  QDomDocument globalDom;
  if (globalXMLFile!=QString::null &&
      DomUtil::openDOMFile(globalDom,globalXMLFile)) {
    readTypes(globalDom, false);
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
      if (subtyperef==QString::null) {
        FileCreateFileType * filetype = getType(ext);
        if (filetype) {
          filetype->setEnabled(true);
          if (filetype->subtypes().count())
            filetype->setSubtypesEnabled(true);
        }
      } else {
        // if an extension + subtype have been specified, enable
        // the subtype and the extension (the 'parent')
        FileCreateFileType * filetype = getType(ext);
        FileCreateFileType * subtype = getType(ext,subtyperef);
        if (filetype && subtype) {
          filetype->setEnabled(true);
          subtype->setEnabled(true);
        }
      }
    }
  }

  // read in the list of file types for this project
  if ( readTypes( *projectDom(), true )==0  ) {
    // default by scanning the templates directory if no template info
    // found in project file
    QDir templDir( project()->projectDirectory() + "/templates/" );
    templDir.setFilter( QDir::Files );
    const QFileInfoList * list = templDir.entryInfoList();
    if( list ){
      QFileInfoListIterator it( *list );
      QFileInfo *fi;
      while ( (fi = it.current()) != 0 ) {
        FileCreateFileType * filetype = getType(fi->fileName());
        if (!filetype) {
          filetype = new FileCreateFileType;
          filetype->setName( fi->fileName() + " files" );
          filetype->setExt( fi->fileName() );
          filetype->setCreateMethod("template");
          m_filetypes.append(filetype);
        }
        filetype->setEnabled(true);
        ++it;
      }
    }
  }

  // refresh view
  refresh();
}

void FileCreatePart::slotProjectClosed() {
  m_filetypes.clear();
  refresh();
}

void FileCreatePart::slotFiletypeSelected(const FileCreateFileType * filetype) {
  KDevCreateFile::CreatedFile createdFile = createNewFile(filetype->ext(),
                                                          QString::null,
                                                          QString::null,
                                                          filetype->subtypeRef());
  if (createdFile.status == KDevCreateFile::CreatedFile::STATUS_OK) {
    KURL uu(project()->projectDirectory() + createdFile.dir + "/" + createdFile.filename );
    kdDebug(9034) << "Opening url: " << uu.prettyURL().latin1() << endl;
    partController()->editDocument ( uu );
  }
}

int FileCreatePart::readTypes(const QDomDocument & dom, bool enable) {
  int numRead = 0;
  QDomElement fileTypes = DomUtil::elementByPath(dom,"/kdevfilecreate/filetypes");
  if (!fileTypes.isNull()) {
    for(QDomNode node = fileTypes.firstChild();!node.isNull();node=node.nextSibling()) {
      if (node.isElement() && node.nodeName()=="type") {
        QDomElement element = node.toElement(); 
        FileCreateFileType * filetype = new FileCreateFileType;
        filetype->setName( element.attribute("name") );
        filetype->setExt( element.attribute("ext") );
        filetype->setCreateMethod( element.attribute("create") );
        filetype->setIcon( element.attribute("icon") );
        filetype->setDescr( (DomUtil::namedChildElement(element, "descr")).text() );
        filetype->setEnabled(enable);
        m_filetypes.append(filetype);
        numRead++;

        kdDebug(9034) << "node: " << filetype->name().latin1() << endl;
        
        if (node.hasChildNodes()) {
          for(QDomNode subnode = node.firstChild();!subnode.isNull();subnode=subnode.nextSibling()) {
            kdDebug(9034) << "subnode: " << subnode.nodeName().latin1() << endl;
            if (subnode.isElement() && subnode.nodeName()=="subtype") {
              QDomElement subelement = subnode.toElement();
              FileCreateFileType * subtype = new FileCreateFileType;
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

FileCreateFileType * FileCreatePart::getType(const QString & ex, const QString subtRef) {

  QString subtypeRef = subtRef;
  QString ext = ex;
  int dashPos = ext.find('-');
  if (dashPos>-1 && subtRef==QString::null) {
    ext = ex.left(dashPos);
    subtypeRef = ex.mid(dashPos+1);
  }

  QPtrList<FileCreateFileType> filetypes = getFileTypes();
  for(FileCreateFileType * filetype = filetypes.first();
      filetype;
      filetype=filetypes.next()) {
    if (filetype->ext()==ext) {
      if (subtypeRef==QString::null) return filetype;
      QPtrList<FileCreateFileType> subtypes = filetype->subtypes();
      for(FileCreateFileType * subtype = subtypes.first();
          subtype;
          subtype=subtypes.next()) {
        if (subtypeRef==subtype->subtypeRef()) return subtype;
      }
    }
  }
  return NULL;
}

// KDevFileCreate interface

KDevCreateFile::CreatedFile FileCreatePart::createNewFile(QString ext, QString dir, QString name, QString subtype) {
  
  KDevCreateFile::CreatedFile result;
  KURL projectURL( project()->projectDirectory() );
  KURL selectedURL;

  KDialogBase * dialogBase = NULL;
  FileCreateWidget2 * filetypeWidget = NULL;

  KFileDialog * fileDialogWidget = NULL;  
  
  // If the file type (extension) is unknown, we're going to need to ask
  if (ext==QString::null) {
    m_filedialogFiletype = NULL;
    filetypeWidget = new FileCreateWidget2(this);
    connect( filetypeWidget->signaller(), SIGNAL(filetypeSelected(const FileCreateFileType *) ) ,
             this, SLOT(slotNoteFiletype(const FileCreateFileType *)) );
    filetypeWidget->refresh();
  }

  // If the file name or path is unknown, we're going to need to ask  
  if (dir==QString::null || name==QString::null) {

    // if no path is known, start at project root
    if (dir==QString::null)
      dir=project()->projectDirectory();

    fileDialogWidget = new KFileDialog(dir, "*." + ext, 0, "New file", true, filetypeWidget);
    projectURL = fileDialogWidget->baseURL();

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
      filetypeWidget = NULL; // deleted as reparented in KFileDialog
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
  if (!relToProj.endsWith("." + ext)) relToProj+="." + ext;
  
  QString filename = URLUtil::filename(relToProj);
  
  kdDebug(9034) << "relative to proj dir = " << relToProj << endl;  
  kdDebug(9034) << "filename = " << filename << endl;

  // add in subtype, if specified
  if (subtype!=QString::null && subtype!="")
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

void FileCreatePart::slotNoteFiletype(const FileCreateFileType * filetype) {
  kdDebug(9034) << "Noting file type: " << (filetype ? filetype->ext() : QString::fromLatin1("Null") ) << endl;
  m_filedialogFiletype = filetype;
}

#include "filecreate_part.moc"
