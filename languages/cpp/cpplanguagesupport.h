/*
 * KDevelop C++ Language Support
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVCPPLANGUAGESUPPORT_H
#define KDEVCPPLANGUAGESUPPORT_H

#include <kdevlanguagesupport.h>


class CppLanguageSupport : public KDevLanguageSupport
{
Q_OBJECT
public:
    CppLanguageSupport( QObject* parent, const char* name = 0,
                        const QStringList& args = QStringList() );
    virtual ~CppLanguageSupport();

    //KDevLanguageSupport implementation
    virtual int features() const;
    virtual KMimeType::List mimeTypes() const;

    virtual QString formatTag( const Tag& /*tag*/ ) const { return QString(); }
    virtual QString formatModelItem( const KDevCodeItem* /*item*/, bool ) const { return QString(); }
    virtual QString formatClassName( const QString& name ) const { return name; }
    virtual QString unformatClassName( const QString& name ) const { return name; }

private:
    int m_features;
    KMimeType::List m_mimetypes;
};

#endif
