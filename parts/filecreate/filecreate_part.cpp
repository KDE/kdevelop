#include <qwhatsthis.h>
#include <qdom.h>
#include <qdir.h>
#include <qfileinfo.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kfiledialog.h>

#include <iostream>

#include "kdevcore.h"
#include "kdevtoplevel.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"

#include "filetemplate.h"
#include "domutil.h"
#include "urlutil.h"

#include "filecreate_widget.h"
#include "filecreate_part.h"
#include "filecreate_filetype.h"

typedef KGenericFactory<FileCreatePart> FileCreateFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevfilecreate, FileCreateFactory( "kdevfilecreate" ) );

FileCreatePart::FileCreatePart(QObject *parent, const char *name, const QStringList& )
  : KDevPlugin(parent, name)
{
  setInstance(FileCreateFactory::instance());
  setXMLFile("kdevpart_filecreate.rc"); 

  m_widget = new FileCreateWidget(this);
  m_filetypes.setAutoDelete(true);
  QWhatsThis::add(m_widget, i18n("This part makes the creation of new files within the project easier."));

  connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
  connect( core(), SIGNAL(projectClosed()), this, SLOT(slotProjectClosed()) );
  connect( m_widget, SIGNAL(filetypeSelected(const FileCreateFileType *)), this, SLOT(slotFiletypeSelected(const FileCreateFileType *)) );
  std::cout << "filecreate constructor called" << std::endl;
  
  topLevel()->embedSelectView(m_widget, i18n("New File"), i18n("file creation"));
  
}


FileCreatePart::~FileCreatePart()
{
  if (m_widget) delete m_widget;
}

void FileCreatePart::slotProjectOpened() {
  KDevProject * proj = project();
  QDomDocument & dom = *projectDom();

  // read in the list of file types for this project
  QDomElement fileTypes = DomUtil::elementByPath(dom,"/kdevfilecreate/filetypes");
  // if file types are specified in the project file, use them
  if (!fileTypes.isNull()) {
    for(QDomNode node = fileTypes.firstChild();!node.isNull();node=node.nextSibling()) {
      std::cout << "found node..." << node.nodeName() << std::endl;
      if (node.isElement() && node.nodeName()=="type") {
        QDomElement element = node.toElement(); 
        FileCreateFileType * filetype = new FileCreateFileType;
        filetype->setName( element.attribute("name") );
        filetype->setExt( element.attribute("ext") );
        filetype->setCreateMethod( element.attribute("create") );
        m_filetypes.append(filetype);

        std::cout << "node: " << filetype->name().latin1() << std::endl;
        
        if (node.hasChildNodes()) {
          for(QDomNode subnode = node.firstChild();!subnode.isNull();subnode=subnode.nextSibling()) {
            std::cout << "subnode: " << subnode.nodeName().latin1() << std::endl;
            if (subnode.isElement() && subnode.nodeName()=="subtype") {
              QDomElement subelement = subnode.toElement();
              FileCreateFileType * subtype = new FileCreateFileType;
              subtype->setExt( filetype->ext() );
              subtype->setCreateMethod( filetype->createMethod() );
              subtype->setSubtypeRef( subelement.attribute("ref") );
              subtype->setName( subelement.attribute("name") );
              filetype->addSubtype(subtype);
            }
          }
        }
      }
    }
  }

  if (m_filetypes.count()==0) {
    // otherwise, default by scanning the templates directory
    QDir templDir( project()->projectDirectory() + "/templates/" );
    templDir.setFilter( QDir::Files );
    const QFileInfoList * list = templDir.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo *fi;
    while ( (fi = it.current()) != 0 ) {
      FileCreateFileType * filetype = new FileCreateFileType;
      filetype->setName( fi->fileName() + " files" );
      filetype->setExt( fi->fileName() );
      filetype->setCreateMethod("template");
      m_filetypes.append(filetype);
      ++it;
    }
  }

  // refresh view
  m_widget->refresh();
}

void FileCreatePart::slotProjectClosed() {
  m_filetypes.clear();
  m_widget->refresh();
}

void FileCreatePart::slotFiletypeSelected(const FileCreateFileType * filetype) {
  KDevCreateFile::CreatedFile createdFile = createNewFile(filetype->ext(),
                                                          QString::null,
                                                          QString::null,
                                                          filetype->subtypeRef());
  if (createdFile.status == KDevCreateFile::CreatedFile::STATUS_OK) {
    KURL uu(project()->projectDirectory() + createdFile.dir + "/" + createdFile.filename );
    std::cout << "Opening url: " << uu.prettyURL().latin1() << std::endl;
    partController()->editDocument ( uu );
  }
}

// KDevFileCreate interface

KDevCreateFile::CreatedFile FileCreatePart::createNewFile(QString ext, QString dir, QString name, QString subtype) {
  KDevCreateFile::CreatedFile result;
  if (ext==QString::null) { result.status = KDevCreateFile::CreatedFile::STATUS_NOTCREATED; return result; }
  
  if (dir==QString::null)
    dir=project()->projectDirectory();
  
  //QString filename = KFileDialog::getSaveFileName(dir, "*." + ext);
  KFileDialog fd(dir, "*." + ext, 0, "New file", true);
  KURL baseURL = fd.baseURL();
  std::cout << "Base URL= " << baseURL.prettyURL().latin1() << std::endl;

  int fdResult = fd.exec();
  if (fdResult==QDialog::Rejected) {
    result.status = KDevCreateFile::CreatedFile::STATUS_NOTCREATED;
    return result;
  }

  KURL selectedURL = fd.selectedURL();
  if (!baseURL.isParentOf(selectedURL)) {
    result.status = KDevCreateFile::CreatedFile::STATUS_NOTWITHINPROJECT;
    return result;
  }

  
  QString relToProj = URLUtil::relativePath(baseURL, selectedURL, true /* include prefix slash */);
  if (!relToProj.endsWith("." + ext)) relToProj+="." + ext;
  
  QString filename = URLUtil::filename(relToProj);
  
  std::cout << "relative to proj dir = " << relToProj << std::endl;  
  std::cout << "filename = " << filename << std::endl;

  if (subtype!=QString::null && subtype!="")
    ext += "-" + subtype;
    
  if (FileTemplate::exists(this, ext)) {
    FileTemplate::copy(this, ext, project()->projectDirectory() + relToProj);
    project()->addFile(relToProj.mid(1));
  }


  result.filename = filename;
  result.dir = URLUtil::directory(relToProj);
  result.status = KDevCreateFile::CreatedFile::STATUS_OK;
  return result;  
  
}



#include "filecreate_part.moc"
