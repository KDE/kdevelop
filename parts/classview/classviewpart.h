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

#include <qguardedptr.h>
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
class KListViewAction;
class QListViewItem;
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

    QMap<QString, NamespaceItem*> nsmap;
    QMap<ClassDom, ClassItem*> clmap;
    QMap<FunctionDom, FunctionItem*> fnmap;

    KListViewAction *m_namespaces;
    KListViewAction *m_classes;
    KListViewAction *m_functions;

    NamespaceItem *global_item;
    
    KListViewAction *m_functionsnav;
    Navigator *navigator;

    void updateClassesForAdd(NamespaceDom nsdom);
    void updateFunctionsForAdd(ClassDom cldom);

private slots:
    void selectNamespace(QListViewItem*);
    void selectClass(QListViewItem*);
    void selectFunction(QListViewItem*);
    void slotProjectOpened();
    void slotProjectClosed();
    void refresh();
    void removeFile(const QString& fileName);
    void addFile(const QString& fileName);
    void removeNamespace(const QString &name);
    void switchedViewPopup();
    void graphicalClassView();

    void goToFunctionDeclaration();
    void goToFunctionDefinition();
    void goToClassDeclaration();
    void goToNamespaceDeclaration();
    void selectedAddClass();
    void selectedAddMethod();
    void selectedAddAttribute();

    void focusNamespaces();
    void focusClasses();
    void focusFunctions();
    void unfocusNamespaces();
    void unfocusClasses();
    void unfocusFunctions();

    void syncCombos();
    void activePartChanged(KParts::Part*);

private:
    void setupActions();

    NamespaceDom syncNamespaces(const FileDom &dom);
    ClassDom syncClasses(const NamespaceDom &dom);
    FunctionDom syncFunctions(const ClassDom &dom);

    QGuardedPtr<ClassViewWidget> m_widget;

    bool sync;

    KAction *m_followCode;
    KToolBarPopupAction *m_popupAction;

    QString m_activeFileName;
    KTextEditor::Document* m_activeDocument;
    KTextEditor::View* m_activeView;
    KTextEditor::SelectionInterface* m_activeSelection;
    KTextEditor::EditInterface* m_activeEditor;
    KTextEditor::ViewCursorInterface* m_activeViewCursor;

    friend class NamespaceItem;
    friend class ClassItem;
    friend class FunctionItem;
    friend class Navigator;
};


#endif
