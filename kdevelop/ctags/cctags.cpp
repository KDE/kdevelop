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


CTagsDataBase::CTagsDataBase() : m_init(false)
{
    kdDebug() << "in ctags constructor\n";
}
CTagsDataBase::~CTagsDataBase(){
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
