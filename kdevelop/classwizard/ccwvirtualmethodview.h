/***************************************************************************
               ccwvirtualmethodview.h  -  declaration

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

#ifndef _CWVIRTUALMETHODVIEW_H_INCLUDED
#define _CWVIRTUALMETHODVIEW_H_INCLUDED

#include <kapp.h>
#include <qtabdialog.h>
#include <qlayout.h> 
#include <qlabel.h>
#include <qcombo.h>
#include <qlistbox.h> 
#include <qwidget.h>
#include <qpushbutton.h>
#include "../classparser/ClassStore.h"

/** The virtual method view for the classwizard dialog. */
class CCWVirtualMethodView : public QWidget
{
  Q_OBJECT

public: // Constructor and destructor

  CCWVirtualMethodView( QWidget *parent = 0, const char *aName = "");
  ~CCWVirtualMethodView();

signals:

  /** Emitted when a method should be added to a class. */
  void addMethod(const char *,CParsedMethod *);

public: // Methods to set attribute values.

  /** Set the store to use to search for classes. */
  void setStore( CClassStore *aStore );

private: // Private widgets
  /** Main layout for the virtual functions view. */
  QVBoxLayout topLayout;
  
  /** Layout for choosing classes. */
  QHBoxLayout classComboLayout;

  /** Label that shows the text 'Class'. */
  QLabel classLbl;

  /** Combo in which you can choose the class to manipulate. */
  QComboBox classCombo;

  /** Layout for the two virtual function parts. */
  QHBoxLayout listViewLayout;

  /** Layout for the available part. */
  QVBoxLayout availLayout;

  /** Label for the available listbox. */
  QLabel availLbl;

  /** The available listbox. */
  QListBox availLb;

  /** Layout for the implemented part. */
  QVBoxLayout implLayout;

  /** Label for the overridden listbox. */
  QLabel implLbl;

  /** The overridden listbox. */
  QListBox implLb;

  /** Layout for the right-hand side buttons. */
  QVBoxLayout buttonLayout;

  /** Button to add virtual functions. */
  QPushButton addBtn;

  /** Top filler. */
  QLabel topFiller;

  /** Fill out vertical size. */
  QLabel filler;

protected slots:

  /** Executed when the user makes a selection in the classcombo. */
  void slotClassComboChoice(int idx);

  /** Executed when the user clicks on the add button. */
  void slotAddMethod(); 

private: // Private methods

  void setWidgetValues();
  void setCallbacks();
  void setTooltips();

private: // Private attributes

  /** Store holding all parsed objects. */
  CClassStore *store;

  /** Dict holding all available virtual functions. */
  QDict<CParsedMethod> availDict;
};

#endif
