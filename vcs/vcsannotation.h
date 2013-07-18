/* This file is part of KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_VCSANNOTATION_H
#define KDEVPLATFORM_VCSANNOTATION_H

#include "vcsexport.h"
#include <QtCore/QVariant>

class QString;
class QDateTime;
class KUrl;

namespace KDevelop
{
class VcsRevision;

/**
 * Annotation information for a line of a version controlled file
 */
class KDEVPLATFORMVCS_EXPORT VcsAnnotationLine
{
public:
    VcsAnnotationLine();
    VcsAnnotationLine( const VcsAnnotationLine& );
    virtual ~VcsAnnotationLine();
    /**
     * @return the line number of this annotation line
     */
    int lineNumber() const;
    /**
     * @return the text of this line
     */
    QString text() const;
    /**
     * @return the author that last changed this line
     */
    QString author() const;
    /**
     * @return the revision this line was last changed
     */
    VcsRevision revision() const;
    /**
     * @return the date of the last change to this line
     */
    QDateTime date() const;
    /**
     * @return the commit message of the revision in this line
     */
    QString commitMessage() const;
    /**
     * set the line number of this annotation line
     * @param lineno the line number
     */
    void setLineNumber( int lineno );
    /**
     * set the text of this annotation line
     * @param text the text of the line
     */
    void setText( const QString& );
    /**
     * set the author of this annotation line
     * @param author the author of the last change
     */
    void setAuthor( const QString& );
    /**
     * set the revision of this annotation line
     * @param revision the revision of the last change
     */
    void setRevision( const VcsRevision& );
    /**
     * set the date of this annotation line
     * @param date the date of the last change
     */
    void setDate( const QDateTime& );
    /**
     * set the commit message of the revision in this
     * line
     * @param msg the message of the commit
     */
    void setCommitMessage( const QString& msg );
    
    VcsAnnotationLine& operator=( const VcsAnnotationLine& rhs);
private:
    class VcsAnnotationLinePrivate* d;
};

/**
 * Annotations for a local file.
 *
 * This class lets the user fetch information for each line of a local file,
 * including date of last change, author of last change and revision of
 * last change to the line.
 */
class KDEVPLATFORMVCS_EXPORT VcsAnnotation
{
public:
    VcsAnnotation();
    VcsAnnotation(const VcsAnnotation&);
    virtual ~VcsAnnotation();
    /**
     * @return the local url of the file
     */
    KUrl location() const;
    /**
     * @return the number of lines in the file
     */
    int lineCount() const;

    /**
     * retrieve the annotation line for the given number
     */
    VcsAnnotationLine line( int linenumber ) const;

    /**
     * insert a new line to list of lines using
     * the parameters
     *
     * @param lineno the line for which to insert the content
     * @param line the annotation line that should be inserted
     *
     */
    void insertLine( int lineno, const VcsAnnotationLine& );

    /**
     * @param location the location of the file
     */
    void setLocation( const KUrl& );

    bool containsLine( int lineno ) const;

    VcsAnnotation& operator=( const VcsAnnotation& rhs);
private:
    class VcsAnnotationPrivate* const d;
};

}

Q_DECLARE_METATYPE( KDevelop::VcsAnnotation )
Q_DECLARE_METATYPE( KDevelop::VcsAnnotationLine )

#endif

