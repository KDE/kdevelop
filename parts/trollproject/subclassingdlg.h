/***************************************************************************
 *   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
 *   jsgaarde@tdcspace.dk                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SUBCLASSINGDLG_H
#define SUBCLASSINGDLG_H

#include "subclassingdlgbase.h"
#include "trollprojectwidget.h"

class QStringList;
class QDomDocument;
class QCheckListItem;
class QListViewItem;


class SlotItem : public QCheckListItem
{
  public:
                  SlotItem(QListView *parent,const QString &text,
                           const QString &specifier, const QString &Access,
                           const QString &returnType,bool isFunc,
                           bool callBaseClass=false);
  QString         m_access;
  QString         m_methodName;
  QString         m_returnType;
  QString         m_specifier;
  bool            m_isFunc;
  bool            m_callBaseClass;
};


class SubclassingDlg : public SubclassingDlgBase
{
public:
                        SubclassingDlg(const QString &formFile,QStringList &newFileNames,
                                       QWidget* parent = 0, const char* name = 0,
                                       bool modal = FALSE, WFlags fl = 0 );
                        ~SubclassingDlg();

private:
  void                  updateDlg();
  bool                  replaceKeywords(QString &buffer);
  void                  replace(QString &string, const QString& search, const QString& replace);
  bool                  saveBuffer(QString &buffer, const QString& filename);
  bool                  loadBuffer(QString &buffer, const QString& filename);

public slots:
  virtual void          accept();
  virtual void          onChangedClassName();

protected:
  QStringList           &m_newFileNames;
  QString               m_formFile;
  QString               m_baseClassName;
  QString               m_qtBaseClassName;
  QString               m_baseCaption;
  QString               m_formName;
  QString               m_formPath;
  QValueList<SlotItem*> m_slots;
  QValueList<SlotItem*> m_functions;

};

#endif

