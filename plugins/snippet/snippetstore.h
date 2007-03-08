/***************************************************************************
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __SNIPPETSTORE_H__
#define __SNIPPETSTORE_H__

#include <QObject>
#include <QString>
#include <QList>
#include <QStandardItemModel>

class SnippetRepository;

/**
 * This class is implemented as singelton.
 * It holds the toplevel repositories and acts as a model for the snippet tree.
 * All toplevel repositories are stored in the @a repos_ QList.
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class SnippetStore : public QStandardItemModel
{
    Q_OBJECT

public:
    /**
     * Retuns the SnippetStore.
     * It will be created if it does not yet exist.
     */
    static SnippetStore* instance();

    virtual ~SnippetStore();

    /**
     * Creates a new repository and adds it to the model.
     * If @a parent is NULL, the new repository will become a toplevel item and will therefor
     * be stored in the @a repos_ QList.
     * Other repos become a children of the passed @a parent.
     */
    void createNewRepository(SnippetRepository* parent, QString name, QString dir);

    /**
     * 
     */
    void remove(SnippetRepository* repo);

private:
    SnippetStore();
    virtual Qt::ItemFlags flags (const QModelIndex & index) const;

    static SnippetStore* self_;
    QList<SnippetRepository*> repos_;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; indent-mode cstyle; replace-tabs on; auto-insert-doxygen on;
