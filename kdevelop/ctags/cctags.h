/***************************************************************************
                          cctags.h  -  description
                             -------------------
    begin                : Wed Feb 21 2001
    copyright            : (C) 2001 rokrau (Roland Krause)
    email                : rokrau@yahoo.com
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
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qdict.h>

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
  enum pointTo {tFile,tDefinition,tDeclaration};
public:
  /** default constructor */
  CTag() :
    m_file(QString::null), m_excmd(QString::null),
    m_ext(QString::null), m_type(' '), m_pointTo(tDeclaration)
    {}
  /** constructor, requires filename, ex command and extension as arguments */
  CTag(QString _file, QString _excmd, QString _ext) :
    m_file(_file), m_excmd(_excmd), m_ext(_ext), m_type(' '),
    m_pointTo(tDeclaration)
    {
      parse_ext();
    }
  /** destructor */
 ~CTag() {}
  /** return the tag type */
  char type() const {return m_type;}
  /** return a string characterizing the tag type */
  QString typeName() const ;
  /** return true if the tag points to a file */
  bool isFile() const ;
  /** return true if the tag points to definition */
  bool isDefinition() const ;
  /** return true if the tag points to declaration */
  bool isDeclaration() const ;
  /** return the file name */
  QString file() const {return m_file;}
  /** return stripped search pattern for kwrite based editor */
  QString pattern() const ;
  /** return line number extracted from excmd */
  int line() const ;
  /** return a copy of ctags extension string */
  QString ext() const {return m_ext;}
protected:
  void parse_ext(); // parse exuberant extension
private:
  QString m_file;     // file name
  QString m_excmd;    // vi (ex) search command
  QString m_ext;      // exuberant ctags extension
  char    m_type;     // tag type from ctags extension
  pointTo m_pointTo;  // strictly internal, what the tag points to
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
 * We distinguish three groups of tags, tags that point to a file, tags that
 * point to a definition and tags that point to a declaration.
 *
 * \author rokrau@yahoo.com
 **/
class CTagList : public QValueList<CTag> {
public:
  /** constructor, requires the tag name as argument */
  CTagList(QString _tag=QString::null) : QValueList<CTag>(),
    m_tag(_tag), m_nfiles(0), m_ndefinitions(0), m_ndeclarations(0) {}
  /** destructor */
  ~CTagList() {}
  /** append a CTag to the list and count it */
  CTagList::Iterator append(const CTag& ctag);
  /** return the tag */
  QString tag() const {return m_tag;}
  /** return number of file tags */
  int nFileTags() const {return m_nfiles;}
//  /** return all file tags */
//  CTagList getFileTags() const ;
  /** return number of definition tags */
  int nDefinitionTags() const {return m_ndefinitions;}
//  /** return all definition tags */
//  CTagList getDefinitionTags() const ;
  /** return number of Declaration tags */
  int nDeclarationTags() const {return m_ndeclarations;}
//  /** return all declaration tags */
//  CTagList getDeclarationTags() const ;
private:
  QString m_tag;       // tag name
  int m_nfiles;        // number of file tags
  int m_ndefinitions;  // number of definition tags
  int m_ndeclarations; // number of declaration tags
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
  /** load a tags file and create the search database */
  void load(const QString& file);
  /** unload, remove search database but keep file list */
  void unload();
  /** reload, unload and reload current search database */
  void reload();
  /** clear, unload database and remove file list */
  void clear();
  /** the number of CTag entries found for a tag */
  int nCTags(const QString& tag) const ;
  /** return the list of CTag entries for a tag */
  const CTagList* ctaglist(const QString& tag) const ;
  /** return whether CTagsDataBase is initialized */
  bool is_initialized() const {return m_init;}
private:
  /** true if search database is initialized */
  bool m_init;
  /** searchable database */
  CTagListDict m_taglistdict;
  /** tagfile name from which the database was created */
  QStringList m_filelist;
};
#endif
