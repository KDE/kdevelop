/***************************************************************************
                          cclonefunctiondlg.cpp  -  description
                             -------------------
    begin                : Sat Nov 18 2000
    copyright            : (C) 2000 by The KDevelop Team
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cclonefunctiondlg.h"
#include "./classparser/ParsedClass.h"
#include "cclassview.h"
#include <klocale.h>
#include <kmessagebox.h>
//#include <kapp.h>

CCloneFunctionDlg::CCloneFunctionDlg(CClassView* class_tree, QWidget *parent, const char *name )
  : QDialog(parent,name,true),
    topLayout( this, 5 ),
    functionLayout( 5, "functions" ),
    buttonLayout( 5, "buttons" ),
	  allclasses(FALSE, this, "classes"),
	  methods(FALSE, this, "methods"),
    okBtn( this, "okBtn" ),
    cancelBtn( this, "cancelBtn" ),
    classtree(class_tree)
{
	// set up dialog
  setCaption( i18n("Select function to copy") );

  topLayout.addLayout( &functionLayout );
  topLayout.addLayout( &buttonLayout );

  allclasses.setFixedSize( 400, 30 );
  methods.setFixedSize( 400, 30 );
  //allclasses.setAutoResize(TRUE);
  //methods.setAutoResize(TRUE);

  functionLayout.addWidget(&allclasses);
  functionLayout.addWidget(&methods);

  // Buttons
  //okBtn.setGeometry( 10, 370, 100, 30 );
  okBtn.setFixedSize( 100, 30 );
  okBtn.setText( i18n("OK") );
  okBtn.setDefault( TRUE );

  //cancelBtn.setGeometry( 170, 370, 100, 30 );
  cancelBtn.setFixedSize( 100, 30 );
  cancelBtn.setText( i18n("Cancel") );
  cancelBtn.setAutoRepeat( FALSE );
  cancelBtn.setAutoResize( FALSE );

   // Button layout
  buttonLayout.addWidget( &okBtn );
  buttonLayout.addWidget( &cancelBtn );

   // Ok and cancel buttons.
  connect( &okBtn, SIGNAL( clicked() ), SLOT( OK() ) );
  connect( &cancelBtn, SIGNAL( clicked() ), SLOT( reject() ) );

  // populate the comboboxes
  // look up parent
  QString parentname;
  CParsedClass* curr = classtree->getCurrentClass();
	if ( curr && curr->parents.first() )
		parentname = curr->parents.first()->name;

	// create list of all classes
  QList<CParsedClass>* all = classtree->store->getSortedClassList();
	for (CParsedClass* i=all->first(); i != 0; i=all->next() ) {
		allclasses.insertItem(i->name);
	  if (i->name == parentname)
	  	allclasses.setCurrentItem(allclasses.count()-1);
	}
	delete (all);
	slotNewClass( allclasses.currentText () );
				
	// change methods on class selection
	connect(&allclasses, SIGNAL(highlighted(const QString&)),
			SLOT(slotNewClass(const QString&)) );
					
	// Set the default focus.
  allclasses.setFocus();
}


void CCloneFunctionDlg::OK()
{
 if( strlen( methods.currentText() ) == 0 )
    KMessageBox::information( this,
                            i18n("You have to select a method."),
                            i18n("No method") );
 else
    accept();
}

/** update methods/slots */
void CCloneFunctionDlg::slotNewClass(const QString& name)
{
	QString str;
	
	// all Methods
	QList<CParsedMethod> *implList = new QList<CParsedMethod>;
	QList<CParsedMethod> *availList = new QList<CParsedMethod>;
  CParsedClass *theClass = classtree->store->getClassByName( name );
  QList<CParsedMethod> *all = theClass->getSortedMethodList();

	methods.clear();
  for(CParsedMethod* i = all->first(); i != 0; i=all->next() )
  	if (! (i->isDestructor || i->isConstructor))
      methods.insertItem(i->asString(str));
	delete (all);
	delete (implList);
	delete (availList);

	// all slots
  all = theClass->getSortedSlotList();
  for(CParsedMethod* i = all->first(); i != 0; i=all->next() )
      methods.insertItem(i->asString(str));
	delete (all);
	
  // all signals
  all = theClass->getSortedSignalList();
  for(CParsedMethod* i = all->first(); i != 0; i=all->next() )
      methods.insertItem(i->asString(str));
	delete (all);
}
/** get the selected method */
CParsedMethod* CCloneFunctionDlg::getMethod(){
 	QString str;
  QString selected = methods.currentText();
  CParsedClass *theClass = classtree->store->getClassByName( allclasses.currentText() );
  QList<CParsedMethod> *all = theClass->getSortedMethodList();

  // check methods
  for(CParsedMethod* i = all->first(); i != 0; i=all->next() )
      if ( selected == i->asString(str)) {
	        delete (all);
	        return i;
	    }
	delete (all);
	
	// not found - try all slot
  all = theClass->getSortedSlotList();
  for(CParsedMethod* i = all->first(); i != 0; i=all->next() )
     if ( selected == i->asString(str)) {
	        delete (all);
	        return i;
	    }
	delete (all);
	
	// not found - try all slot
  all = theClass->getSortedSignalList();
  for(CParsedMethod* i = all->first(); i != 0; i=all->next() )
     if ( selected == i->asString(str)) {
	        delete (all);
	        return i;
	    }

	// oops 	
	delete (all);
	return NULL;									
}
#include "cclonefunctiondlg.moc"
