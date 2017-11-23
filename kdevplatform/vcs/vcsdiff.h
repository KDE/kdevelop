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

#ifndef KDEVPLATFORM_VCSDIFF_H
#define KDEVPLATFORM_VCSDIFF_H

#include <QSharedDataPointer>
#include <QtGlobal>
#include <QMetaType>

#include "vcsexport.h"

class QUrl;
class QString;

namespace KDevelop
{

class KDEVPLATFORMVCS_EXPORT VcsDiff
{
public:
    VcsDiff();
    virtual ~VcsDiff();
    VcsDiff( const VcsDiff& );

    /**
     * @returns the difference between the first and the second file in
     * unified diff format or an empty QString if this is a binary diff
     * or a textual diff using raw format
     */
    QString diff() const;
    
    /** @returns the base directory of the diff. */
    QUrl baseDiff() const;

	/**
	 * Depth - number of directories to left-strip from paths in the patch - see "patch -p"
     * Defaults to 0
	 */
    uint depth() const;

    /** Sets the base directory of the diff to the @p url */
    void setBaseDiff(const QUrl& url);
    /** Sets the depth of the diff to @p depth */
    void setDepth(const uint depth);

    void setDiff( const QString& );

    VcsDiff& operator=( const VcsDiff& rhs);
    
    /** @returns whether there are not changes on the diff */
    bool isEmpty() const;
    
private:
    QSharedDataPointer<class VcsDiffPrivate> d;
};

}

Q_DECLARE_METATYPE( KDevelop::VcsDiff )
Q_DECLARE_TYPEINFO( KDevelop::VcsDiff, Q_MOVABLE_TYPE );

#endif

