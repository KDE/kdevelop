/***************************************************************************
                          cspec.cpp  -  description
                             -------------------
    begin                : Sun Oct 10 1999
    copyright            : (C) 1999 by ian geiser
    email                : geiseri@msoe.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cspec.h"
#include <qregexp.h>
#include <qstring.h>
#include <iostream.h>
cSpec::cSpec(){
  qsCatagory = "KDE";
  qsLic = "GPL";
  qsInfo = "KDE Application";
  qsAuthor = "none";
  qsDesc = "none";
  qsProjectName = "none";
  qsRelease = "1";
  qsSource = "none";
  qsUrl = "none";
  qsVendor = "none";
  qsVersion = "none";
  qsBuildRoot = "/tmp/buildroot/";
  qsIcon = "";
}
cSpec::~cSpec(){
}

/** Sets the name of the current project spec file. */
void cSpec::setName( QString newName ){
  qsProjectName = newName.lower();
}

/** Sets the author name of the current project spec file.*/
void cSpec::setAuthor(QString newAuthor){
  qsAuthor = newAuthor;
}

/** Set the version of the current project spec file. */
void cSpec::setVersion(QString newVersion){
  qsVersion = newVersion;
}

/** Sets the short info for the current project spec file. */
void cSpec::setShortInfo(QString newInfo ){
  qsInfo = newInfo;
}

/** Sets the new configure options for the current project spec file. */
void cSpec::setConfigOpts(QString newOpts){
  qsConfigOpts = newOpts;
}

/** Gets the name of the current project spec file. */
QString cSpec::getName(){
  return qsProjectName;
}

/** Gets the author name of the current project spec file.*/
QString cSpec::getAuthor(){
  return qsAuthor;
}

/** Get the version of the current project spec file. */
QString cSpec::getVersion(){
  return qsVersion;
}

/** Gets the short info for the current project spec file. */
QString cSpec::getShortInfo(){
  return qsInfo;
}

/** Gets the new configure options for the current project spec file. */
QString cSpec::getConfigOpts(){
  return qsConfigOpts;
}

/** Generates a spec file per the current data structure.
It will return a string which is the spec file. */
QString cSpec::generateSpec(QString specIn){
// Fix source code file
  qsSource = qsProjectName;
  qsSource += "-";
  qsSource += qsVersion;
  qsSource += ".tar.gz";

  QString specOut;
//  Look for each Key tag in the template and clobber them
//  with the current data strucutre values.
  if (qsAuthor)
     specOut = specIn.replace( QRegExp("%%author%%"), qsAuthor );
  if (qsCatagory)
     specOut = specIn.replace( QRegExp("%%catagory%%"), qsCatagory );
  if (qsConfigOpts)
     specOut = specIn.replace( QRegExp("%%config%%"), qsConfigOpts );
  if (qsInfo)
     specOut = specIn.replace( QRegExp("%%info%%"), qsInfo );
  if (qsProjectName)
     specOut = specIn.replace( QRegExp("%%name%%"), qsProjectName );
  if (qsRelease)
     specOut = specIn.replace( QRegExp("%%release%%"), qsRelease );
  if (qsSource)
     specOut = specIn.replace( QRegExp("%%source%%"), qsSource );
  if (qsVersion)
     specOut = specIn.replace( QRegExp("%%version%%"), qsVersion );
  if (qsVendor)
     specOut = specIn.replace( QRegExp("%%vendor%%"), qsVendor);
  if (qsDesc)
     specOut = specIn.replace( QRegExp("%%desc%%"), qsDesc );
  if (qsLic)
     specOut = specIn.replace( QRegExp("%%license%%"), qsLic);
  if (qsCatagory)
     specOut = specIn.replace( QRegExp("%%catagory%%"), qsCatagory);
  if (qsUrl)
     specOut = specIn.replace( QRegExp("%%url%%"), qsUrl);
  if (qsBuildRoot)
     specOut = specIn.replace( QRegExp("%%buildroot%%"), qsBuildRoot);
  if (qsIcon)
     specOut = specIn.replace( QRegExp("%%icon%%"), qsIcon);
  return specOut;
}

/** Sets the release number. */
void cSpec::setRelease( QString newRelease){
  qsRelease = newRelease;
}

/** Returns the current release */
QString cSpec::getRelease(){
  return qsRelease;
}


/** Returns the long description. */
QString cSpec::getDesc(){
  return qsDesc;
}


/** Sets the ong description */
void cSpec::setDesc(QString newDesc){
  qsDesc = newDesc;
}

/** Returns the name of the source code package. */
QString cSpec::getSource(){
// Fix source code file
  qsSource = qsProjectName;
  qsSource += "-";
  qsSource += qsVersion;
  qsSource += ".tar.gz";
  return qsSource;
}
/** This will set the URL of the spec file from the main dialog */
void cSpec::setURL(QString newURL){
     qsUrl = newURL;
}
/** Set the licencse of the new RPM */
void cSpec::setLic( QString newLicense){
     qsLic = newLicense;
}
/** Set the vendor of the current project */
void cSpec::setVendor( QString newVendor){
     qsVendor = newVendor;
}
/** Sets the catagory of the current spec file. */
void cSpec::setCatagory(QString newCatagory){
     qsCatagory = newCatagory;
}
/** Will return the path to the spec file icon. */
QString cSpec::getIcon(){
     return qsIcon;
}
/** Set the path to the icon. */
void cSpec::setIcon(){
     qsIcon = "Icon:     ";
//     qsIcon += "-";
//     qsIcon += qsVersion;
//     qsIcon += "/";
//     qsIcon += qsProjectName;
//     qsIcon += "/";
     qsIcon += qsProjectName;
     qsIcon += ".xpm";
}
/** Get the path to the build root. */
QString cSpec::getBuildRoot(){
     return qsBuildRoot;
}
/** Set the build root. */
void cSpec::setBuildRoot(QString newBuildRoot){
     qsBuildRoot = newBuildRoot;
}
