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

#include <qobject.h>
#include <qdom.h>
#include <q3dict.h>
#include <q3valuelist.h>

class QWidget;
class KURL;
class KDevPlugin;

/**
 * This class stores and restores the last situation before the certain project
 * was closed.
 * Session stuff that is not related to a certain project doesn't belong to here;
 * it must be saved in a program session which likely is "kdeveloprc".
 **/
class ProjectSession : public QObject
{
  Q_OBJECT
// methods
public:
  ProjectSession();
  virtual ~ProjectSession();

  /** Opens the .kdevses file and saves the project session in XML format to it. */
  bool saveToFile(const QString& fileName, const Q3ValueList<KDevPlugin*> plugins );

  /** Opens the .kdevses file and loads the project session from it. */
  bool restoreFromFile(const QString& fileName, const Q3ValueList<KDevPlugin*> plugins );

signals:
  void sig_restoreMainWindowProperties(const QDomElement* el);
  void sig_saveMainWindowProperties(QDomElement* el);

  void sig_restoreAdditionalViewProperties(const QString& viewName, const QDomElement* el);
  void sig_saveAdditionalViewProperties(const QString& viewName, QDomElement* el);

private slots:
  /**
   * This slot loads one document from _docDataList and sets a timer to load the next
   */
  void loadDocument();

private:
  /** Restores the part of the project session that concerns to the documents (files). */
  void recreateDocs(QDomElement& el);
  /** recreates views and their properties of a certain document. */
  void recreateViews(KURL& url, QDomElement docEl, bool activate);
  /** setup a valid XML file. */
  void initXMLTree();

// attributes
private:
  /** the XML document object controlling the XML tree. */
  QDomDocument domdoc;

  struct DocumentData
  {
	  DocumentData() : line(0) {}
	  KURL url;
	  int line;
	  QString type;
      bool activate;
  };

  Q3ValueList<DocumentData> _docDataList;

};

#endif // _PROJECTSESSION_H_
