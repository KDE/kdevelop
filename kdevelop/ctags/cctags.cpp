/***************************************************************************
                          cctags.cpp  -  description
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

#include <kdebug.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qfileinfo.h>
#include "cctags.h"

/**
 *  parse extension that exuberant ctags appends to excmd
 *  we currently only look a the type (or kind) of the tag
 **/
void CTag::parse_ext()
{
  // split extension which is tab seperated, according to ctags manual
  QChar sep('\t');
  QStringList extlist = QStringList::split(sep,m_ext);
  int n_ext = extlist.count();
  if (n_ext==0) return ;
  for (QStringList::Iterator it=extlist.begin() ; it!=extlist.end() ; ++it)
  {
    QString key, value;
    char c=':'; // the seperator for key:value pairs
    int np=(*it).find(c);
    // if no seperator, this must be a kind key
    if (np==-1) {
      key = "kind";
      value = (*it);
    }
    else {
      key = (*it).left(np);
      value = (*it).right(np+1);
    }
    // indicates the type, or kind, of tag
    if (key=="kind")
    {
      m_type = value[0].latin1();
      switch (m_type) {
        case 'F': // filename
          m_pointTo = tFile;
        break;
        case 'd': // macro definitions (and #undef names)
        case 'f': // function definitions
        case 'S': // subroutines (fortran)
        case 't': // typedefs
        case 'v': // variable definitions
          m_pointTo = tDefinition;
        break;
        case 'c': // classes
        case 'e': // enumerators
        case 'g': // enumeration names
        case 'L': // fortran locals
        case 'm': // class, struct, or union members
        case 'n': // namespaces
        case 'p': // function prototypes and declarations
        case 's': // structure names
        case 'u': // union names
        case 'x': // extern and forward variable declarations
        default:
          m_pointTo = tDeclaration;
        break;
      }
    }
    // Indicates the visibility of this class member
    else if (key=="acces") {}
    // Indicates that the tag has file-limited visibility.
    else if (key=="file") {}
    // indicates a implementation (abstract vs. concrete, "virtual" or "pure virtual")
    else if (key=="implementation") {}
    // comma-separated list of classes from which this class is derived
    else if (key=="inherits") {}
    // unknown
    else {}
  }
}
int CTag::line() const
{
  bool ok = false;
  int l,ip;
  // strip ;
  if ((ip=m_excmd.find(";")) > 0)
    l = m_excmd.left(ip).toInt(&ok);
  else
    l = m_excmd.toInt(&ok);
  return ok?l:1;
}
/** return a string characterizing the tag type */
QString CTag::typeName() const
{
  QString s;
  switch (m_type) {
    case 'F':
      s = "filename";
    break;
    case 'd':
      s = "macro";
    break;
    case 'f':
      s = "function";
    break;
    case 'S':
      s = "subroutine";
    break;
    case 't':
      s = "typedef";
    break;
    case 'v':
      s = "variable";
    break;
    case 'c':
      s = "class";
    break;
    case 'e':
      s = "enumerator";
    break;
    case 'g':
      s = "enumeration name";
    break;
    case 'L':
      s = "local";
    break;
    case 'm':
      s = "member";
    break;
    case 'n':
      s = "namespace";
    break;
    case 'p':
      s = "prototype";
    break;
    case 's':
      s = "structure";
    break;
    case 'u':
      s = "union";
    break;
    case 'x':
      s = "extern";
    break;
    default:
      s = "unknown";
    break;
  }
  return s;
}
/** return true if the tag points to a file */
bool CTag::isFile() const
{
  return (m_pointTo==tFile?true:false);
}
/** return true if the tag points to definition */
bool CTag::isDefinition() const
{
  return (m_pointTo==tDefinition?true:false);
}
/** return true if the tag points to declaration */
bool CTag::isDeclaration() const
{
  return (m_pointTo==tDeclaration?true:false);
}
/** append a CTag to the list and count it */
CTagList::Iterator CTagList::append(const CTag& ctag)
{
  if (ctag.isDeclaration())
    m_ndeclarations++;
  else if (ctag.isDefinition())
    m_ndefinitions++;
  else if (ctag.isFile())
    m_nfiles++;
  return QValueList<CTag>::append(ctag);
}

///** return all file tags */
//CTagList CTagList::getFileTags() const {
//  CTagList fileTags(m_tag);
//  int ntags = count();
//  for (int it=0; it<ntags; ++it)
//  {
//    const CTag& ctag = (*this)[it];
//    char t = ctag.type();
//    if (t=='F'){
//      fileTags.QValueList<CTag>::append(ctag);
//      fileTags.m_nfiles++;
//    }
//  }
//  return fileTags;
//}
//
///** return all definition tags */
//CTagList CTagList::getDefinitionTags() const {
//  CTagList fileTags(m_tag);
//  int ntags = count();
//  for (int it=0; it<ntags; ++it)
//  {
//    const CTag& ctag = (*this)[it];
//    char t = ctag.type();
//    switch (t) {
//      case 'd': // macro definitions (and #undef names)
//      case 'f': // function definitions
//      case 'S': // subroutines (fortran)
//      case 't': // typedefs
//      case 'v': // variable definitions
//        fileTags.QValueList<CTag>::append(ctag);
//        fileTags.m_ndefinitions++;
//      break;
//      default:
//      break;
//    }
//  }
//  return fileTags;
//}
//
///** return all declaration tags */
//CTagList CTagList::getDeclarationTags() const {
//  CTagList fileTags(m_tag);
//  int ntags = count();
//  for (int it=0; it<ntags; ++it)
//  {
//    const CTag& ctag = (*this)[it];
//    char t = ctag.type();
//    switch (t) {
//      case 'c': // classes
//      case 'e': // enumerators
//      case 'g': // enumeration names
//      case 'L': // fortran locals
//      case 'm': // class, struct, or union members
//      case 'n': // namespaces
//      case 'p': // function prototypes and declarations
//      case 's': // structure names
//      case 'u': // union names
//      case 'x': // extern and forward variable declarations
//        fileTags.QValueList<CTag>::append(ctag);
//        fileTags.m_ndeclarations++;
//      break;
//      default:
//      break;
//    }
//  }
//  return fileTags;
//}

CTagsDataBase::CTagsDataBase()
  : m_init(false), m_taglistdict(17,true), m_filelist()
{
  m_taglistdict.setAutoDelete(true);
  kdDebug() << "in ctags constructor\n";
}
CTagsDataBase::~CTagsDataBase()
{
}


/** load a tags file and create the search database */
void CTagsDataBase::load(const QString& file)
{
  QFile tagsfile(file);
  if (tagsfile.open(IO_ReadOnly)) {
    kdDebug() << "tags file opened succefully, now reading...\n" ;
    QTextStream ts(&tagsfile);
    QString aline;
    int n=0;
    while (!ts.eof()) {
      ++n;
      aline = ts.readLine();
      // do some checking that we have the right version of ctags
      if (aline[0] == '!') {
        // we could look at the stuff here that ctags writes
        // and then move on
      }
      // exclude comments from being parsed
      else {
        // split line into components according to ctags manual
        QChar sep('\t');
        QStringList taglist = QStringList::split(sep,aline);
        // tag<TAB>filename<TAB>ex command"<TAB>ctags extension
        QStringList::Iterator it = taglist.begin();
        QStringList::Iterator ptag = it;
        QStringList::Iterator pfile = ++it;
        QStringList::Iterator pexcmd = ++it;
        QStringList::Iterator pext = ++it;
        CTagList* pTagList = m_taglistdict[*ptag];
        if (!pTagList) {
          pTagList = new CTagList(*ptag);
          if (!pTagList) {
            // error out of memory while attempting to create tags database
            return;
          }
          m_taglistdict.insert(*ptag,pTagList);
        }
        // we can probably avoid one expensive copy here if we
        // change the taglist from a QValuelist to a Qlist
        pTagList->append(CTag(*pfile,*pexcmd,*pext));
      }
    }
  }
  else {
    // "Unable to open tags file: %1"
    kdDebug() << "cant open tags file: " << file << "\n";
    return;
  }
  tagsfile.close();
  m_filelist.append(file);
  m_init = true;
}


/** unload, i.e. destroy the current search database */
void CTagsDataBase::unload()
{
  /* Since auto deletion is enabled this should
     take care of the entire data structure. It will delete
     all taglists, which in turn are value based so they
     will be really deleted, no really. */
  m_taglistdict.clear();
  m_init = false;
}


/** clear, unload database and remove file list */
void CTagsDataBase::clear()
{
  unload();
  m_filelist.clear();
}


/** reload, unload current and load regenerated search database */
void CTagsDataBase::reload()
{
  if (m_init) unload();
  QStringList::Iterator it;
  for (it=m_filelist.begin();it!=m_filelist.end();++it)
  {
    load(*it);
  }
}


/** the number of CTag entries found for a tag */
int CTagsDataBase::nCTags(const QString& tag) const
{
  if (!m_init) return 0;
  CTagList* ctaglist = m_taglistdict[tag];
  if (!ctaglist) return 0;
  return ctaglist->count();
}


/** return the list of CTag entries for a tag */
const CTagList* CTagsDataBase::ctaglist(const QString& tag) const
{
  if (!m_init) return 0L;
  return m_taglistdict[tag];
}
