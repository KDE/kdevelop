/*
 *  Copyright (C) 2003 Roberto Raggi (roberto@kdevelop.org)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#ifndef __KDEVPART_CLASSVIEW_H__
#define __KDEVPART_CLASSVIEW_H__

#include "kdevlanguagesupport.h"
#include "viewcombos.h"

#include <qpointer.h>
#include <qmap.h>

#include <kdevplugin.h>

#include <codemodel.h>

namespace KParts { class Part; }
namespace KTextEditor
{
    class Document;
    class View;
    class EditInterface;
    class SelectionInterface;
    class ViewCursorInterface;
}


class ClassViewWidget;
class QTreeWidgetAction;
class QTreeWidgetItem;
class KToolBarPopupAction;
class NamespaceItem;
class Navigator;

class ClassViewPart : public KDevPlugin
{
    Q_OBJECT
public:
    ClassViewPart(QObject *parent, const char *name, const QStringList &);
    virtual ~ClassViewPart();

    bool langHasFeature(KDevLanguageSupport::Features feature);

#if 0
    QTreeWidgetAction *m_functionsnav;
#endif
    Navigator *navigator;

private slots:
    void slotProjectOpened();
    void slotProjectClosed();
    void graphicalClassView();

    void activePartChanged(KParts::Part*);

private:
    void setupActions();

    QPointer<ClassViewWidget> m_widget;

    QString m_activeFileName;
    KTextEditor::Document* m_activeDocument;
    KTextEditor::View* m_activeView;
    KTextEditor::SelectionInterface* m_activeSelection;
    KTextEditor::EditInterface* m_activeEditor;
    KTextEditor::ViewCursorInterface* m_activeViewCursor;

    friend class Navigator;
};


#endif
