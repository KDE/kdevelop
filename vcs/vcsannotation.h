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

#ifndef VCSANNOTATION_H
#define VCSANNOTATION_H

#include <vcsexport.h>
#include <QtCore/QVariant>

class QString;
class QDateTime;
class KUrl;

namespace KDevelop
{
class VcsRevision;

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
     * @param linenum the number of the line, counting from 0
     * @return the content of the specified line
     */
    QString line( int linenum ) const;
    /**
     * @param linenum the number of the line, counting from 0
     * @return the revision of the last change on the specified line
     */
    VcsRevision revision( int linenum ) const;
    /**
     * @param linenum the number of the line, counting from 0
     * @return the author of the last change on the specified line
     */
    QString author( int linenum ) const;
    /**
     * @param linenum the number of the line, counting from 0
     * @return the date of the last change on the specified line
     */
    QDateTime date( int linenum ) const;

    /**
     * add a new line to the end of the list of lines using
     * the parameters
     *
     * @param text the text of the line
     * @param author the author of the last change to the line
     * @param date the date of the last change to the line
     * @param revision the revision of the last change to the line
     *
     */
    void addLine( const QString& text, const QString& author, 
                  const QDateTime& date, const VcsRevision& revision );
    void setLocation( const KUrl& );

    VcsAnnotation& operator=( const VcsAnnotation& rhs);
private:
    class VcsAnnotationPrivate* const d;
};

}

Q_DECLARE_METATYPE( KDevelop::VcsAnnotation )

#endif

