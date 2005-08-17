/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _APPOUTPUTWIDGET_H_
#define _APPOUTPUTWIDGET_H_

#include <qstringlist.h>
#include <qregexp.h>

#include "processwidget.h"

class AppOutputViewPart;

enum FilterType {
  eNoFilter=0,
  eFilterStr=1,
  eFilterRegExp=2
};

class AppOutputWidget : public ProcessWidget
{
  Q_OBJECT
  friend class AppOutputViewPart;
public:
  AppOutputWidget(AppOutputViewPart* part);
  virtual ~AppOutputWidget();

public slots:
  void slotRowSelected(QListWidgetItem* row);
  void insertStdoutLine(const QString &line);
  void insertStderrLine(const QString &line);

private slots:
  void popupContextMenu(const QPoint &pt);

private:
  virtual void childFinished(bool normal, int status);

  QStringList strList;
  FilterType iFilterType;
  QString strFilterStr;
  bool bCS;

  AppOutputViewPart* m_part;
  QRegExp assertMatch;
  QRegExp lineInfoMatch;
};

#endif
