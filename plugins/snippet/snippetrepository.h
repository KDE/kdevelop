/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __SNIPPETREPOSITORY_H__
#define __SNIPPETREPOSITORY_H__

#include <QObject>
#include <QPointer>
#include <QList>
#include <QStandardItem>
#include <QDir>

class Snippet;

/**
 * Each object of this type is a repository of snippets. Each repository
 * has a name and a directory from where the snippets get loaded.
 * The Snippets are stored in the @a snippets_ member.
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class SnippetRepository : public QObject, public QStandardItem
{
    Q_OBJECT

public:
    /**
     * Creates a new SnippetRepository.
     * After object creation slotSyncRepository() will be called.
     * @param name A name for the repository
     * @param location The directory from where to load the snippets
     */
    SnippetRepository(const QString& name, const QString& location);
    ~SnippetRepository();

    /**
     * adds the snippet to this repository
     */
    void addSnippet( Snippet* snippet );

    /**
     * @return The snippets that are currently stored in this repository
     */
    const QList<Snippet*>& getSnippets() { return snippets_; }

    /**
     * @return The directory of the repository
     */
    const QString& getLocation() { return location_; }

    /**
     * Calling this method will append @p repo to this repository.
     * This makes @p repo a child of this SnippetRepository.
     * @param repo @p repo will become a child of this SnippetRepository.
     */
    void addSubRepo(SnippetRepository* repo) { appendRow( repo ); subrepos_.append( repo ); }

    /**
     * Change the location of the repository.
     * If @p newName is not empty, the name of the repository will also be changed.
     * @param newLocation The repository will be moved to this location
     * @param newName If set, the name of the repository will be changed
     */
    void changeLocation(const QString& newLocation, const QString& newName = "");

    /**
     * Remove this repository from the disk.
     */
    void removeDirectory();

    /**
     * Creates a new subdirectory named @p subrepo.
     * @note Also calles slotSyncRepository()
     */
    void createSubRepo(const QString& subrepo);

public slots:
    /**
     * When this slot gets called, all stored snippets gets removed from the
     * internal data structure and the directory of the repository get searched
     * for snippets. All found snippets will then be stored in the internal
     * snippets_ list.
     */
    void slotSyncRepository();

private:
    /**
     * @see changeLocation()
     */
    void setLocation(QString loc) { location_ = QDir::cleanPath(loc); setToolTip(location_); }

    QString location_;

    QList<Snippet*> snippets_;
    QList<SnippetRepository*> subrepos_;
};

#endif

