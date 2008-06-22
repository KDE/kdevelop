/***************************************************************************
 *   Copyright 2005 Roberto Raggi <roberto@kdevelop.org>            *
 *   Copyright 2005 Harald Fernengel <harry@kdevelop.org>           *
 *   Copyright 2006 Matt Rogers <mattr@kde.org>                     *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                  *
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
#include <iplugin.h>
#include <QtCore/QVariant>

class QAction;
class QDesignerIntegrationInterface;
class QDesignerFormEditorInterface;
class QtDesignerDocumentFactory;
class QtDesignerToolViewFactory;

namespace KDevelop
{
  class IDocument;
}

template<class T> class QList;

typedef QHash<QAction*, QAction*> DesignerActionHash;


/**
 * This is a plugin that embeds Qt designer into a kdevplatform application
 *
 * TODO: - fix moving of form contents via mouse
 * TODO: - properly enable/disable save button
 * TODO: - detect external changes by using QFileSystemWatcher (more reliable implementation than KDirWatch)
 * TODO: - fix other ui bugs, like having to double-click in a menubar to edit it
 */
class QtDesignerPlugin: public KDevelop::IPlugin
{
    Q_OBJECT
public:
    QtDesignerPlugin(QObject *parent, const QVariantList &args = QVariantList() );
    virtual ~QtDesignerPlugin();

//   virtual bool openFile();
//   virtual bool saveFile();

    QDesignerFormEditorInterface *designer() const;
public Q_SLOTS:
//     void activateDocument( KDevelop::IDocument* );
protected:

private:

private Q_SLOTS:
//     void saveActiveDocument();
private:
    QDesignerIntegrationInterface* m_designer;

//     KDevelop::IDocument* m_activeDoc;
    QtDesignerDocumentFactory* m_docFactory;
    QtDesignerToolViewFactory* m_widgetBoxFactory;
    QtDesignerToolViewFactory* m_propertyEditorFactory;
    QtDesignerToolViewFactory* m_objectInspectorFactory;
    QtDesignerToolViewFactory* m_actionEditorFactory;
};

#endif // QTDESIGNER_PART_H
//kate: space-indent on; indent-width 2; replace-tabs on; indent-mode cstyle;
