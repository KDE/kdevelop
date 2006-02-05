/*
 * KDevelop C++ Language Support
 *
 * Copyright (c) 2005 Matt Rogers <mattr@kde.org>
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

class CodeModel;
class CodeProxy;
class CodeDelegate;
class CodeAggregate;
class BackgroundParser;

class CppLanguageSupport : public KDevLanguageSupport
{
Q_OBJECT
public:
    CppLanguageSupport( QObject* parent, const char* name = 0,
                        const QStringList& args = QStringList() );
    virtual ~CppLanguageSupport();

    //KDevLanguageSupport implementation
    virtual KDevCodeModel *codeModel( const KUrl &url = KUrl() ) const;
    virtual KDevCodeProxy *codeProxy() const;
    virtual KDevCodeDelegate *codeDelegate() const;
    virtual KDevCodeRepository *codeRepository() const;
    virtual QStringList mimeTypes() const;

private slots:
    void documentLoaded( const KUrl &url );
    void documentClosed( const KUrl &url );
    void documentActivated( const KUrl &url );

private:
    QStringList m_mimetypes;
    CodeProxy *m_codeProxy;
    CodeDelegate *m_codeDelegate;
    BackgroundParser *m_backgroundParser;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
