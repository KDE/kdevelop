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

#include "vcsexport.h"
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
    unsigned int lineCount() const;
    /**
     * @param linenum the number of the line, counting from 0
     * @return the content of the specified line
     */
    QString line( unsigned int linenum ) const;
    /**
     * @param linenum the number of the line, counting from 0
     * @return the revision of the last change on the specified line
     */
    VcsRevision revision( unsigned int linenum ) const;
    /**
     * @param linenum the number of the line, counting from 0
     * @return the author of the last change on the specified line
     */
    QString author( unsigned int linenum ) const;
    /**
     * @param linenum the number of the line, counting from 0
     * @return the date of the last change on the specified line
     */
    QDateTime date( unsigned int linenum ) const;

    void setLines( const QStringList& );
    void setAuthors( const QStringList& );
    void setDates( const QList<QDateTime>& );
    void setRevisions( const QList<VcsRevision>& );
    void setLocation( const KUrl& );

    VcsAnnotation& operator=( const VcsAnnotation& rhs);
private:
    class VcsAnnotationPrivate* const d;
};

}

Q_DECLARE_METATYPE( KDevelop::VcsAnnotation )

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
