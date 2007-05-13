/***************************************************************************
 *   Copyright (C) 2005 by Roberto Raggi <roberto@kdevelop.org>            *
 *   Copyright (C) 2005 by Harald Fernengel <harry@kdevelop.org>           *
 *   Copyright (C) 2006 by Matt Rogers <mattr@kde.org>                     *
 *   Copyright (C) 2007 by Andreas Pakulat <apaku@gmx.de>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef QTDESIGNER_PART_H
#define QTDESIGNER_PART_H

#include <QObject>
#include <QHash>
#include <QtCore/QPointer>
#include <kparts/part.h>

class QAction;
class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QWorkspace;

class KAboutData;
class KAction;

namespace KDevelop
{
  class IDocument;
}

template<class T> class QList;

typedef QHash<QAction*, QAction*> DesignerActionHash;


class QtDesignerPart: public KDevelop::IPlugin
{
  Q_OBJECT
public:
  QtDesignerPart(QWidget* parentWidget, QObject *parent, const QStringList &args);
  virtual ~QtDesignerPart();

  static KAboutData* createAboutData();

  void setupActions();

  virtual bool openFile();
  virtual bool saveFile();

  QDesignerFormEditorInterface *designer() const;

protected:
  bool eventFilter(QObject*, QEvent*);

private:
  //wrap the actions provided by QDesignerFormWindowManagerInterface in
  //KActions
  QAction* wrapDesignerAction( QAction*, KActionCollection*, const char* );
  void updateDesignerAction( QAction*, QAction* );

private Q_SLOTS:
  void updateDesignerActions();
  void activated( KDevelop::Document* );


private:
  QPointer<QDesignerFormEditorInterface> m_designer;
  QPointer<QDesignerFormWindowInterface> m_window;
  QPointer<QWorkspace> m_workspace;

  DesignerActionHash m_designerActions;

};

#endif // QTDESIGNER_PART_H
//kate: space-indent on; indent-width 2; replace-tabs on; indent-mode cstyle;
