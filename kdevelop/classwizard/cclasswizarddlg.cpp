/***************************************************************************
               cclasstooldlg.cpp  -  implementation

                             -------------------

    begin                : Fri Mar 19 1999

    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@cenacle.se

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kapp.h>
#include <qtooltip.h>
#include "cclasswizarddlg.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

CClassWizardDlg::CClassWizardDlg( QWidget *parent, const char *name )
  : QTabDialog( parent, name, true ),
    virtualMethodView( this, "virtualMethodView" )
{
  store = NULL;

  setCaption( i18n("ClassWizard Tool") );

  setWidgetValues();
  setCallbacks();
  setTooltips();
}

CClassWizardDlg::~CClassWizardDlg()
{
}

/*********************************************************************
 *                                                                   *
 *                          PRIVATE METHODS                          *
 *                                                                   *
 ********************************************************************/

void CClassWizardDlg::setWidgetValues()
{
  // Add the tabs
  addTab( &virtualMethodView,i18n("Virtual functions"));
}

void CClassWizardDlg::setTooltips()
{
}

void CClassWizardDlg::setCallbacks()
{
  connect( &virtualMethodView, 
           SIGNAL(addMethod(const char *,CParsedMethod *)),
           SLOT(slotAddMethod(const char *,CParsedMethod *)));
                  
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*--------------------------------------- CClassWizardDlg::setStore()
 * setStore()
 *   Set the classtore.
 *
 * Parameters:
 *   aStore         The store.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassWizardDlg::setStore( CClassStore *aStore )
{
  assert( aStore != NULL );

  store = aStore;

  virtualMethodView.setStore( store );
}

void CClassWizardDlg::setInitialClass( const char *aClassName )
{
  assert( aClassName != NULL && strlen( aClassName ) > 0 );

  initialClass = aClassName;
}

/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/

/** Executed when the user adds a method in some view. */
void CClassWizardDlg::slotAddMethod( const char *aClassName,
                                     CParsedMethod *aMethod )
{
  emit addMethod( aClassName, aMethod );
}
