
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
#include <kapp.h>
#include <klocale.h>
#include <qlayout.h>

/* 
 *  Constructs a FilePropsPage which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
AddExistingFilesDlg::AddExistingFilesDlg( QWidget* parent,  const char* name,QString startDir,
					  QStringList filters) : AddExistingFilesBase( parent, name, true){
  m_scanningAborted = false;
  m_pScanningDlg = new QSemiModal(this,"dlg",true);
  m_pScanningDlg->resize( 202, 93 ); 
  m_pScanningDlg->setCaption( tr( "Progress"  ) );
  QGridLayout* grid = new QGridLayout(m_pScanningDlg ); 
  grid->setSpacing( 6 );
  grid->setMargin( 11 );
  
  m_pScanningAbortButton = new QPushButton(m_pScanningDlg, "PushButton1" );
  m_pScanningAbortButton->setText( i18n( "Abort"  ) );
  
  grid->addWidget(m_pScanningAbortButton, 1, 1 );
  QSpacerItem* spacer = new QSpacerItem( 32, 20, QSizePolicy::Expanding, QSizePolicy::Fixed );
  grid->addItem( spacer, 1, 2 );
  QSpacerItem* spacer_2 = new QSpacerItem( 32, 20, QSizePolicy::Expanding, QSizePolicy::Fixed );
  grid->addItem( spacer_2, 1, 0 );
  
  QHBoxLayout* hbox = new QHBoxLayout; 
  hbox->setSpacing( 6 );
  hbox->setMargin( 0 );
  m_pScanningFixedLabel = new QLabel(m_pScanningDlg, "TextLabel1" );
  m_pScanningFixedLabel->setText( i18n( "Scanning Directories..."  ) );
  hbox->addWidget(m_pScanningFixedLabel );

  m_pScanningCountLabel = new QLabel( m_pScanningDlg, "TextLabel2" );
  m_pScanningCountLabel->setText( tr( "2342"  ) );
  hbox->addWidget( m_pScanningCountLabel );
  grid->addMultiCellLayout( hbox, 0, 0, 0, 2 );
  m_pScanningDlg->hide();
  
  connect(m_pScanningAbortButton,SIGNAL(clicked()),SLOT(scanAbortedClicked()));

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
  if(directory.right(1) != "/"){
    directory = directory + "/";
  }
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
  QStringList dirs;
  if(dlg.exec()){
    QString directory = dlg.directory();
    dirs.append(directory);
    directory = directory.remove(directory.length()-1,1); // removes last /, need in scanDir
    m_pScanningFixedLabel->setText( i18n( "Scanning Directories..."  ) );
    m_pScanningDlg->show();
    QString filter = dlg.filter();
    if(dlg.recursive()){  // add files recursive
      m_Count = 0;
      m_scanningAborted=false;
      scanDir(directory,dirs);
      if(m_scanningAborted){
	return; // abort
      }
    }
    
    if(dirs.contains(m_currentDir) !=0) {
      m_pFilesListbox->clear();
    }
    m_Count = 0;
    m_scanningAborted=false;
    m_pScanningFixedLabel->setText( i18n( "Adding Files..."  ) );
    QDir dir;
    QString file;
    dir.setNameFilter(filter);
    dir.setFilter(QDir::Files);
    
    for ( QStringList::Iterator dirIt = dirs.begin(); dirIt != dirs.end(); ++dirIt ) {
      if(m_scanningAborted) continue;
      kapp->processEvents();
      dir.setPath(*dirIt);
      QStringList files = dir.entryList();
      for ( QStringList::Iterator fileIt = files.begin(); fileIt != files.end(); ++fileIt ) {
	file = (*dirIt) + (*fileIt);
	if(m_addedFiles.contains(file) == 0){ // add only if not in the addlist
	  m_addedFiles.append(file);
	  m_pAddedFilesListbox->insertItem(KMimeType::pixmapForURL( file,0,KIcon::Small),file);
	  m_pScanningCountLabel->setText(QString::number(m_Count++));
	  if(m_scanningAborted) continue;
	}
      }
      
    } // end for
    m_pScanningDlg->hide();
  } // end if dlg.exec
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

void AddExistingFilesDlg::scanDir(const QString& directory,QStringList& dirs) {
  if (m_scanningAborted)
    return;
  // taken from RFV (KDevelop1)
  QString currentPath;
  QStringList dirList;
  QDir dir(directory);
  m_pScanningCountLabel->setText(QString::number(m_Count++));
  kapp->processEvents();
  // Stop recursion if the directory doesn't exist.
  if (!dir.exists()) {
    return;
  }
  
  dir.setSorting(QDir::Name);
  dir.setFilter(QDir::Dirs);
  dirList = dir.entryList();
  
  // Remove '.' and  '..'
  dirList.remove(dirList.begin());
  dirList.remove(dirList.begin());
  
  // Recurse through all directories
  QStringList::Iterator it;
  for( it = dirList.begin(); it != dirList.end(); ++it){
    currentPath = directory+"/"+(*it);
    dirs.append(currentPath);    
    // Recursive call to fetch subdirectories
    scanDir( currentPath, dirs);
  }
}

void AddExistingFilesDlg::scanAbortedClicked(){
  m_scanningAborted = true;
  m_pScanningDlg->hide();
}
#include "addexistingfilesdlg.moc"
