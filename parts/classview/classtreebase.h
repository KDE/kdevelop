/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CLASSTREEBASE_H_
#define _CLASSTREEBASE_H_

#include <klistview.h>
#include "parseditem.h"
#include "parsedscopecontainer.h"
#include "parsedclass.h"
#include "parsedmethod.h"
#include "parsedattribute.h"
#include "classviewpart.h"
#include "parsedscript.h"
//Added by qt3to4:
#include <Q3ValueList>

class ClassTreeItem;
class KPopupMenu;


class ClassTreeBase : public KListView
{
    Q_OBJECT
    
public: 
    ClassTreeBase( ClassViewPart *part, QWidget *parent=0, const char *name=0 );
    ~ClassTreeBase();

protected:
    typedef Q3ValueList<QStringList> TreeState;
    typedef Q3ValueList<QStringList>::Iterator TreeStateIterator;
    TreeState treeState() const;
    void setTreeState(TreeState state);
    
    ClassTreeItem *contextItem;
    virtual KPopupMenu *createPopup() = 0;
    
private slots:
    void slotItemExecuted(Q3ListViewItem*);
    void slotItemPressed(int button, Q3ListViewItem *item);
    void slotContextMenuRequested(Q3ListViewItem *item, const QPoint &p);
    void slotGotoDeclaration();
    void slotGotoImplementation();
    void slotAddMethod();
    void slotAddAttribute();
    void slotClassBaseClasses();
    void slotClassDerivedClasses();
    void slotClassTool();
    
protected:
    ClassViewPart *m_part;
    friend class ClassTreeItem;
    friend class ClassTreeScopeItem;
};


class ClassTreeItem : public Q3ListViewItem, public NotifyClient
{
public:
    ClassTreeItem( ClassTreeBase *parent, ClassTreeItem *lastSibling, ParsedItem *parsedItem )
        : Q3ListViewItem(parent, lastSibling), NotifyClient(), m_item(parsedItem)
    {
        init();
    }
    ClassTreeItem( ClassTreeItem *parent, ClassTreeItem *lastSibling, ParsedItem *parsedItem )
        : Q3ListViewItem(parent, lastSibling), NotifyClient(), m_item(parsedItem)
    {
        init();
    }
    ClassTreeItem( const ClassTreeItem& other )
        : Q3ListViewItem( other.parent(), other.nextSibling()), NotifyClient()
    {
        m_item = other.m_item;
        init();
    }
    ClassTreeItem& operator=( const ClassTreeItem& other )
    {
        m_item = other.m_item;
        init();
        return *this;
    }
    ~ClassTreeItem()
    {
        if ( m_item )
            m_item->unregisterNotifyClient( (NotifyClient*)this );
    }

    // m_item has been deleted.
    void notify() { m_item = 0; }

    KPopupMenu *createPopup();
    bool isOrganizer() { return !m_item; }
    void init() 
    { 
        if ( m_item )
            m_item->registerNotifyClient( (NotifyClient*)this );
    }
    
    void getDeclaration(QString *toFile, int *toLine);
    void getImplementation(QString *toFile, int *toLine);

    virtual QString scopedText() const;
    virtual QString text( int ) const;
    virtual QString tipText() const;
    
protected:
    ClassTreeBase *classTree()
        { return static_cast<ClassTreeBase*>(listView()); }
    ParsedItem *m_item;
};


class ClassTreeOrganizerItem : public ClassTreeItem
{
public:
    ClassTreeOrganizerItem( ClassTreeBase *parent, ClassTreeItem *lastSibling,
                            const QString &text )
        : ClassTreeItem(parent, lastSibling, 0 )
        , m_text( text )
        { init(); }
    ClassTreeOrganizerItem( ClassTreeItem *parent, ClassTreeItem *lastSibling,
                            const QString &text )
        : ClassTreeItem(parent, lastSibling, 0 )
        , m_text( text )
        { init(); }
    ~ClassTreeOrganizerItem()
        {}

    virtual QString text( int ) const { return m_text; }
    
private:
    QString m_text;
    
    void init();
};


class ClassTreeScopeItem : public ClassTreeItem
{
public:
    ClassTreeScopeItem( ClassTreeBase *parent, ClassTreeItem *lastSibling,
                        ParsedScopeContainer *parsedScope )
        : ClassTreeItem(parent, lastSibling, parsedScope)
    { 
      init();
    }
    ClassTreeScopeItem( ClassTreeItem *parent, ClassTreeItem *lastSibling,
                        ParsedScopeContainer *parsedScope )
        : ClassTreeItem(parent, lastSibling, parsedScope)
    { 
      init();
    }
    ~ClassTreeScopeItem()
    {
    }

    virtual QString text( int ) const;
    virtual void setOpen(bool o);

private:
    void init();
};


class ClassTreeClassItem : public ClassTreeItem
{
public:
    ClassTreeClassItem( ClassTreeBase *parent, ClassTreeItem *lastSibling,
                        ParsedClass *parsedClass, bool isStruct=false )
        : ClassTreeItem(parent, lastSibling, parsedClass), m_isStruct( isStruct )
        {
          init();
        }
    ClassTreeClassItem( ClassTreeItem *parent, ClassTreeItem *lastSibling,
                        ParsedClass *parsedClass, bool isStruct=false )
        : ClassTreeItem(parent, lastSibling, parsedClass), m_isStruct( isStruct )
        {
          init();
        }
    ~ClassTreeClassItem()
        {
        }

    virtual void setOpen(bool o);

private:
    void init();

private:
    bool m_isStruct;
};


class ClassTreeMethodItem : public ClassTreeItem
{
public:
    ClassTreeMethodItem( ClassTreeItem *parent, ClassTreeItem *lastSibling,
                         ParsedMethod *parsedMethod );
    ~ClassTreeMethodItem()
        {
        }

    virtual QString text( int ) const;
};


class ClassTreeAttrItem : public ClassTreeItem
{
public:
    ClassTreeAttrItem( ClassTreeItem *parent, ClassTreeItem *lastSibling,
                       ParsedAttribute *parsedAttr );
    ~ClassTreeAttrItem()
        {
        }

    virtual QString text( int ) const;
};

class ClassTreeScriptItem : public ClassTreeItem
{
public:
    ClassTreeScriptItem( ClassTreeItem *parent, ClassTreeItem *lastSibling,
                       ParsedScript *parsedScript );
    ~ClassTreeScriptItem()
        {
        }

    virtual QString text( int ) const;
    virtual void setOpen(bool o);
};


#endif
