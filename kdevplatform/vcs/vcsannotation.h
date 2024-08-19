/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_VCSANNOTATION_H
#define KDEVPLATFORM_VCSANNOTATION_H

#include "vcsexport.h"
#include <QMetaType>
#include <QSharedDataPointer>

class QString;
class QDateTime;
class QUrl;

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
    void setText( const QString& text );
    /**
     * set the author of this annotation line
     * @param author the author of the last change
     */
    void setAuthor( const QString& author );
    /**
     * set the revision of this annotation line
     * @param revision the revision of the last change
     */
    void setRevision( const VcsRevision& revision );
    /**
     * set the date of this annotation line
     * @param date the date of the last change
     */
    void setDate( const QDateTime& date );
    /**
     * set the commit message of the revision in this
     * line
     * @param msg the message of the commit
     */
    void setCommitMessage( const QString& msg );
    
    VcsAnnotationLine& operator=( const VcsAnnotationLine& rhs);

private:
    QSharedDataPointer<class VcsAnnotationLinePrivate> d;
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
    QUrl location() const;
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
    void insertLine( int lineno, const VcsAnnotationLine& line );

    /**
     * Clear the line list
     */
    void clearLines();

    /**
     * @param location the location of the file
     */
    void setLocation( const QUrl& location );

    bool containsLine( int lineno ) const;

    VcsAnnotation& operator=( const VcsAnnotation& rhs);

private:
    QSharedDataPointer<class VcsAnnotationPrivate> d;
};

}

Q_DECLARE_METATYPE( KDevelop::VcsAnnotation )
Q_DECLARE_TYPEINFO( KDevelop::VcsAnnotation, Q_MOVABLE_TYPE );
Q_DECLARE_METATYPE( KDevelop::VcsAnnotationLine )
Q_DECLARE_TYPEINFO( KDevelop::VcsAnnotationLine, Q_MOVABLE_TYPE);

#endif

