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
#include <kprocess.h>
#include "../cproject.h"
#include "cctags.h"


CTagsDataBase::CTagsDataBase()
  : m_init(false), m_taglistdict(), m_file(QString::null)
{
  m_taglistdict.setAutoDelete(true);
  kdDebug() << "in ctags constructor\n";
}
CTagsDataBase::~CTagsDataBase()
{
}

/** create a tags file for a CProject */
void CTagsDataBase::create_tags(KShellProcess& process, CProject& project)
{
  kdDebug() << "creating tags file\n";
  // collect all files belonging to the project
  QString files;
  QStrListIterator isrc(project.getSources());
  while (!isrc.atLast())
  {
      files = files + *isrc + " ";
      ++isrc;
  }
  QStrListIterator ihdr(project.getHeaders());
  while (!ihdr.atLast())
  {
      files = files + *ihdr + " ";
      ++ihdr;
  }
  // get the ctags command
  CtagsCommand& cmd = project.ctags_cmd();
  // set the name of the output file
  m_file = project.getProjectDir() + "/tags";

  // use a shell_process that was already set up
  process.clearArguments();
  process << cmd.m_command;
  process << cmd.m_totals;
  process << cmd.m_excmd_pattern;
  process << cmd.m_file_scope;
  process << cmd.m_file_tags;
  process << cmd.m_ctypes;
  process << "-f" ;
  process << m_file ;
  process << files ;
  process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
}


/** load a tags file and create the search database */
void CTagsDataBase::load()
{
  if (!QFileInfo(m_file).exists()) return;
  QFile tagsfile(m_file);
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
        // tag<TAB>filename<TAB>ex command<TAB>ctags extension
        QStringList::Iterator it = taglist.begin();
        QStringList::Iterator ptag = it;
        QStringList::Iterator pfile = ++it;
        QStringList::Iterator pexcmd = ++it;
        QStringList::Iterator pext = ++it;
        //kdDebug() << "1 tag    2 filename    3 ex command    4 ctags ext\n" ;
        //kdDebug() << (*ptag).latin1() << "    "
        //          << (*pfile).latin1() << "    "
        //          << (*pexcmd).latin1() << "    "
        //          << (*pext).latin1() << "\n";
        CTagList* pTagList = m_taglistdict[*ptag];
        if (!pTagList) {
          pTagList = new CTagList(*ptag);
          if (!pTagList) {
            // error out of memory while attempting to create tags database
            return;
          }
          m_taglistdict.insert(*ptag,pTagList);
        }
        pTagList->append(CTag(*pfile,*pexcmd,*pext));
      }
    }
  }
  else {
    // "Unable to open tags file: %1"
    return;
  }
  tagsfile.close();
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


/** reload, unload current and load regenerated search database */
void CTagsDataBase::reload()
{
  if (m_init) unload();
  load();
}
