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
  CTagsCommandLine() {
    m_command = "ctags";
    m_append = "--append=no";
    m_sort = "--sort=no";
    m_totals = "--totals=yes";
    m_excmd_pattern = "--excmd=number"; //pattern
    m_file_scope = "--file-scope=yes";
    m_file_tags = "--file-tags=yes";
    m_c_types = "--c-types=+px";
    m_fortran_types = "--fortran-types=-l+L";
    m_exclude = "--exclude=\"*.inc *.bck glimpse*\"";
    m_fields = "--fields=+i";
  }
  ~CTagsCommandLine() {}
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
  /** access to file_tags option */
  QString file_tags() {return m_file_tags;} const
  void file_tags(const QString& file_tags) {m_file_tags=file_tags;}
  /** access to c_types option */
  QString c_types() {return m_c_types;} const
  void c_types(const QString& c_types) {m_c_types=c_types;}
  /** access to fortran_types option */
  QString fortran_types() {return m_fortran_types;} const
  void fortran_types(const QString& fortran_types) {m_fortran_types=fortran_types;}
  /** access to exclude option */
  QString exclude() {return m_exclude;} const
  void exclude(const QString& exclude) {m_exclude=exclude;}
  /** access to fields option */
  QString fields() {return m_fields;} const
  void fields(const QString& fields) {m_fields=fields;}
private:
  QString m_command;
  QString m_append;
  QString m_sort;
  QString m_totals;
  QString m_excmd_pattern;
  QString m_file_scope;
  QString m_file_tags;
  QString m_c_types;
  QString m_fortran_types;
  QString m_exclude;
  QString m_fields;
};


#endif // CCTAGS_INTERF_H
