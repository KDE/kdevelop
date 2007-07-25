/* This file is part of KDevelop
 *  Copyright 2002 Harald Fernengel <harry@kdevelop.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

 */

#ifndef VALGRIND_PART_H
#define VALGRIND_PART_H

#include <qpointer.h>

#include <qstringlist.h>
#include <kdevplugin.h>

class QTreeView;
class ValgrindModel;
class ValgrindControl;

class ValgrindPart : public KDevPlugin
{
  Q_OBJECT

public:
  ValgrindPart( QObject *parent, const QStringList & );
  ~ValgrindPart();

  void runValgrind( const QString& exec, const QString& parameters, const QString& valExec, const QString& valParameters );

  void savePartialProjectSession( QDomElement* el );
  void restorePartialProjectSession( const QDomElement* el );

  QWidget* pluginView() const;
  Qt::DockWidgetArea dockWidgetAreaHint() const;

private slots:
  void slotExecValgrind();
  void slotExecCalltree();
  void slotKillValgrind();
  void slotStopButtonClicked( KDevPlugin* which );
  void loadOutput();
  void projectOpened();

private:
  void clear();

  QString m_lastExec, m_lastParams, m_lastValExec, m_lastValParams,
      m_lastCtExec, m_lastCtParams, m_lastKcExec;

  QPointer<QTreeView> m_treeView;

  ValgrindModel* m_model;
  ValgrindControl* m_control;
};


#endif
