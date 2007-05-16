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
#ifndef QTDESIGNER_PLUGIN_H
#define QTDESIGNER_PLUGIN_H

#include <QHash>
#include <QtCore/QPointer>
#include "iplugin.h"

class QAction;
class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QWorkspace;
class QtDesignerDocumentFactory;
class QtDesignerToolViewFactory;
class KAboutData;
class KAction;

namespace KDevelop
{
  class IDocument;
}

template<class T> class QList;

typedef QHash<QAction*, QAction*> DesignerActionHash;


class QtDesignerPlugin: public KDevelop::IPlugin
{
    Q_OBJECT
public:
    QtDesignerPlugin(QObject *parent, const QStringList &args);
    virtual ~QtDesignerPlugin();

    void setupActions();

//   virtual bool openFile();
//   virtual bool saveFile();

    QDesignerFormEditorInterface *designer() const;
public slots:
    void activateDocument( KDevelop::IDocument* );
protected:

private:

private Q_SLOTS:
    void saveActiveDocument();
private:
    QPointer<QDesignerFormEditorInterface> m_designer;

    KDevelop::IDocument* m_activeDoc;
    QtDesignerDocumentFactory* m_docFactory;
    QtDesignerToolViewFactory* m_widgetBoxFactory;
    QtDesignerToolViewFactory* m_propertyEditorFactory;
    QtDesignerToolViewFactory* m_objectInspectorFactory;
    QtDesignerToolViewFactory* m_actionEditorFactory;
};

#endif // QTDESIGNER_PART_H
//kate: space-indent on; indent-width 2; replace-tabs on; indent-mode cstyle;
