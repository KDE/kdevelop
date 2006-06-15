#include "filepropspage.h"
#include <qlistbox.h>
#include <klineedit.h>
#include <qtextview.h>
#include <klocale.h>
#include <qlabel.h>
#include <qmultilineedit.h>

/* 
 *  Constructs a FilePropsPage which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
FilePropsPage::FilePropsPage( QWidget* parent,  const char* name, WFlags fl )
    : FilePropsPageBase( parent, name, fl ){
  m_props = new QPtrList<ClassFileProp>;
  m_current_class = 9999; // no current  
}

/*  
 *  Destroys the object and frees any allocated resources
 */
FilePropsPage::~FilePropsPage()
{
    // no need to delete child widgets, Qt does it all for us
}

/* 
 * public slot
 */
void FilePropsPage::slotSelectionChanged()
{
  int item = classes_listbox->currentItem();
  ClassFileProp* prop;
  // save the old values
  if (m_current_class != 9999){ // != no selected
    prop = m_props->at(m_current_class);
    prop->m_classname = classname_edit->text();
    prop->m_headerfile = headerfile_edit->text();
    if(m_different_header_impl){
      prop->m_implfile = implfile_edit->text();
    }
    if(prop->m_change_baseclass){
      prop->m_baseclass = baseclass_edit->text();
    }
  }
  prop = m_props->at(item);
  classname_edit->setText(prop->m_classname);
  desc_textview->setText(prop->m_description);
  headerfile_edit->setText(prop->m_headerfile);
  if(m_different_header_impl){
    implfile_edit->setText(prop->m_implfile);
  }
  baseclass_edit->setText(prop->m_baseclass);
  if(prop->m_change_baseclass){
    baseclass_edit->setEnabled(true);
  }
  else {
    baseclass_edit->setEnabled(false);
  }
  m_current_class = item;
}

void FilePropsPage::setClassFileProps(QPtrList<ClassFileProp> props,bool different_header_impl){
  *m_props = props;
  m_different_header_impl = different_header_impl;
  if (!m_different_header_impl){
    implfile_edit->hide();
    implfile_label->hide();
    headerfile_label->setText(i18n("Header/Implementation file:"));
    
  }
  // fill the listbox
  ClassFileProp* prop;
  for ( prop=m_props->first(); prop != 0; prop=m_props->next() ){
    classes_listbox->insertItem(prop->m_classname);
  }
  classes_listbox->setSelected(0,true);
  slotSelectionChanged();
}

QPtrList<ClassFileProp> FilePropsPage::getClassFileProps(){
  return *m_props;
}
void FilePropsPage::slotClassnameChanged(const QString& text){
  classes_listbox->changeItem(text, classes_listbox->currentItem());
}

#include "filepropspage.moc"
