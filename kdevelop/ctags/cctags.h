/***************************************************************************
                          cctags.h  -  description
                             -------------------
    begin                : Wed Feb 21 2001
    copyright            : (C) 2001 by kdevelop-team
    email                : kdevelop-team@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CCTAGS_H
#define CCTAGS_H

#include <qstring.h>
#include <qvaluelist.h>
#include <qdict.h>

/**ctags support for kdevelop
  *@author rokrau (kdevelop-team)
  */
class CTag {
public:
  CTag() :
    m_file(QString::null), m_excmd(QString::null), m_ext(QString::null)
    {}
  CTag(QString _file, QString _excmd, QString _ext) :
    m_file(_file), m_excmd(_excmd), m_ext(_ext)
    {}
 ~CTag() {}
//private: not yet
  QString m_file;   // file name
  QString m_excmd;  // vi (ex) search command
  QString m_ext;    // exuberant ctags extension
};
class CTagList : public QValueList<CTag> {
public:
  CTagList(QString _tag) : QValueList<CTag>(), m_tag(_tag) {}
  ~CTagList() {}
//private: not yet
  QString m_tag;    // tag name
};
typedef QDict<CTagList> CTagListDict;

class CCtags {
public:
	CCtags();
	~CCtags();
	/** Open, read and parse the tags file. */
	void load_tags();
	void unload_tags();
	/** Create a tags file for the current project. The file is stored in the project parent
	directory. Later this should be configurable.  */
	void make_tags();
private: // Private attributes
	QString m_file;
};

#endif
