/***************************************************************************
                          KDevCompiler.cpp  -  description
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

#include "KDevCompiler.h"
#include "kdebug.h"

/* Class KDevCompiler
 * Provide functions to register a compiler
 * and its dependant attributes.
 */

// constructors
KDevCompiler::KDevCompiler(){
	compilerName = new QString("");
	compilerFlags = new QString("");
}
KDevCompiler::KDevCompiler(const QString &name){
	compilerName = new QString(name);
	compilerFlags = new QString("");
}
KDevCompiler::KDevCompiler(const QString &name, const QString &icon){
	compilerName = new QString(name);
	compilerIcon = new QString(icon);
	compilerFlags = new QString("");
}

KDevCompiler::KDevCompiler(const QString &name, const QString &icon, QWidget &qw){
	compilerName = new QString(name);
	compilerIcon = new QString(icon);
	compilerFlags = new QString("");
	optionsPageWidget = &qw;
}

// destructor
// what happens if any of the pointers are null
KDevCompiler::~KDevCompiler(){
	if(compilerName)      delete compilerName;
	if(compilerFlags)     delete compilerFlags;
	if(compilerIcon)      delete compilerIcon;
	if(optionsPageWidget) delete optionsPageWidget;
}

// returns the name of the compiler
QString* KDevCompiler::name(){
	return compilerName;
}

// returns the compiler flags
QString* KDevCompiler::flags(){
	return compilerFlags;
}

// returns the icon of the compiler
QString* KDevCompiler::icon(){
	return compilerIcon;
}

// returns the option widget of the compiler
QWidget* KDevCompiler::optionsWidget(){
	return optionsPageWidget;
}

// set the compiler Name
// MEMORY LEAK what happens if compilerName/Flags/Icon are already set?
void KDevCompiler::setName(const QString &name){
  if(compilerName) delete compilerName;
	compilerName = new QString(name);
}

// set the compiler flags
void KDevCompiler::setFlags(const QString &flags){
  if(compilerFlags) delete compilerFlags;
	compilerFlags = new QString(flags);
}

// set the compiler icon
void KDevCompiler::setIcon(const QString &icon){
  if(compilerIcon) delete compilerIcon;
	compilerIcon = new QString(icon);
}

// set the compiler icon
void KDevCompiler::setOptionsWidget(QWidget &qw){
	optionsPageWidget = &qw;
}

QWidget* KDevCompiler::initOptionsWidget(QWidget* parent, QWidget* pdlg){
	return 0;
}

#include "KDevCompiler.moc"
