/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000 by Bernd Gehrmann                                  *
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
#include "parsedstruct.h"
#include "parsedmethod.h"
#include "parsedattribute.h"
#include "classview.h"

class ClassTreeItem;
class KPopupMenu;


class ClassTreeBase : public KListView
{
    Q_OBJECT
    
public: 
    ClassTreeBase( ClassView *part, QWidget *parent=0, const char *name=0 );
    ~ClassTreeBase();

public slots:
    virtual void setClassStore(ClassStore *store);
    virtual void setLanguageSupport(KDevLanguageSupport *ls);

protected:
    ClassStore *m_store;
    ClassView *m_part;
    ClassTreeItem *contextItem;
    virtual KPopupMenu *createPopup() = 0;
    
private slots:
    void slotItemPressed(int button, QListViewItem *item);
    void slotRightButtonPressed(QListViewItem *item, const QPoint &p);
    void slotGotoDeclaration();
    void slotGotoImplementation();
    void slotAddMethod();
    void slotAddAttribute();
    void slotClassBaseClasses();
    void slotClassDerivedClasses();
    void slotClassTool();
    
private:
    KDevLanguageSupport *m_langsupport;
    friend class ClassTreeItem;
};


class ClassTreeItem : public QListViewItem
{
public:
    ClassTreeItem( ClassTreeBase *parent, ClassTreeItem *lastSibling,
                   ParsedItem *parsedItem, const QString &text=QString::null )
        : QListViewItem(parent, lastSibling, text), m_item(parsedItem)
        { init(text); }
    ClassTreeItem( ClassTreeItem *parent, ClassTreeItem *lastSibling,
                   ParsedItem *parsedItem, const QString &text=QString::null )
        : QListViewItem(parent, lastSibling, text), m_item(parsedItem)
        { init(text); }
    ~ClassTreeItem()
        {}

    KPopupMenu *createPopup();
    QString scopedText() const;
    void getDeclaration(QString *toFile, int *toLine);
    void getImplementation(QString *toFile, int *toLine);

protected:
    ParsedItem *m_item;

private:
    void init(const QString &text);
    ClassTreeBase *classTree()
        { return static_cast<ClassTreeBase*>(listView()); }
};


class ClassTreeOrganizerItem : public ClassTreeItem
{
public:
    ClassTreeOrganizerItem( ClassTreeBase *parent, ClassTreeItem *lastSibling,
                            const QString &text )
        : ClassTreeItem(parent, lastSibling, 0, text)
        { init(); }
    ClassTreeOrganizerItem( ClassTreeItem *parent, ClassTreeItem *lastSibling,
                            const QString &text )
        : ClassTreeItem(parent, lastSibling, 0, text)
        { init(); }
    ~ClassTreeOrganizerItem()
        {}

private:
    void init();
};


class ClassTreeScopeItem : public ClassTreeItem
{
public:
    ClassTreeScopeItem( ClassTreeBase *parent, ClassTreeItem *lastSibling,
                        ParsedScopeContainer *parsedScope )
        : ClassTreeItem(parent, lastSibling, parsedScope)
        { init(); }
    ClassTreeScopeItem( ClassTreeItem *parent, ClassTreeItem *lastSibling,
                        ParsedScopeContainer *parsedScope )
        : ClassTreeItem(parent, lastSibling, parsedScope)
        { init(); }
    ~ClassTreeScopeItem()
        {}

protected:
    virtual void setOpen(bool o);

private:
    void init();
};


class ClassTreeClassItem : public ClassTreeItem
{
public:
    ClassTreeClassItem( ClassTreeBase *parent, ClassTreeItem *lastSibling,
                        ParsedClass *parsedClass )
        : ClassTreeItem(parent, lastSibling, parsedClass)
        { init(); }
    ClassTreeClassItem( ClassTreeItem *parent, ClassTreeItem *lastSibling,
                        ParsedClass *parsedClass )
        : ClassTreeItem(parent, lastSibling, parsedClass)
        { init(); }
    ~ClassTreeClassItem()
        {}

protected:
    virtual void setOpen(bool o);

private:
    void init();
};


class ClassTreeStructItem : public ClassTreeItem
{
public:
    ClassTreeStructItem( ClassTreeItem *parent, ClassTreeItem *lastSibling,
                         ParsedStruct *parsedStruct );
    ~ClassTreeStructItem()
        {}

protected:
    virtual void setOpen(bool o);
};


class ClassTreeMethodItem : public ClassTreeItem
{
public:
    ClassTreeMethodItem( ClassTreeItem *parent, ClassTreeItem *lastSibling,
                         ParsedMethod *parsedMethod );
    ~ClassTreeMethodItem()
        {}
};


class ClassTreeAttrItem : public ClassTreeItem
{
public:
    ClassTreeAttrItem( ClassTreeItem *parent, ClassTreeItem *lastSibling,
                       ParsedAttribute *parsedAttr );
    ~ClassTreeAttrItem()
        {}
};

#endif
