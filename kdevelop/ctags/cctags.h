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

class KShellProcess;
class CProject;

/**
 * ctags support for kdevelop
 * @author rokrau@yahoo.com (the kdevelop-team)
 **/

/**
 * \class CTag
 * \brief The basic tag data structure
 *
 * A CTag consists of three QStrings, the filename, an "ex" command which
 * is a regular expression used by "legacy" editors to find the tag in the
 * file and an "extension". The extension is only found when "Exuberant Ctags"
 * was used to generate the file. "Exuberant Ctags" is therefore strogly
 * recommended. It can be found at http://ctags.sourceforge.net
 *
 * \author rokrau@yahoo.com
 **/
class CTag {
public:
  /** default constructor */
  CTag() :
    m_file(QString::null), m_excmd(QString::null),
    m_ext(QString::null), m_type(' ')
    {}
  /** constructor, requires filename, ex command and extension as arguments */
  CTag(QString _file, QString _excmd, QString _ext) :
    m_file(_file), m_excmd(_excmd), m_ext(_ext), m_type(' ')
    {
      parse_ext();
    }
  /** destructor */
 ~CTag() {}
  /** return the tag type */
  char type() const {return m_type;}
  /** return the file name */
  QString file() const {return m_file;}
  /** return stripped search pattern for kwrite based editor */
  QString pattern() ;
protected:
  void parse_ext(); // parse exuberant extension
private:
  QString m_file;   // file name
  QString m_excmd;  // vi (ex) search command
  QString m_ext;    // exuberant ctags extension
  char    m_type;   // tag type from ctags extension
};
/**
 * \class CTagList
 * \brief A QValueList of CTags that has a tag name
 *
 * Tags usually have multiple occurences. Functions can be overloaded, classes
 * have declarations as well as definitions, there is always tag ambiguity.
 * All tags with the same name are therefore collected in a CTagList. The
 * CTagList additioally has the tag name. A smart algorithm can then be used
 * to help the user find the approriate location, depending on the context
 * of the search tag and the choosen location method.
 *
 * \author rokrau@yahoo.com
 **/
class CTagList : public QValueList<CTag> {
public:
  /** constructor, requires the tag name as argument */
  CTagList(QString _tag) : QValueList<CTag>(), m_tag(_tag) {}
  /** destructor */
  ~CTagList() {}
private:
  QString m_tag;    // tag name
};
/**
 * \def A CTagListDict is a QDict for CTagLists
 *
 * \author rokrau@yahoo.com
 **/
typedef QDict<CTagList> CTagListDict;
/**
 * \class CTagsDataBase, frontend for ctags handling
 *
 * \brief This class provides access to the ctags database
 *
 * CTagsDataBase is a frontend for handling of ctags data. The class provides
 * methods to create, load and reload a tags file. It handles searching,
 * as well as access to the ctags "extension" functionality.
 * There should be exactly one CTagsDataBase object per CProject but later if we
 * have multiple projects that can change to one CTagsDataBase object per workspace.
 *
 * \author rokrau@yahoo.com
 **/
class CTagsDataBase {
public:
  /** default constructor */
  CTagsDataBase();
  /** destructor */
  ~CTagsDataBase();
  /** create a tags file for a CProject */
  void create_tags(KShellProcess& process, CProject& project);
  /** load a tags file and create the search database */
  void load();
  /** unload, i.e. destroy the current search database */
  void unload();
  /** reload, unload current and load regenerated search database */
  void reload();
  /** the number of CTag entries found for a tag */
  int nCTags(const QString& tag) const ;
  /** return the list of CTag entries for a tag */
  const CTagList* ctaglist(const QString& tag) const ;
  /** return whether CTagsDataBase is initialized */
  bool is_init() const {return m_init;}
private:
  /** true if search database is initialized */
  bool m_init;
  /** searchable database */
  CTagListDict m_taglistdict;
  /** tagfile name from which the database was created */
  QString m_file;
};
/**
 * \class CtagsCommand
 *
 * \brief ctags command and command line options
 *
 * \author rokrau@yahoo.com
 **/
class CtagsCommand {
  friend class CTagsDataBase;
public:
  CtagsCommand() {
    m_command = "ctags";
    m_totals = "--totals=yes";
    m_excmd_pattern = "--excmd=pattern";
    m_file_scope = "--file-scope=yes";
    m_file_tags = "--file-tags=yes";
    m_ctypes = "--c-types=+px";
    m_fortran_types = "--fortran-types=-l+L";
    m_exclude = "*.inc *.bck glimpse*";
    m_fields = "+i";
  }
  ~CtagsCommand() {}
private:
  QString m_command;
  QString m_totals;
  QString m_excmd_pattern;
  QString m_file_scope;
  QString m_file_tags;
  QString m_ctypes;
  QString m_fortran_types;
  QString m_exclude;
  QString m_fields;
};
#endif
