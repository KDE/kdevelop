/*
 *  Copyright (C) 2003 Alexander Dymo (cloudtemple@mksat.net)
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
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */
#include <qlistview.h>
#include "codemodel.h"

class ClassViewPart;
class QComboView;

class NamespaceItem: public QListViewItem{
public:
    NamespaceItem(ClassViewPart *part, QListView *parent, QString name, NamespaceDom dom);
    NamespaceItem(ClassViewPart *part, QListViewItem *parent, QString name, NamespaceDom dom);
    NamespaceDom dom() const;
    virtual void setup();
private:
    NamespaceDom m_dom;
    ClassViewPart *m_part;
};

class ClassItem: public QListViewItem{
public:
    ClassItem(ClassViewPart *part, QListView *parent, QString name, ClassDom dom);
    ClassItem(ClassViewPart *part, QListViewItem *parent, QString name, ClassDom dom);
    ClassDom dom() const;
    virtual void setup();
private:
    ClassDom m_dom;
    ClassViewPart *m_part;
};

class FunctionItem: public QListViewItem{
public:
    FunctionItem(ClassViewPart *part, QListView *parent, QString name, FunctionDom dom);
    FunctionItem(ClassViewPart *part, QListViewItem *parent, QString name, FunctionDom dom);
    FunctionDom dom() const;
    virtual void setup();
private:
    FunctionDom m_dom;
    ClassViewPart *m_part;
};

namespace ViewCombosOp{

void refreshNamespaces(ClassViewPart *part, QComboView *view);
void refreshClasses(ClassViewPart *part, QComboView *view, const NamespaceDom &dom);
void refreshFunctions(ClassViewPart *part, QComboView *view, const ClassDom & dom);
void refreshFunctions(ClassViewPart *part, QComboView *view, const NamespaceDom & dom);

void processNamespace( ClassViewPart *part, NamespaceItem * item );
void processClass( ClassViewPart *part, ClassItem * item );
void processFunction( ClassViewPart *part, FunctionItem * item );

}
