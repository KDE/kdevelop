
#include "addexistingfilesdlg.h"
#include "flatdirtreeview.h"
#include <qlistbox.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include <iostream.h>
#include <kmimetype.h>
#include <qpushbutton.h>
#include "addtreedlg.h"

/* 
 *  Constructs a FilePropsPage which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
AddExistingFilesDlg::AddExistingFilesDlg( QWidget* parent,  const char* name,QString startDir,
					  QStringList filters) : AddExistingFilesBase( parent, name, true){
  
  m_filters =  filters;
  m_pFileFilterComboBox->insertStringList(filters);
  connect(m_pDirView,SIGNAL(dirSelected(QString)),this,SLOT(slotDirectorySelected(QString)));
  connect(m_pFileFilterComboBox,SIGNAL(activated(const QString&)),
	  this,SLOT(slotComboBoxActivated(const QString&)));
  connect(m_pAddButton,SIGNAL(clicked()),this,SLOT(slotAddClicked()));
  connect(m_pAddAllButton,SIGNAL(clicked()),this,SLOT(slotAddAllClicked()));
  connect(m_pAddTreeButton,SIGNAL(clicked()),this,SLOT(slotAddTreeClicked()));
  connect(m_pAddInvertButton,SIGNAL(clicked()),m_pFilesListbox,SLOT(invertSelection()));
  connect(m_pRemoveButton,SIGNAL(clicked()),this,SLOT(slotRemoveClicked()));
  connect(m_pRemoveAllButton,SIGNAL(clicked()),this,SLOT(slotRemoveAllClicked()));
  connect(m_pRemoveInvertButton,SIGNAL(clicked()),m_pAddedFilesListbox,SLOT(invertSelection()));

  m_pDirView->setDirLocation(startDir);
  slotDirectorySelected(startDir);
}

/*  
 *  Destroys the object and frees any allocated resources
 */
AddExistingFilesDlg::~AddExistingFilesDlg(){
    // no need to delete child widgets, Qt does it all for us
}


void AddExistingFilesDlg::slotDirectorySelected(QString directory){
  m_currentDir = directory;
  QDir dir(m_currentDir);
  dir.setNameFilter(m_pFileFilterComboBox->currentText());
  dir.setFilter(QDir::Files);
  QStringList files = dir.entryList();
  m_pFilesListbox->clear();
  for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
    if(m_addedFiles.contains(m_currentDir + (*it)) == 0){ // add only if not in the addlist
      m_pFilesListbox->insertItem(KMimeType::pixmapForURL( m_currentDir + (*it),0,KIcon::Small),*it);
    }
  }
}

void AddExistingFilesDlg::slotComboBoxActivated(const QString&){
  slotDirectorySelected(m_currentDir);
}

void AddExistingFilesDlg::slotAddClicked(){
  QStringList files;
  QListBoxItem* pItem=0;
  QString file;
  for( pItem= m_pFilesListbox->firstItem();pItem != 0;pItem = pItem->next()){
    if(pItem->selected()){
      files.append(pItem->text());
    }  
  }
 
  for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
    pItem = m_pFilesListbox->findItem((*it));// possible bug because case-insensitive search :-(
    if(pItem !=0){
      m_pFilesListbox->removeItem(m_pFilesListbox->index(pItem));
    }
    QString file = m_currentDir + (*it);
    m_addedFiles.append(file);
    m_pAddedFilesListbox->insertItem(KMimeType::pixmapForURL( file,0,KIcon::Small),file);
  }
}
void AddExistingFilesDlg::slotAddAllClicked(){
  QStringList files;
  QListBoxItem* pItem=0;
  QString file;
  for( pItem= m_pFilesListbox->firstItem();pItem != 0;pItem = pItem->next()){
      files.append(pItem->text());
  }
  m_pFilesListbox->clear();
  for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
    QString file = m_currentDir + (*it);
    m_addedFiles.append(file);
    m_pAddedFilesListbox->insertItem(KMimeType::pixmapForURL( file,0,KIcon::Small),file);
  }
  
}
void AddExistingFilesDlg::slotAddTreeClicked(){
  AddTreeDlg dlg(this,"add",m_currentDir,m_filters);
  dlg.show();

}
void AddExistingFilesDlg::slotRemoveClicked(){
  QStringList files;
  QListBoxItem* pItem=0;
  QString file;
  for( pItem= m_pAddedFilesListbox->firstItem();pItem != 0;pItem = pItem->next()){
    if(pItem->selected()){
      files.append(pItem->text());
    } 
  }
 
  for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
    pItem = m_pAddedFilesListbox->findItem((*it));// possible bug because case-insensitive search :-(
    if(pItem !=0){
      m_pAddedFilesListbox->removeItem(m_pAddedFilesListbox->index(pItem));
    }
    m_addedFiles.remove(*it);
  }
  slotDirectorySelected(m_currentDir);
}
void AddExistingFilesDlg::slotRemoveAllClicked(){
  QStringList files;
  QListBoxItem* pItem=0;
  QString file;
  for( pItem= m_pAddedFilesListbox->firstItem();pItem != 0;pItem = pItem->next()){
      files.append(pItem->text());
  }
  m_pAddedFilesListbox->clear();
  for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
    pItem = m_pAddedFilesListbox->findItem((*it));// possible bug because case-insensitive search :-(
    if(pItem !=0){
      m_pAddedFilesListbox->removeItem(m_pAddedFilesListbox->index(pItem));
    }
    m_addedFiles.remove(*it);
  }
  slotDirectorySelected(m_currentDir);
}
QStringList AddExistingFilesDlg::addedFiles(){
  return m_addedFiles;
}
#include "addexistingfilesdlg.moc"
