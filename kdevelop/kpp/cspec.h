/***************************************************************************
                          cspec.h  -  description
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

#ifndef CSPEC_H
#define CSPEC_H
#include <qstring.h>

/**This is the datastructure that will keep all of the specfile information.
  *@author ian geiser
  */

class cSpec {
public: 
	cSpec();
	~cSpec();
  /** Sets the author name of the current project spec file. */
  void setAuthor(QString newAuthor);
  /** Sets the name of the current project spec file. */
  void setName( QString newName );
  /** Set the version of the current project spec file. */
  void setVersion(QString newVersion);
  /** Sets the new configure options for the current project spec file. */
  void setConfigOpts(QString newOpts);
  /** Sets the short info for the current project spec file. */
  void setShortInfo(QString newInfo );
  /** Gets the author name of the current project spec file. */
  QString getAuthor();
  /** Gets the name of the current project spec file. */
  QString getName();
  /** Get the version of the current project spec file. */
  QString getVersion();
  /** Gets the new configure options for the current project spec file. */
  QString getConfigOpts();
  /** Gets the short info for the current project spec file. */
  QString getShortInfo();
  /** Generates a spec file per the current data structure.
It will return a string which is the spec file. This function takes
in a template.  This template will be formatted as shown below:
  <pre>
  %%author%%  = qsAuthor
  %%catagory%% = qsCatagory
  %%config%%  = qsConfigOpts
  %%desc%%  = qsDesc
  %%name%%  = qsProjectName
  %%release%% = qsRelease
  %%source%% = qsSource
  %%version%% = qsVersion
  %%info%% = qsInfo
  </pre>
  */
  QString generateSpec(QString specIn);
  /** Returns the current release */
  QString getRelease();
  /** Sets the release number. */
  void setRelease( QString newRelease);
  /** Returns the long description. */
  QString getDesc();
  /** Sets the ong description */
  void setDesc( QString newDesc);
  /** Returns the name of the source code package. */
  QString getSource();
  /** This will set the URL of the spec file from the main dialog */
  void setURL(QString newURL);
  /** Set the vendor of the current project */
  void setVendor( QString newVendor);
  /** Set the licencse of the new RPM */
  void setLic( QString newLicense);
  /** Sets the catagory of the current spec file. */
  void setCatagory(QString newCatagory);
  /** Set the path to the icon. */
  void setIcon();
  /** Will return the path to the spec file icon. */
  QString getIcon();
  /** Set the build root. */
  void setBuildRoot(QString newBuildRoot);
  /** Get the path to the build root. */
  QString getBuildRoot();
private: // Private attributes
  /** The author of the current project in the format of Author Name <email@address> */
  QString qsAuthor;
  /** The configure options that need to be passed into the ./configure script during build time. */
  QString qsConfigOpts;
  /** The name of the source code tar gziped archive. */
  QString qsSource;
  /** The version number from the current project */
  QString qsVersion;
  /** The address of the current projects author */
  QString qsInfo;
  /** The name of the current project */
  QString qsProjectName;
  /** The catagory of which the RPM should be filed under. */
  QString qsCatagory;
  /** The release number of the project.
This will need to be set higher than a previous release version if you are building the same version of an RPM. */
  QString qsRelease;
  /** The long description of the package */
  QString qsDesc;
  /** The url for the current RPM */
  QString qsUrl;
  /** The license type */
  QString qsLic;
  /** The vendor of the current RPM */
  QString qsVendor;
  /** The icon for the spec file  */
  QString qsIcon;
  /** The user defined build root  */
  QString qsBuildRoot;
};

#endif






















