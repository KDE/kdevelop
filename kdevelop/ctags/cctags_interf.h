/***************************************************************************
                          cctags_interf.h  -
                          ctags interface declaration
                             -------------------
    begin                : Tue Apr 24 2001
    copyright            : (C) 2001 by rokrau and the kdevelop-team
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
#ifndef CCTAGS_INTERF_H
#define CCTAGS_INTERF_H

#include <qstring.h>

/**
 * \class CTagsCommandLine
 *
 * \brief ctags command and command line options
 *
 * \author rokrau@yahoo.com
 **/
class CTagsCommandLine {
public:
  CTagsCommandLine() {                    // long form of arguments
    m_onlyProjectFiles=false;
    m_command = "ctags";
    m_append = "";                        //--append=no      [default]
    m_sort = "-u";                        //--sort=no
    m_totals = "--totals";                //--totals=yes
    m_excmd_pattern = "-n";               //--excmd=number
    m_file_scope = "";                    //--file-scope=yes [default]
    m_c_types = "--c++-types=+px";
    m_fortran_types = "--fortran-types=-l+L";
    m_fields = "--fields=+i";
    m_extra = " --extra=+f";
    m_recurse = "-R";                     // --recurse=on
    m_exclude.append("--exclude=[Mm]ake*");
    m_exclude.append("--exclude=config*");
    m_exclude.append("--exclude=*.inc");
    m_exclude.append("--exclude=*.bck");
    m_exclude.append("--exclude=glimpse*");
    m_exclude.append("--exclude=.glimpse*");
    m_exclude.append("--exclude=CVS");
  }
  ~CTagsCommandLine() {}
  /** access to onlyProjectFiles */
  bool onlyProjectFiles() {return m_onlyProjectFiles;} const
  void onlyProjectFiles(const bool onlyProjectFiles) {m_onlyProjectFiles=onlyProjectFiles;}
  /** access to command */
  QString command() {return m_command;} const
  void command(const QString& command) {m_command=command;}
  /** access to append option */
  QString append() {return m_append;} const
  void append(const QString& append) {m_append=append;}
  /** access to sort option */
  QString sort() {return m_sort;} const
  void sort(const QString& sort) {m_sort=sort;}
  /** access to totals option */
  QString totals() {return m_totals;} const
  void totals(const QString& totals) {m_totals=totals;}
  /** access to excmd_pattern option */
  QString excmd_pattern() {return m_excmd_pattern;} const
  void excmd_pattern(const QString& excmd_pattern) {m_excmd_pattern=excmd_pattern;}
  /** access to file_scope option */
  QString file_scope() {return m_file_scope;} const
  void file_scope(const QString& file_scope) {m_file_scope=file_scope;}
  /** access to c_types option */
  QString c_types() {return m_c_types;} const
  void c_types(const QString& c_types) {m_c_types=c_types;}
  /** access to fortran_types option */
  QString fortran_types() {return m_fortran_types;} const
  void fortran_types(const QString& fortran_types) {m_fortran_types=fortran_types;}
  /** access to fields option */
  QString fields() {return m_fields;} const
  void fields(const QString& fields) {m_fields=fields;}
  /** access to extra option */
  QString extra() {return m_extra;} const
  void extra(const QString& extra) {m_extra=extra;}
  /** access to recuse option */
  QString recurse() {return m_recurse;} const
  void recurse(const QString& recurse) {m_recurse=recurse;}
  /** access to exclude list */
  QStringList exclude() {return m_exclude;} const
  void exclude(const QStringList& exclude) {m_exclude=exclude;}
private:
  bool m_onlyProjectFiles;
  QString m_command;
  QString m_append;
  QString m_sort;
  QString m_totals;
  QString m_excmd_pattern;
  QString m_file_scope;
  QString m_c_types;
  QString m_fortran_types;
  QString m_fields;
  QString m_extra;
  QString m_recurse;
  QStringList m_exclude;
};


#endif // CCTAGS_INTERF_H
