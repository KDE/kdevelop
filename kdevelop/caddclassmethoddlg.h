/***************************************************************************
               cclassaddmethoddlg.h  -  description

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


#ifndef CADDCLASSMETHODDLG_H
#define CADDCLASSMETHODDLG_H

#include <qdialog.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qmultilinedit.h>
#include <qlayout.h> 
#include "./classparser/ParsedMethod.h"

/** Dialog to create a new method for a class. 
 * @author Jonas Nordin
 */
class CAddClassMethodDlg : public QDialog
{
  Q_OBJECT
public: // Constructor & Destructor

  CAddClassMethodDlg( QWidget *parent=0, const char *name=0 );

public: // Public queries

  CParsedMethod *asSystemObj();

protected: // Private widgets

  ///////////////////////////////
  // Layouts
  ///////////////////////////////

  /** Main layout for the dialog. */
  QVBoxLayout topLayout;
  
  /** Layout for function definition. */
  QGridLayout functionLayout;

  /** Layout for choosing access. */
  QGridLayout accessLayout;

  /** Layout for choosing type. */
  QGridLayout typeLayout;

  /** Layout for choosing modifier. */
  QGridLayout modifierLayout;

  /** Layout for the ok/cancel buttons. */
  QHBoxLayout buttonLayout;

  ///////////////////////////////
  // Button groups
  //////////////////////////////

  QButtonGroup modifierGrp;
  QButtonGroup typeGrp;
  QButtonGroup functionGrp;
  QButtonGroup accessGrp;

  QLabel typeLbl;
  QLineEdit typeEdit;
  QLabel declLbl;
  QLineEdit declEdit;
  QLabel docLbl;
  QMultiLineEdit docEdit;

  /** Public method. */
  QRadioButton publicRb;
  /** Protected method rb. */
  QRadioButton protectedRb;
  /** Private method rb. */
  QRadioButton privateRb;

  /** This is a method rb. */
  QRadioButton methodRb;
  /** This is a signal rb. */
  QRadioButton signalRb;
  /** This is a slot rb. */
  QRadioButton slotRb;

  /** This method is virtual. */
  QCheckBox virtualCb;
  /** This method is pure-virtual. */
  QCheckBox pureCb;
  /** This method is static. */
  QCheckBox staticCb;
  /** This method is const. */
  QCheckBox constCb;

  QPushButton okBtn;
  QPushButton cancelBtn;
  QLabel btnFill;

protected slots:

  /** This slot is executed when the user clicks on the type rb.
   * It toggles the modifiers on/off. */
  void slotToggleModifier();

  /** This slot is executed when the user click the virtual button.
   * It toggles the pure button on/off.*/ 
  void slotVirtualClicked(); 
 
  void OK();

private: // Private methods

  void setWidgetValues();
  void setCallbacks();

};

#endif


