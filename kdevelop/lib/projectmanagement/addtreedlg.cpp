#include "addtreedlg.h"
#include "flatdirtreeview.h"
#include <qcombobox.h>
#include <iostream.h>
#include <qcheckbox.h>

/* 
 *  Constructs a AddTreeDlg which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AddTreeDlg::AddTreeDlg( QWidget* parent,  const char* name,QString startDir,QStringList filters)
    : AddTreeDlgBase( parent, name, true){
  m_pFilterComboBox->insertStringList(filters);
  m_pDirView->setDirLocation(startDir);
}

/*  
 *  Destroys the object and frees any allocated resources
 */
AddTreeDlg::~AddTreeDlg()
{
    // no need to delete child widgets, Qt does it all for us
}
bool AddTreeDlg::recursive(){
  return m_pRecursiveCheckbox->isChecked();
}
QString AddTreeDlg::directory(){
  return m_pDirView->currentDir();
}
QString AddTreeDlg::filter(){
  return m_pFilterComboBox->currentText();
}

#include "addtreedlg.moc"
