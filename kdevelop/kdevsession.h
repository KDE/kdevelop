/***************************************************************************
                          kdevsession.h  -  description
                             -------------------
    begin                : Mon Mar 26 2001
    copyright            : (C) 2001 by Falk Brettschneider
    email                : falk.brettschneider@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _KDEVSESSION_H_
#define _KDEVSESSION_H_

#include <qlist.h>
#include <qstringlist.h>
#include <qdom.h>

//to avoid includes
class QextMdiChildView;
class DocViewMan;

//------------------------------------------------------------------------------
/**
   Saving, loading of the KDevelop session
*/
class KDevSession
{
// methods
public:

  KDevSession(DocViewMan* pDocViewMan, const QString& fileName = "");
  ~KDevSession() {};

  /** file I/O functions - opens the file and execute save/load
   */
  bool saveToFile(const QString& fileName);
  bool restoreFromFile(const QString& fileName);

  /** recreates views of a document and their properties */
  void recreateViews( QObject* pDoc, QDomElement docEl);
  /** */
  void saveViewGeometry( QWidget* pView, QDomElement viewEl);
  /** */
  void loadViewGeometry( QWidget* pView, QDomElement viewEl);
  /** No descriptions */
  void recreateDocs(QDomElement& el);

  /** add a new compile configuration (configname)  to the config list (creates a new node in the tree) */
  void addCompileConfig(const QString& configname);
  /** get the names of the different compile configurations in a string list to
  	insert them into the toolbar and into the project options dialog */
  QStringList getCompileConfigs();

  /** sets the VPATH subdir for the configuration configname */
  void setVPATHSubdir(const QString& configname, const QString& subdir);
  /** returns the VPATH subdir configured for this configuration to start compilation in. */
  QString getVPATHSubdir(const QString& configname);
  /** sets the architecture (processor type) for the configuration configname */
  void setArchitecture(const QString& configname, const QString& arch);
  /** returns the architecture set for the given configuration (which is the processortype) */
  QString getArchitecture(const QString& configname);
  /** sets the platform (Operating System) for the given configuration configname */
  void setPlatform(const QString& configname, const QString& platform);
  /** returns the platform of the given configuration (Operating System) */
  QString getPlatform(const QString& configname);
  /** sets the CPPFLAGS string for the configuration configname */
  void setCPPFLAGS(const QString& configname, const QString& cppflags);
  /** return the CPPFLAGS set for the given configuration. */
  QString getCPPFLAGS(const QString& configname);
  /** sets the CFLAGS string for the given configuration configname */
  void setCFLAGS(const QString& configname, const QString& cflags);
  /** return the CFLAGS for the given configuration to export before compiling. */
  QString getCFLAGS(const QString& configname);
  /** sets the CXXFLAGS string for the configuration configname */
  void setCXXFLAGS(const QString& configname, const QString& cxxflags);
  /** return the CXXFLAGS string for the configuration configname */
  QString getCXXFLAGS(const QString& configname);
  /** In analogy to the kdevprj project file the additional cxxflags not occupied by the configuration dialog
are put in here. */
  void setAdditCXXFLAGS(const QString& configname, const QString& additflags);
  /** in analogy to the kdevprj project file all flags who are not occupied by the GUI are stored in the additional CXXFLAGS and put into the lineedit
of the project options dialog. */
  QString getAdditCXXFLAGS(const QString& configname);
  /** sets the configure arguments for the configuration configname */
  void setConfigureArgs(const QString& configname, const QString& confargs);
  /** return the configure arguments for this configuration. */
  QString getConfigureArgs(const QString& configname);
  /** initializes the XML tree on startup of kdevelop and when a project has been closed to ensure
that the XML tree exists already including doctype when a project gets opened that doesn't have 
a kdevses file or a new project gets generated (which doesn't have a kdevses file either as the project
has never been closed before opening it). */
  void initXMLTree();
  /** sets the linker flags for the configuration configname. */
  void setLDFLAGS(const QString& configname, const QString& ldflags);
  /** retrieve the linker flags (LDFLAGS) for the configuration configname. */
  QString getLDFLAGS(const QString& configname);
  /** sets the last compile config  before the project was closed. */
  void setLastCompile(const QString& comp);
  /** returns the last used compile config for restore after loading */
  QString getLastCompile();

//attributes
  /** The modules doc and view manager */
  DocViewMan*              m_pDocViewMan;

private: //to avoid use (not implemented)
  KDevSession(const KDevSession&);
  KDevSession& operator=(const KDevSession&);
	QDomDocument domdoc;
	QDomElement helper;
//---------helper functions----------//
  /** find the QDomElement of the configuration configname and return a reference to it. Used by all get/set functions. */
  const QDomElement& getConfigElement(const QString& configname);
  /** creates a new QDomElement or fills an existing one with a textnode as its only child. the QDomElement name is equal to name, the
textnode value is value.  */
  void createEntry(QDomElement parent, const QString& name, const QString& value);
};

//------------------------------------------------------------------------------
#endif
