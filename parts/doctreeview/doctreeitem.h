/***************************************************************************
 *   Copyright (C) 1999-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2002 by Sebastian Kratzert                              *
 *   skratzert@gmx.de                                                      *
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DOCTREEITEM_H
#define DOCTREEITEM_H

#include <qlistview.h>

class KListView;

class DocTreeItem : public QListViewItem
{
public:
    enum Type { Folder, Book, Doc };
    DocTreeItem( KListView *parent, Type type, const QString &text, const QString &context, bool lazy = false );
    DocTreeItem( DocTreeItem *parent, Type type, const QString &text, const QString &context, bool lazy = false );
    virtual ~DocTreeItem();
    
    inline void postInit() { if (!m_lazy) refresh(); }
    
    virtual QString fileName();
    void setFileName(const QString &fn);
    
    const QString& indexFileName() const;
        
    virtual void clear();
    virtual QString context() const;
    virtual Type type() const;
    
    bool isCurrent() const;
    void setDirty();
    virtual void setOpen(bool open);

protected:
    void setIndexFileName(const QString& fileName);
    
    virtual void refresh();
    
private:
    void init();
    
    Type m_typ;
    QString m_fileName, m_indexFileName, m_context;
    bool m_current : 1, m_lazy : 1;
};

#endif
