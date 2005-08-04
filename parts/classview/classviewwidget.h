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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 * Partially based on KDE Studio ClassListView http://www.thekompany.com/projects/kdestudio/
 */

#ifndef __CLASSVIEW_WIDGET_H__
#define __CLASSVIEW_WIDGET_H__

#include <klistview.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <QContextMenuEvent>
#include <codemodel.h>

class KDevProject;
class ClassViewPart;
class ClassViewItem;
class FolderBrowserItem;
class NamespaceDomBrowserItem;
class ClassDomBrowserItem;
class TypeAliasDomBrowserItem;
class FunctionDomBrowserItem;
class VariableDomBrowserItem;
class KSelectAction;

class ClassViewWidget : public KListView, public QToolTip
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

signals:
    void removedNamespace(const QString &name);

private slots:
    void slotNewClass();
    void slotAddMethod();
    void slotAddAttribute();
    void slotOpenDeclaration();
    void slotOpenImplementation();
	void slotCreateAccessMethods();

protected:
    void contentsContextMenuEvent( QContextMenuEvent* );
	void maybeTip( QPoint const & );

private slots:
    void slotProjectOpened();
    void slotProjectClosed();
    void refresh();
    void insertFile( const QString& fileName );
    void removeFile( const QString& fileName );
    void slotExecuted( Q3ListViewItem* item );

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
	KAction* m_actionCreateAccessMethods;
    friend class ClassViewItem;
    friend class FolderBrowserItem;
    friend class NamespaceDomBrowserItem;
    friend class ClassDomBrowserItem;
    friend class TypeAliasDomBrowserItem;
    friend class FunctionDomBrowserItem;
    friend class VariableDomBrowserItem;
};

class ClassViewItem: public KListViewItem
{
public:
    ClassViewItem( Q3ListView* parent, const QString& text=QString::null )
    	: KListViewItem( parent, text ) {}
    ClassViewItem( Q3ListViewItem* parent, const QString& text=QString::null )
    	: KListViewItem( parent, text ) {}
    
    virtual const CodeModelItem* model() const { return 0; }

    virtual bool isFolder() const { return false; }
    virtual bool isFile() const { return false; }
    virtual bool isNamespace() const { return false; }
    virtual bool isClass() const { return false; }
    virtual bool isFunction() const { return false; }
    virtual bool isVariable() const { return false; }
    virtual bool isTypeAlias() const { return false; }

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
    FolderBrowserItem( ClassViewWidget* widget, Q3ListView* parent, const QString& name=QString::null )
    	: ClassViewItem( parent, name ), m_widget(widget) {}
    FolderBrowserItem( ClassViewWidget* widget, Q3ListViewItem* parent, const QString& name=QString::null )
    	: ClassViewItem( parent, name ), m_widget(widget) {}

    virtual bool isFolder() const { return true; }

    void setup();
    QString key( int, bool ) const;

    void processFile( FileDom file, QStringList& path, bool remove=false );
    void processNamespace( NamespaceDom ns, bool remove=false );
    void processClass( ClassDom klass, bool remove=false );
    void processTypeAlias( TypeAliasDom typeAlias, bool remove=false );
    void processFunction( FunctionDom fun, bool remove=false );
    void processVariable( VariableDom var, bool remove=false );

private:
    QMap<QString, FolderBrowserItem*> m_folders;
    QMap<QString, NamespaceDomBrowserItem*> m_namespaces;
    QMap<ClassDom, ClassDomBrowserItem*> m_classes;
    QMap<TypeAliasDom, TypeAliasDomBrowserItem*> m_typeAliases;
    QMap<FunctionDom, FunctionDomBrowserItem*> m_functions;
    QMap<VariableDom, VariableDomBrowserItem*> m_variables;

    ClassViewWidget* m_widget;
};

class NamespaceDomBrowserItem: public ClassViewItem
{
public:
    NamespaceDomBrowserItem( Q3ListView* parent, NamespaceDom dom )
    	: ClassViewItem( parent, dom->name() ), m_dom( dom ) {}
    NamespaceDomBrowserItem( Q3ListViewItem* parent, NamespaceDom dom )
    	: ClassViewItem( parent, dom->name() ), m_dom( dom ) {}

    const CodeModelItem* model() const { return m_dom; }
    
    virtual bool isNamespace() const { return true; }

    void setup();
    QString key( int, bool ) const;

    void processNamespace( NamespaceDom ns, bool remove=false );
    void processClass( ClassDom klass, bool remove=false );
    void processTypeAlias( TypeAliasDom typeAlias, bool remove=false );
    void processFunction( FunctionDom fun, bool remove=false );
    void processVariable( VariableDom var, bool remove=false );

    NamespaceDom dom() { return m_dom; }

private:
    NamespaceDom m_dom;
    QMap<QString, NamespaceDomBrowserItem*> m_namespaces;
    QMap<ClassDom, ClassDomBrowserItem*> m_classes;
    QMap<TypeAliasDom, TypeAliasDomBrowserItem*> m_typeAliases;
    QMap<FunctionDom, FunctionDomBrowserItem*> m_functions;
    QMap<VariableDom, VariableDomBrowserItem*> m_variables;
};

class ClassDomBrowserItem: public ClassViewItem
{
public:
    ClassDomBrowserItem( Q3ListView* parent, ClassDom dom )
    	: ClassViewItem( parent, dom->name() ), m_dom( dom ) {}
    ClassDomBrowserItem( Q3ListViewItem* parent, ClassDom dom )
    	: ClassViewItem( parent, dom->name() ), m_dom( dom ) {}

    const CodeModelItem* model() const { return m_dom; }
    virtual bool isClass() const { return true; }

    virtual bool hasDeclaration() const { return true; }
    virtual void openDeclaration();

    void setup();
    QString key( int, bool ) const;

    void processClass( ClassDom klass, bool remove=false );
    void processTypeAlias( TypeAliasDom typeAlias, bool remove=false );
    void processFunction( FunctionDom fun, bool remove=false );
    void processVariable( VariableDom var, bool remove=false );

    ClassDom dom() { return m_dom; }

private:
    ClassDom m_dom;
    QMap<ClassDom, ClassDomBrowserItem*> m_classes;
    QMap<TypeAliasDom, TypeAliasDomBrowserItem*> m_typeAliases;
    QMap<FunctionDom, FunctionDomBrowserItem*> m_functions;
    QMap<VariableDom, VariableDomBrowserItem*> m_variables;
};

class TypeAliasDomBrowserItem: public ClassViewItem
{
public:
    TypeAliasDomBrowserItem( Q3ListView* parent, TypeAliasDom dom )
    	: ClassViewItem( parent, dom->name() ), m_dom( dom ) {}
    TypeAliasDomBrowserItem( Q3ListViewItem* parent, TypeAliasDom dom )
    	: ClassViewItem( parent, dom->name() ), m_dom( dom ) {}

    const CodeModelItem* model() const { return m_dom; }
	virtual bool isTypeAlias() const { return true; }

    virtual bool hasDeclaration() const { return true; }
    virtual void openDeclaration();

    void setup();
    QString key( int, bool ) const;

    TypeAliasDom dom() { return m_dom; }

private:
    TypeAliasDom m_dom;
};

class FunctionDomBrowserItem: public ClassViewItem
{
public:
    FunctionDomBrowserItem( Q3ListView* parent, FunctionDom dom )
    	: ClassViewItem( parent, dom->name() ), m_dom( dom ) {}
    FunctionDomBrowserItem( Q3ListViewItem* parent, FunctionDom dom )
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
    VariableDomBrowserItem( Q3ListView* parent, VariableDom dom )
    	: ClassViewItem( parent, dom->name() ), m_dom( dom ) {}
    VariableDomBrowserItem( Q3ListViewItem* parent, VariableDom dom )
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

struct FindOp
{
   FindOp( const FunctionDom& dom ): m_dom( dom ) {}

   bool operator() ( const FunctionDefinitionDom& def ) const
   {
       if( m_dom->name() != def->name() )
           return false;

       if( m_dom->isConstant() != m_dom->isConstant() )
           return false;

       QString scope1 = QString("::") + m_dom->scope().join("::");
       QString scope2 = QString("::") + def->scope().join("::");
       if( !scope1.endsWith(scope2) )
           return false;

       const ArgumentList args = m_dom->argumentList();
       const ArgumentList args2 = def->argumentList();
       if( args.size() != args2.size() )
           return false;

       for( uint i=0; i<args.size(); ++i ){
           if( args[i]->type() != args[i]->type() )
	       return false;
       }

       return true;
   }

private:
   const FunctionDom& m_dom;
};

#endif
