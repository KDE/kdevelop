/***************************************************************************
                          cprjaddopts.h  -  description
                             -------------------
    begin                : Wed Feb 19 2002
    copyright            : (C) 2002 by W. Tasin
    email                : tasin@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "cprjaddopts.h"
#include "cprjaddopts.moc"
#include "cproject.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>

CPrjAddOpts::CPrjAddOpts(CPrjConfChange *confIn, QWidget *parent, const char *name ) :
  CPrjAddOptsDlg(parent,name) 
{
  configureIn = confIn;
  initGUI();
}

void CPrjAddOpts::initGUI()
{
  bool versionAvail=configureIn->isLineAvailable("^#MIN_CONFIG");
  bool nooptAvail=false;
  
  old_KDE_QT_ver=configureIn->getMacroParameter("^#MIN_CONFIG");
  
  // get kde/qt version number (if available)
  if (!versionAvail)
  {
    versionAvail=configureIn->isLineAvailable("^[\\s]*KDE_USE_QT");
    if (versionAvail)
      old_KDE_QT_ver=configureIn->getMacroParameter("^[\\s]*KDE_USE_QT");
  }

  kde_qt_version_label->setEnabled(versionAvail);
  kde_qt_version->setEnabled(versionAvail);
  if (versionAvail)
    kde_qt_version->setText(old_KDE_QT_ver);

  // check for exceptions-option
  exceptions->setEnabled(configureIn->isOptionAvailable("CXXFLAGS=", "exc"));
  exceptionsEnabled=configureIn->isOptionEnabled("CXXFLAGS=", "exc");
  exceptions->setChecked(exceptionsEnabled);
  
  // check for noopt flags-option
  nooptAvail=configureIn->isOptionAvailable("CXXFLAGS=", "noopt") ||
             configureIn->isOptionAvailable("CFLAGS=", "noopt");
  
  noopt->setEnabled(nooptAvail);
  nooptEnabled=configureIn->isOptionEnabled("CXXFLAGS=", "noopt") || 
               configureIn->isOptionEnabled("CFLAGS=", "noopt");
  noopt->setChecked(nooptEnabled);
  
  // check for misc tests-option
  misc_tests->setEnabled(configureIn->isOptionAvailable("KDE_MISC_TESTS"));
  miscTestsEnabled=configureIn->isOptionEnabled("KDE_MISC_TESTS");
  misc_tests->setChecked(miscTestsEnabled);
  
  // check for flex-option
  flex->setEnabled(configureIn->isOptionAvailable("KDE_NEED_FLEX"));
  flexEnabled=configureIn->isOptionEnabled("KDE_NEED_FLEX");
  flex->setChecked(flexEnabled);
  
  // check for yacc-option
  yacc->setEnabled(configureIn->isOptionAvailable("AC_PROG_YACC"));
  yaccEnabled=configureIn->isOptionEnabled("AC_PROG_YACC");
  yacc->setChecked(yaccEnabled);
  
}

bool CPrjAddOpts::changed() const
{
  bool resChanged=false;
  QString verString=kde_qt_version->text();
  
  // check for changement in version string
  if (!resChanged && verString.stripWhiteSpace()!=old_KDE_QT_ver)
    resChanged=true;
  
  // check now for exceptions flag changement
  if (!resChanged && exceptions->isChecked()!=exceptionsEnabled)
    resChanged=true;
  
  // check now for noopt flag changement
  if (!resChanged && noopt->isChecked()!=nooptEnabled)
    resChanged=true;
  
  // check now for misc-tests-flag changement
  if (!resChanged && misc_tests->isChecked()!=miscTestsEnabled)
    resChanged=true;
  
  // check now for flex-flag changement
  if (!resChanged && flex->isChecked()!=flexEnabled)
    resChanged=true;
  
  // check now for yacc-flag changement
  if (!resChanged && yacc->isChecked()!=yaccEnabled)
    resChanged=true;
  
  return resChanged;
}

void CPrjAddOpts::modifyConfigureIn()
{
  QString verString=(kde_qt_version->text()).stripWhiteSpace();

  // change version string inside configure.in(.in)
  if (verString.isEmpty())
  {
    configureIn->setLine("^#MIN_CONFIG", "#MIN_CONFIG");
    configureIn->setLine("^[\\s]*KDE_USE_QT", "KDE_USE_QT");
  }
  else
  {
    configureIn->setLine("^#MIN_CONFIG", "#MIN_CONFIG("+verString+")");
    configureIn->setLine("^[\\s]*KDE_USE_QT", "KDE_USE_QT("+verString+")");
  }
 
  // change noopt string inside configure.in(.in)
  if (noopt->isChecked())
  {
    configureIn->enableOption("CXXFLAGS=", "\"$NOOPT_CXXFLAGS\"", "noopt");
    configureIn->enableOption("CFLAGS=", "\"$NOOPT_CFLAGS\"", "noopt");
  }
  else
  {
    configureIn->disableOption("CXXFLAGS=", "\"$NOOPT_CXXFLAGS\"", "noopt");
    configureIn->disableOption("CFLAGS=", "\"$NOOPT_CFLAGS\"", "noopt");
  }
  
  // change exceptions string inside configure.in(.in)
  if (exceptions->isChecked())
    configureIn->enableOption("CXXFLAGS=", "\"$CXXFLAGS $USE_EXCEPTIONS\"", "exc");
  else
    configureIn->disableOption("CXXFLAGS=", "\"$CXXFLAGS $USE_EXCEPTIONS\"", "exc");
  
  // change misc tests string inside configure.in(.in)
  if (misc_tests->isChecked())
    configureIn->enableOption("KDE_MISC_TESTS");
  else
    configureIn->disableOption("KDE_MISC_TESTS");
  
  // change flex string inside configure.in(.in)
  if (flex->isChecked())
    configureIn->enableOption("KDE_NEED_FLEX");
  else
    configureIn->disableOption("KDE_NEED_FLEX");
  
  // change yacc string inside configure.in(.in)
  if (yacc->isChecked())
    configureIn->enableOption("AC_PROG_YACC");
  else
    configureIn->disableOption("AC_PROG_YACC");
  
  // write out changements
  configureIn->writeConfFile();  

}


CPrjAddOpts::~CPrjAddOpts()
{

}

