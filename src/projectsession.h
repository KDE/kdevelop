/***************************************************************************
                          projectsession.h  -  description
                             -------------------
    begin                : 30 Nov 2002
    copyright            : (C) 2002 by Falk Brettschneider
    email                : falk@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _PROJECTSESSION_H_
#define _PROJECTSESSION_H_

#include <qdom.h>

class QWidget;
class KURL;

/**
 * This class stores and restores the last situation before the certain project
 * was closed.
 * Session stuff that is not related to a certain project doesn't belong to here;
 * it must be saved in a program session which likely is "kdeveloprc".
 **/
class ProjectSession
{
// methods
public:  
  ProjectSession();
  virtual ~ProjectSession();

  /** Opens the .kdevses file and saves the project session to it. */
  bool saveToFile(const QString& fileName);
  /** Opens the .kdevses file and loads the project session to it. */
  bool restoreFromFile(const QString& fileName);

private:
  /** recreates views of a document and their properties */
  void recreateViews(KURL& url, QDomElement docEl);
  /** Stores the geometry of a certain view to the XML tree. */
  void saveViewGeometry(QWidget* pView, QDomElement viewEl);
  /** Restores the geometry of a certain view from the XML tree. */
  void loadViewGeometry(QDomElement viewEl);
  /** Restores the part of the project session that concerns to the documents. */
  void recreateDocs(QDomElement& el);
  /** setup a valid XML file. */
  void initXMLTree();

// attributes
private:
  /** the XML document object controlling the XML tree. */
  QDomDocument domdoc;
};

#endif // _PROJECTSESSION_H_
