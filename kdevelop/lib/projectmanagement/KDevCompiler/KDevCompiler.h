/***************************************************************************
                          KDevCompiler.h  -  description
                             -------------------
    begin                : Tue Jan 23 2001
    copyright            : (C) 2001 by Omid Givi
    email                : omid@givi.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVCOMPILER_H
#define KDEVCOMPILER_H

#include <qstring.h>
#include <qwidget.h>


/**
  *@author Omid Givi
  */

class KDevCompiler;

// A compiler object
class KDevCompiler {
public:
	KDevCompiler();
	~KDevCompiler();
	KDevCompiler(const QString &name);
	KDevCompiler(const QString &name, const QString &icon);
	KDevCompiler(const QString &name, const QString &icon, QWidget &qw);

  enum CompilerID{
    Gcc = 0,
    Cpp,
    Fortran,
    Modula3
  };
	
	KDevCompiler(CompilerID cid);
	
	QString* name();
	QString* flags();
	QWidget* optionsWidget();
	QString* icon();
	int id();
	void setName(const QString &name);
	void setFlags(const QString &flags);
	void setIcon(const QString &icon);
	void setID(CompilerID cid);
	void setOptionsWidget(QWidget &qw);
	QWidget* initOptionsWidget(QWidget* parent = 0, QWidget* pdlg = 0);
		
private:
// gcc compiler
	void initGcc();
	QWidget* initGccOptionsWidget(QWidget* parent = 0, const char* name = 0, QWidget* pdlg = 0);
// cpp compiler
	void initCpp();
	QWidget* initCppOptionsWidget(QWidget* parent = 0, const char* name = 0, QWidget* pdlg = 0);
	
	CompilerID compilerID;
	QString* compilerName;
	QString* compilerFlags;
	QString* compilerIcon;
	QWidget* optionsPageWidget;
};

#endif
