/***************************************************************************
                          KDevLinker.cpp  -  description
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

#include "KDevLinker.h"

/* Class KDevLinker
 * Provide functions to register a Linker
 * and its dependant attributes.
 */

// constructors
KDevLinker::KDevLinker(){
	LinkerName = new QString("");
	LinkerFlags = new QString("");
}
KDevLinker::KDevLinker(const QString &name){
	LinkerName = new QString(name);
	LinkerFlags = new QString("");
}
KDevLinker::KDevLinker(const QString &name, const QString &icon){
	LinkerName = new QString(name);
	LinkerIcon = new QString(icon);
	LinkerFlags = new QString("");
}

KDevLinker::KDevLinker(const QString &name, const QString &icon, QWidget &qw){
	LinkerName = new QString(name);
	LinkerIcon = new QString(icon);
	LinkerFlags = new QString("");
	optionsPageWidget = &qw;
}

// destructor
KDevLinker::~KDevLinker(){
	if (LinkerName) delete LinkerName;
	if (LinkerName) delete LinkerFlags;
	if (LinkerName) delete LinkerIcon;
	if (LinkerName) delete optionsPageWidget;
}

// returns the name of the Linker
QString* KDevLinker::name(){
	return LinkerName;
}

// returns the Linker flags
QString* KDevLinker::flags(){
	return LinkerFlags;
}

// returns the icon of the Linker
QString* KDevLinker::icon(){
	return LinkerIcon;
}

// returns the option widget of the Linker
QWidget* KDevLinker::optionsWidget(){
	return optionsPageWidget;
}

// set the Linker Name
void KDevLinker::setName(const QString &name){
	if (LinkerName)
		*LinkerName = *name;
	else
		LinkerName = new QString(name);
}

// set the Linker flags
void KDevLinker::setFlags(const QString &flags){
	if (LinkerFlags)
		LinkerFlags = new QString(flags);
	else
	  *LinkerFlags = *flags;
}

// set the Linker icon
void KDevLinker::setIcon(const QString &icon){
	if (LinkerIcon)
		LinkerIcon = new QString(icon);
	else
	  *LinkerIcon = *icon;
}

// set the Linker widget
void KDevLinker::setOptionsWidget(QWidget &qw){
	optionsPageWidget = &qw;
}

QWidget* KDevLinker::initOptionsWidget(QWidget* parent, QWidget* pdlg){
	return 0;
}

#include "KDevLinker.moc"
