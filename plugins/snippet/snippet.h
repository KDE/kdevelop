/***************************************************************************
 *   Copyright 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __SNIPPET_H__
#define __SNIPPET_H__

#include <QObject>
#include <QIcon>

#include <QStringList>
#include <QStandardItem>

#include "snippetvariablemodel.h"

class SnippetRepository;

/**
 * One object of this class represents one snippet.
 * Each snippet gets read from a file which needs to be passed to the ctor.
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class Snippet : public QObject, public QStandardItem
{
    Q_OBJECT

public:
    /**
     * Construct a snippet. The object will issue slotSyncSnippet() right after
     * it has been set up.
     * @param filename The file from which to load the snippet. Must be relative
     *                 to the location of the repository. The filename will also
     *                 act as the snippet's name
     * @param repo The SnippetRepository which should hold this Snippet.
     *             The new Snippet appends itself to this SnippetRepository
     */
    Snippet(const QString& filename, SnippetRepository* repo);
    virtual ~Snippet();

    /**
     * @return A list of keywords that are associated to this snippet
     */
    QStringList& getKeywordList() { return keywords_; }

    /**
     * @attention This method only returns the snippet's plain text; If you would
     *            like to get variables substituted which real values call
     *            interpretSnippet() instead.
     * @return The snippet's plain text
     */
    const QString getSnippetPlainText() { return snippetText_; }

    /**
     * Returns the snippet's text. If the snippet conatins variables, a dialog will
     * be opened in which the user can input the substituion values.
     * @return Returns the substituted snippet text or an empty QString if the user
     *         canceled the substitution dialog
     */
    const QString interpretSnippet();

    /**
     * Call to change the snippet's text
     */
    void setSnippetText(QString text) { snippetText_ = text; setToolTip( text); }

    /**
     * @returns the filename including the path where this snippet is stored
     */
    const QString getFileName();

    /**
     * Store the values from this Snippet-object in it's file.
     */
    void save();

    /**
     * Changing the snippet's name also includes changing the filename
     * Calling this function will do so.
     * @note If a file before the name change does not yet exist, the filename
     *       will be changed and the snippet gets saved to the new file
     */
    void changeName(const QString& newName);

    /**
     * Removes the file from the disk
     */
    void removeSnippetFile();

private slots:
    /**
     * When this slot gets called, the snippet regets all data from it's file
     */
    void slotSyncSnippet();

private:
    /**
     * Parses all information out of @p data and stores it in this object's
     * private members.
     */
    void setRawData(const QString& data);
    int initVariableModel(SnippetVariableModel& model);
    void handleMetaData(QStringList& metadata);

    SnippetRepository* repo_;
    QString name_;
    QString snippetText_;
    QStringList keywords_;

};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; indent-mode cstyle; replace-tabs on; auto-insert-doxygen on;
