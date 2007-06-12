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

#ifndef VCSDIFF_H
#define VCSDIFF_H

#include <vcsexport.h>
#include <QtCore/QVariant>

class QString;
class QByteArray;

namespace KDevelop
{

class KDEVPLATFORMVCS_EXPORT VcsDiff
{
public:
    /**
     * Specify the type of difference the diff() method should create. Note that a
     * request for DiffUnified may not be honored, e.g. if the items being diffed are
     * binary rather than text.
     */
    enum Type
    {
        DiffRaw         /**<Request complete copies of both items.*/,
        DiffUnified     /**<Request copy of first item with diff.*/,
        DiffDontCare    /**<Don't care; plugin will return whichever is easiest.*/
    };

    enum Content
    {
        Binary          /** Binary diff, using the full content of both files.*/,
        Text            /** Textual diff.*/
    };

    VcsDiff();
    virtual ~VcsDiff();
    VcsDiff( const VcsDiff& );

    /**
     * @returns the type of diff, i.e. raw or unified
     */
    Type type() const;

    /**
     * @returns the content type, i.e. binary or text
     */
    Content contentType() const;

    /**
     * @returns the binary content of the first file of the difference or
     * an empty QByteArray if this is a textual diff
     */
    QByteArray firstBinary() const;

    /**
     * @returns the binary content of the second file of the difference or
     * an empty QByteArray if this is a textual diff
     */
    QByteArray secondBinary() const;

    /**
     * @returns the textual content of the first file of the difference or
     * an empty QString if this is a binary diff
     */
    QString firstText() const;

    /**
     * @returns the textual content of the second file of the difference or
     * an empty QString if this is a unified or binary diff
     */
    QString secondText() const;

    /**
     * @returns the difference between the first and the second file in
     * unified diff format or an empty QString if this is a binary diff
     * or a textual diff using raw format
     */
    QString diff() const;

    void setDiff( const QString& );
    void setFirstBinary( const QByteArray& );
    void setSecondBinary( const QByteArray& );
    void setFirstText( const QString& );
    void setSecondText( const QString& );
    void setType( Type );
    void setContentType( Content );
    VcsDiff& operator=( const VcsDiff& rhs);
private:
    class VcsDiffPrivate* const d;
};

}

Q_DECLARE_METATYPE( KDevelop::VcsDiff )

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
