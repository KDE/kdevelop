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
 * Partially based on KDE Studio ClassListView http://www.thekompany.com/projects/kdestudio/
 */

#ifndef __CLASSVIEW_WIDGET_H__
#define __CLASSVIEW_WIDGET_H__

#include <klistview.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <codemodel.h>

class KDevProject;
class ClassViewPart;
class ClassViewItem;
class FolderBrowserItem;
class NamespaceDomBrowserItem;
class ClassDomBrowserItem;
class FunctionDomBrowserItem;
class VariableDomBrowserItem;
class KSelectAction;

class ClassViewWidget : public KListView
{
    Q_OBJECT
public:
    enum ViewMode
    {
	KDevelop3ViewMode = 0,
	KDevelop2ViewMode,
	JavaLikeViewMode
    };

public:
    ClassViewWidget( ClassViewPart *part );
    virtual ~ClassViewWidget();

    int viewMode() const;
    void setViewMode( int mode );

    void clear();

private slots:
    void slotNewClass();
    void slotAddMethod();
    void slotAddAttribute();
    void slotOpenDeclaration();
    void slotOpenImplementation();

protected:
    void contentsContextMenuEvent( QContextMenuEvent* );

private slots:
    void slotProjectOpened();
    void slotProjectClosed();
    void refresh();
    void insertFile( const QString& fileName );
    void removeFile( const QString& fileName );
    void slotExecuted( QListViewItem* item );

private:
    ClassViewPart* m_part;
    QStringList removedText;
    QString m_projectDirectory;
    int m_projectDirectoryLength;
    FolderBrowserItem* m_projectItem;
    KSelectAction* m_actionViewMode;
    KAction* m_actionNewClass;
    KAction* m_actionAddMethod;
    KAction* m_actionAddAttribute;
    KAction* m_actionOpenDeclaration;
    KAction* m_actionOpenImplementation;

    friend class ClassViewItem;
    friend class FolderBrowserItem;
    friend class NamespaceDomBrowserItem;
    friend class ClassDomBrowserItem;
    friend class FunctionDomBrowserItem;
    friend class VariableDomBrowserItem;
};

class ClassViewItem: public KListViewItem
{
public:
    ClassViewItem( QListView* parent, const QString& text=QString::null )
    	: KListViewItem( parent, text ) {}
    ClassViewItem( QListViewItem* parent, const QString& text=QString::null )
    	: KListViewItem( parent, text ) {}
    
    virtual const CodeModelItem* model() const { return 0; }

    virtual bool isFolder() const { return false; }
    virtual bool isFile() const { return false; }
    virtual bool isNamespace() const { return false; }
    virtual bool isClass() const { return false; }
    virtual bool isFunction() const { return false; }
    virtual bool isVariable() const { return false; }

    virtual bool hasDeclaration() const { return false; }
    virtual bool hasImplementation() const { return false; }

    virtual void openDeclaration() {}
    virtual void openImplementation() {}

    ClassViewWidget* listView() { return static_cast<ClassViewWidget*>( KListViewItem::listView() ); }
    const ClassViewWidget* listView() const { return static_cast<ClassViewWidget*>( KListViewItem::listView() ); }
};

class FolderBrowserItem: public ClassViewItem
{
public:
    FolderBrowserItem( QListView* parent, const QString& name=QString::null )
    	: ClassViewItem( parent, name ) {}
    FolderBrowserItem( QListViewItem* parent, const QString& name=QString::null )
    	: ClassViewItem( parent, name ) {}

    virtual bool isFolder() const { return true; }

    void setup();
    QString key( int, bool ) const;

    void processFile( FileDom file, QStringList& path, bool remove=false );
    void processNamespace( NamespaceDom ns, bool remove=false );
    void processClass( ClassDom klass, bool remove=false );
    void processFunction( FunctionDom fun, bool remove=false );
    void processVariable( VariableDom var, bool remove=false );

private:
    QMap<QString, FolderBrowserItem*> m_folders;
    QMap<QString, NamespaceDomBrowserItem*> m_namespaces;
    QMap<ClassDom, ClassDomBrowserItem*> m_classes;
    QMap<FunctionDom, FunctionDomBrowserItem*> m_functions;
    QMap<VariableDom, VariableDomBrowserItem*> m_variables;
};

class NamespaceDomBrowserItem: public ClassViewItem
{
public:
    NamespaceDomBrowserItem( QListView* parent, NamespaceDom dom )
    	: ClassViewItem( parent, dom->name() ), m_dom( dom ) {}
    NamespaceDomBrowserItem( QListViewItem* parent, NamespaceDom dom )
    	: ClassViewItem( parent, dom->name() ), m_dom( dom ) {}

    const CodeModelItem* model() const { return m_dom; }
    
    virtual bool isNamespace() const { return true; }

    void setup();
    QString key( int, bool ) const;

    void processNamespace( NamespaceDom ns, bool remove=false );
    void processClass( ClassDom klass, bool remove=false );
    void processFunction( FunctionDom fun, bool remove=false );
    void processVariable( VariableDom var, bool remove=false );

    NamespaceDom dom() { return m_dom; }

private:
    NamespaceDom m_dom;
    QMap<QString, NamespaceDomBrowserItem*> m_namespaces;
    QMap<ClassDom, ClassDomBrowserItem*> m_classes;
    QMap<FunctionDom, FunctionDomBrowserItem*> m_functions;
    QMap<VariableDom, VariableDomBrowserItem*> m_variables;
};

class ClassDomBrowserItem: public ClassViewItem
{
public:
    ClassDomBrowserItem( QListView* parent, ClassDom dom )
    	: ClassViewItem( parent, dom->name() ), m_dom( dom ) {}
    ClassDomBrowserItem( QListViewItem* parent, ClassDom dom )
    	: ClassViewItem( parent, dom->name() ), m_dom( dom ) {}

    const CodeModelItem* model() const { return m_dom; }
    virtual bool isClass() const { return true; }

    virtual bool hasDeclaration() const { return true; }
    virtual void openDeclaration();

    void setup();
    QString key( int, bool ) const;

    void processClass( ClassDom klass, bool remove=false );
    void processFunction( FunctionDom fun, bool remove=false );
    void processVariable( VariableDom var, bool remove=false );

    ClassDom dom() { return m_dom; }

private:
    ClassDom m_dom;
    QMap<ClassDom, ClassDomBrowserItem*> m_classes;
    QMap<FunctionDom, FunctionDomBrowserItem*> m_functions;
    QMap<VariableDom, VariableDomBrowserItem*> m_variables;
};

class FunctionDomBrowserItem: public ClassViewItem
{
public:
    FunctionDomBrowserItem( QListView* parent, FunctionDom dom )
    	: ClassViewItem( parent, dom->name() ), m_dom( dom ) {}
    FunctionDomBrowserItem( QListViewItem* parent, FunctionDom dom )
    	: ClassViewItem( parent, dom->name() ), m_dom( dom ) {}

    const CodeModelItem* model() const { return m_dom; }
    virtual bool isFunction() const { return true; }

    virtual bool hasDeclaration() const { return true; }
    virtual bool hasImplementation() const;

    virtual void openDeclaration();
    virtual void openImplementation();

    void setup();
    QString key( int, bool ) const;

    FunctionDom dom() { return m_dom; }

private:
    FunctionDom m_dom;
};

class VariableDomBrowserItem: public ClassViewItem
{
public:
    VariableDomBrowserItem( QListView* parent, VariableDom dom )
    	: ClassViewItem( parent, dom->name() ), m_dom( dom ) {}
    VariableDomBrowserItem( QListViewItem* parent, VariableDom dom )
    	: ClassViewItem( parent, dom->name() ), m_dom( dom ) {}

    const CodeModelItem* model() const { return m_dom; }
    virtual bool isVariable() const { return true; }

    virtual bool hasDeclaration() const { return true; }
    virtual bool hasImplementation() const { return false; }

    virtual void openDeclaration();
    virtual void openImplementation();

    void setup();
    QString key( int, bool ) const;

    VariableDom dom() { return m_dom; }

private:
    VariableDom m_dom;
};

#endif
