/***************************************************************************
               cclasstooldlg.h  -  declaration

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
#ifndef _CCLASSWIZARDDLG_H_INCLUDED
#define _CCLASSWIZARDDLG_H_INCLUDED

#include "ccwvirtualmethodview.h"
#include "../classparser/ClassStore.h"

/** This dialog handles implementation of virtual functions and connecting signals to slots. */
class CClassWizardDlg : public QTabDialog
{
  Q_OBJECT

public: // Constructor and destructor

  CClassWizardDlg( QWidget *parent = 0, const char *aName = "");
  ~CClassWizardDlg();

public: // Methods to set attribute values.

  /** Set the store to use to search for classes. */
  void setStore( CClassStore *aStore );

  /** Set the initial classname to startup the. */
  void setInitialClass( const char *aClassName );

private: // Private widgets for virtual function view.
  
  /** Main widget for the virtual functions view. */
  CCWVirtualMethodView virtualMethodView;

protected slots:

  /** Executed when the user adds a method in some view. */
  void slotAddMethod( const char *aClassName,CParsedMethod *aMethod );

signals:

  /** Emitted when a method should be added to a class. */
  void addMethod(const char *,CParsedMethod *);

private: // Private methods

  void setWidgetValues();
  void setCallbacks();
  void setTooltips();

private: // Private attributes

  /** Store holding all parsed objects. */
  CClassStore *store;
};

#endif
