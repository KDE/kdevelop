/***************************************************************************
                          cppaddmethoddlg.cpp  -  description
                             -------------------
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@cenacle.se
    copyright            : (C) 2001 by August Hörandl
    email                : august.hoerandl@gmx.at
 ***************************** **********************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cppaddmethoddlg.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <qregexp.h>

#include <klineedit.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include "classstore.h"
#include "parsedmethod.h"
#include "cclonefunctiondlg.h"

//#define TEST_DEBUG_ONLY  1

static const char EQU[] = " = ";
static const int EQULEN = sizeof(EQU)-1;
static const char COMM[] = " // ";
static const int COMMLEN = sizeof(COMM)-1;


CppAddMethodDialog::CppAddMethodDialog(ClassStore *_store, ClassStore *_libstore, const QString &className,
                                       QWidget *parent, const char *name)
    : CppAddMethodDialogBase(parent, name, true), 
    store(_store), libstore(_libstore), currentClass(className), 
    editactive(false), comp(0)
{
  connect( rbMethod, SIGNAL( clicked() ), SLOT( slotToggleModifier() ) );
  connect( rbSlot, SIGNAL( clicked() ), SLOT( slotToggleModifier() ) );
  connect( rbSignal, SIGNAL( clicked() ), SLOT( slotToggleModifier() ) );
  connect( rbKonstruktor, SIGNAL( clicked() ), SLOT( slotToggleModifier() ) );
  connect( cbVirtual, SIGNAL ( clicked() ), SLOT( slotVirtualClicked() ) );

  connect( pbNew, SIGNAL( clicked() ), SLOT( slotNewPara() ) );
  connect( pbDelete, SIGNAL( clicked() ), SLOT( slotDelPara() ) );
  connect( pbMoveUp, SIGNAL( clicked() ), SLOT( slotUpPara() ) );
  connect( pbMoveDown, SIGNAL( clicked() ), SLOT( slotDownPara() ) );

  connect( paraName, SIGNAL( textChanged(const QString&) ), 
	   SLOT( slotUpdateParameter(const QString&) ));
  connect( paraType, SIGNAL( textChanged(const QString&) ), 
	   SLOT( slotUpdateParameter(const QString&) ));
  connect( paraDefault, SIGNAL( textChanged(const QString&) ), 
	   SLOT( slotUpdateParameter(const QString&) ));
  connect( paraDocu, SIGNAL( textChanged(const QString&) ), 
	   SLOT( slotUpdateParameter(const QString&) ));
  connect( lbPara, SIGNAL( selectionChanged(QListBoxItem*) ), 
	   SLOT( slotParaHighLight(QListBoxItem*) ));

  connect( pbClone, SIGNAL( clicked() ), SLOT( slotClone() ) );

  connect( pbOk, SIGNAL( clicked() ), SLOT( accept() ) );
  connect( pbCancel, SIGNAL( clicked() ), SLOT( reject() ) );

  GroupItem->setEnabled(false);
  
  setCompletion();
  
#ifdef TEST_DEBUG_ONLY
  // test only
  pbHelp->setEnabled(true);
  connect( pbHelp, SIGNAL( clicked() ), SLOT( slotDebug() ) );
#endif
}


CppAddMethodDialog::~CppAddMethodDialog()
{
  delete comp;
}

void CppAddMethodDialog::setCompletion()
{
  // create completion
  comp = edType->completionObject();
  paraType->setCompletionObject( comp );
  
  comp->addItem("void");
  comp->addItem("int");
  comp->addItem("long int");
  comp->addItem("unsigned int");
  comp->addItem("unsigned long int");
  comp->addItem("float");
  comp->addItem("double");
  comp->addItem("char");
  comp->addItem("bool");

  QValueList<ParsedClass*> classlist = store->getSortedClassList();
  QValueList<ParsedClass*>::iterator it;
  for ( it = classlist.begin(); it != classlist.end(); ++it )
    comp->addItem((*it)->name());
  classlist = libstore->getSortedClassList();
  for ( it = classlist.begin(); it != classlist.end(); ++it )
    comp->addItem((*it)->name());
}

ParsedMethod *CppAddMethodDialog::asSystemObj()
{
  ParsedMethod *aMethod = new ParsedMethod();

  // return type
  aMethod->setType( edType->text() );
  // name + parameters
  aMethod->setName( getDecl() );

  // Set the type.
  if( rbSlot->isChecked() )
    aMethod->setIsSlot( true );
  else if( rbSignal->isChecked() )
    aMethod->setIsSignal( true );

  // Set the export.
  if( rbPublic->isChecked() )
    aMethod->setAccess( PIE_PUBLIC );
  else if( rbProtected->isChecked() )
    aMethod->setAccess( PIE_PROTECTED );
  else if( rbPrivate->isChecked() )
    aMethod->setAccess( PIE_PRIVATE );

  // Set the modifiers if they are enabled.
  if( cbPure->isChecked() )
    aMethod->setIsPure( true );
  if( cbStatic->isChecked() )
    aMethod->setIsStatic( true );
  if( cbConst->isChecked() )
    aMethod->setIsConst( true );
  if( cbVirtual->isChecked())
    aMethod->setIsVirtual( true );

  // Set comment
  aMethod->setComment( getDocu() );

  return aMethod;
}


void CppAddMethodDialog::slotToggleModifier()
{
  //reset
  edType->setEnabled(true);
  edType->clear();
  edName->clear();
  //rbPublic->setChecked(true);

  if (rbSignal->isChecked())
  {
    rbPublic->setEnabled(false);
    rbPrivate->setEnabled(false);
    rbProtected->setChecked(true);
  }
  else
  {
    rbPublic->setEnabled(true);
    rbPrivate->setEnabled(true);
  }
  if (rbSlot->isChecked() || rbSignal->isChecked())
  {
    cbStatic->setEnabled(false);
    cbConst->setEnabled(false);
    cbVirtual->setEnabled( rbSlot->isChecked() );
    cbPure->setEnabled(false);
    edType->setText("void");
    if ( rbSlot->isChecked() && edName->text().isEmpty())
      edName->setText("slot");
  }
  else
  {
    cbConst->setEnabled(true);
    cbVirtual->setEnabled(true);
    slotVirtualClicked();
  }
  if (rbKonstruktor->isChecked())
  {
    cbStatic->setEnabled(false);
    cbConst->setEnabled(false);
    cbVirtual->setEnabled(false);
    edType->setEnabled(false);
  }
}


void CppAddMethodDialog::slotVirtualClicked()
{
  cbPure->setEnabled( cbVirtual->isChecked() );
  cbStatic->setEnabled( !cbVirtual->isChecked() );
}


void CppAddMethodDialog::accept()
{
  if (!rbKonstruktor->isChecked())
  {
    if (edType->text().isEmpty())
    {
      KMessageBox::sorry(this, i18n("You have to specify a type."), i18n("No type") );
      edType->setFocus();
      return;
    }
  }

  if (edName->text().isEmpty())
  {
    KMessageBox::sorry(this, i18n("You have to specify a name."), i18n("No name") );
    edName->setFocus();
    return;
  }
  QDialog::accept();
}


/** add new Paramter */
void CppAddMethodDialog::slotNewPara()
{
  if (!GroupItem->isEnabled()) GroupItem->setEnabled(true);
  lbPara->insertItem(" ");
  lbPara->setSelected(lbPara->count()-1, true);
  paraType->setFocus();
}


/** delete Paramter */
void CppAddMethodDialog::slotDelPara()
{
  int curr = lbPara->currentItem();
  editactive = true;
  if (curr >= 0)
  {
    lbPara->removeItem(curr);
    paraType->clear();
    paraName->clear();
    paraDefault->clear();
    paraDocu->clear();
    GroupItem->setEnabled(false);
  }
  editactive = false;
}


/** update current parameter in listbox */
void CppAddMethodDialog::slotUpdateParameter(const QString& s)
{
  kdDebug() << "slot UpdatePara called "<< editactive << " " << s << endl;
  if (! editactive)
  {
    editactive = true;
    QString p(paraType->text() + " " + paraName->text());
    if (! paraDefault->text().isEmpty())
      p += EQU + paraDefault->text();
    // p= p.simplifyWhiteSpace().stripWhiteSpace();
    if (! paraDocu->text().isEmpty())
      p += COMM + paraDocu->text();
    lbPara->changeItem(p, lbPara->currentItem());
    editactive = false;
  }
}


/** move Paramter up*/
void CppAddMethodDialog::slotUpPara()
{
  int curr = lbPara->currentItem();
  if (curr > 0)
  {
    QString txt = lbPara->currentText();
    lbPara->removeItem(curr);
    --curr;
    lbPara->insertItem(txt, curr);
    lbPara->setCurrentItem(curr);
  }
}


/** move Paramter down */
void CppAddMethodDialog::slotDownPara()
{
  int curr = lbPara->currentItem();
  if (curr >= 0 && curr < (int)lbPara->count()-1)
  {
    QString txt = lbPara->currentText();
    lbPara->removeItem(curr);
    curr++;
    lbPara->insertItem(txt, curr);
    lbPara->setCurrentItem(curr);
  }
}


/** click onto Parameter */
void CppAddMethodDialog::slotParaHighLight( QListBoxItem * )
{
  kdDebug() << "slot ParaHighlight called "<< editactive << endl;
  if (! editactive)
  {
    if(!(GroupItem->isEnabled())) GroupItem->setEnabled(true);
    editactive = true;
    QString txt = lbPara->currentText();
    int docupos = txt.find(COMM);
    if (docupos != -1)
    {
      QString d = txt.mid(docupos+COMMLEN);
      if (! d.isEmpty() )
        paraDocu->setText(d);
      txt.truncate(docupos);
    }
    else
      paraDocu->setText("");
    int defpos = txt.find(EQU);
    if (defpos != -1)
    {
      QString d = txt.mid(defpos+EQULEN);
      if (! d.isEmpty() )
        paraDefault->setText(d);
      txt.truncate(defpos);
    }
    else
      paraDefault->setText("");

    int namepos = txt.findRev(" ");
    if (namepos != -1)
    {
      QString n(txt.mid(namepos+1).stripWhiteSpace());
      if (! n.isEmpty())
        paraName->setText(n);
      else
        paraName->setText("");
      txt.truncate(namepos);
      paraType->setText(txt.stripWhiteSpace());
    }
    else
    {
      // TODO: ???
      paraName->setText(txt);
      paraType->setText("");
    }
    editactive = false;
  }
}


/** Debug only - show Message boxes with values */
void CppAddMethodDialog::slotDebug()
{
#ifdef TEST_DEBUG_ONLY
  KMessageBox::sorry(this, getDecl(), "getDecl()" );
  KMessageBox::sorry(this, getDocu(), "getDocu()" );
#endif
}


/** return name + parameters */
QString CppAddMethodDialog::getDecl()
{

  QString decl = edName->text() + "(";
  QString sep;
  for(QListBoxItem* i = lbPara->firstItem(); i != 0; i=i->next())
  {
    QString txt = i->text();
    int docupos = txt.find(COMM);
    if (docupos != -1)
      txt.truncate(docupos);
    decl += sep + txt;
    sep = ", ";
  }
  decl += ")";
  return decl;
}


/** return documentation string */
QString CppAddMethodDialog::getDocu()
{
  QString docu = edDocs->text();
  for(QListBoxItem* i = lbPara->firstItem(); i != 0; i=i->next())
  {
    QString txt = i->text();
    int docupos = txt.find(COMM);
    if (docupos != -1)
    {
      QString name(txt);
      int defpos = name.find(EQU);
      if (defpos != -1)
        name.truncate(defpos);
      int namepos = name.findRev(" ");
      docu += "\n\t@param " + name.mid(namepos+1) + " " + txt.mid(docupos+COMMLEN);
    }
  }
  return docu;
}


/** clone a function */
void CppAddMethodDialog::slotClone()
{
  // thats the real one
  CCloneFunctionDlg cloneDlg(store, libstore, currentClass, this, "cloneDlg");
  if (cloneDlg.exec())
  {
    QString type, name, decl, str;
    bool isPrivat, isProtected, isPublic, isvirtual, isSignal, isSlot, isConst;

    if (! cloneDlg.getMethod(type, name, decl, str, 
			     isPrivat, isProtected, isPublic, isvirtual, isSlot, isSignal, isConst ))
      return;
    // copy type and declaration
    edType -> setText(type.simplifyWhiteSpace().stripWhiteSpace());

    // search for paramters
    int parastart = decl.findRev('(');
    int paraend = decl.find(')', parastart);
    edName -> setText(name.simplifyWhiteSpace().stripWhiteSpace());
    QString para(decl.mid(parastart+1, paraend-parastart-1));
    para.replace( QRegExp("\\s*=\\s*"), EQU );
    para = para.simplifyWhiteSpace().stripWhiteSpace();
    lbPara->clear();
    lbPara->insertStringList(QStringList::split(',', para));

    // the comment needs some adjustment
    // remove /** and */
    str.replace( QRegExp("^/\\**\\s*"), "" );
    str.replace( QRegExp("\\s*\\**/$"), "" );
    // clean up line breaks
    str.replace( QRegExp("\n\\s*\\**\\s*"), "\n" );
    edDocs -> setText(str);

    // all the buttons
    rbPrivate->setChecked( isPrivat );
    rbProtected->setChecked( isProtected );
    rbPublic->setChecked( isPublic );

    rbMethod->setChecked( true );
    rbSlot->setChecked( isSlot );
    rbSignal->setChecked( isSignal );
    // virtual
    cbVirtual -> setChecked( isvirtual );
    slotVirtualClicked();
    // const
    cbConst -> setChecked( isConst );
  }
}

#include "cppaddmethoddlg.moc"
