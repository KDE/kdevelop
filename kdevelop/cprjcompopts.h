/***************************************************************************
                          cprjcompopts.h  -  description
                             -------------------
    begin                : Wed Nov 21 2001
    copyright            : (C) 2001 by Ralf Nolden
    email                : nolden@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CPRJCOMPOPTS_H
#define CPRJCOMPOPTS_H

#include <qwidget.h>
#include <cprjcompoptsdlg.h>

class QCheckListItem;
class QStringList;
class CProject;
class KDevSession;

/**Compiler options dialog to configure different environments 
using VPATH and setting their respective architecture,
debugging, optimization, flags and warnings.
  *@author Ralf Nolden <nolden@kde.org>
  */

class CPrjCompOpts : public CPrjCompOptsDlg  {
   Q_OBJECT
public: 
	CPrjCompOpts(CProject* prj,KDevSession* session,const QString& curr, QWidget *parent=0, const char *name=0);
	~CPrjCompOpts();
  /** Load the settings from either the project file if only the default is used (session configuration for the 
other compiler configurations don't exist) and the according configurations in the session file plus their
respective flags and settings. */
  void loadSettings();


private:
 	KDevSession* sess;
  CProject* prj_info;
	// the stringlist that the host shall compile for with a crosscompiler
  QStringList compilearchs;
	// the stringlist that the host shall compile for with a crosscompiler
  QStringList compileplatforms;
  // the stringlist for the config combobox	
  QStringList compconfs;

  QString cfgargs;
		
	
  QCheckListItem* w_all;
  QCheckListItem* w_;
  QCheckListItem* w_traditional;
  QCheckListItem* w_undef;
  QCheckListItem* w_shadow;
  //  QSpinBox*  w_id_clash_len;
  //  QSpinBox*  w_larger_than_len;
  QCheckListItem* w_pointer_arith;
  QCheckListItem* w_bad_function_cast;
  QCheckListItem* w_cast_qual;
  QCheckListItem* w_cast_align;
  QCheckListItem* w_write_strings;
  QCheckListItem* w_conversion;
  QCheckListItem* w_sign_compare;
  QCheckListItem* w_aggregate_return;
  QCheckListItem* w_strict_prototypes;
  QCheckListItem* w_missing_prototypes;
  QCheckListItem* w_missing_declarations;
  QCheckListItem* w_redundant_decls;
  QCheckListItem* w_nested_externs;
  QCheckListItem* w_inline;
  QCheckListItem* w_old_style_cast;
  QCheckListItem* w_overloaded_virtual;
  QCheckListItem* w_synth;
  QCheckListItem* w_error;
private: // Private methods
  /** No descriptions */
  void initGUI();
  /** calculates the current CXXFLAGS for the configuration */
  QString calculateCXXFLAGS();
  /** Calculates the current configure arguments and returns the according argument string
 */
  QString calculateConfigureArgs();
	/** parses the cfgargs string for the option given, returns the argument of the option
		and deletes both from the string */
  QString findConfigureOption(QString option);
  /** reconnects all GUI items at the end of slotConfigChanged() to reestablish user 
manipulation of configuration settings. */
  void connectGUI();
  /** disconnects all connections to slotSettingsChanged(), used in slotConfigChanged() to prevent
overwriting settings by slotSettingsChanged() */
  void disconnectGUI();
protected slots: // Protected slots
  /** Select/create the directory to run configure/make in
for the given configuration */
  void slotCompPath();
  /** invoked by pressing the Add button to add a new 
configuration for compiling to the combobox. */
  void slotConfigAdd();
  /** No descriptions */
  void slotPrefixPath();
  /** No descriptions */
  void slotQtPath();
  /** No descriptions */
  void slotXincPath();
  /** No descriptions */
  void slotXlibPath();
public slots: // Public slots
  /** No descriptions */
  void slotConfigChanged(const QString& conf);
  /** No descriptions */
  void slotSettingsChanged();
};

#endif
