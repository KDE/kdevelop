/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#ifndef KDEVCPPLANGUAGESUPPORT_H
#define KDEVCPPLANGUAGESUPPORT_H

#include <kdevlanguagesupport.h>

// class CodeModel;
// class CodeProxy;
// class CodeDelegate;
// class CodeAggregate;
// class BackgroundParser;

class JavaLanguageSupport : public KDevLanguageSupport
{
Q_OBJECT
public:
    CppLanguageSupport( QObject* parent, const QStringList& args = QStringList() );
    virtual ~CppLanguageSupport();

    //KDevLanguageSupport implementation
//     virtual KDevCodeModel *codeModel( const KUrl& url ) const;
//     virtual KDevCodeProxy *codeProxy() const;
//     virtual KDevCodeDelegate *codeDelegate() const;
//     virtual KDevCodeRepository *codeRepository() const;
    virtual QStringList mimeTypes() const;

private slots:
    void documentLoaded( KDevDocument* file );
    void documentClosed( KDevDocument* file );
    void documentActivated( KDevDocument* file );
    void projectOpened();
    void projectClosed();

private:
    QStringList m_mimetypes;
//     CodeProxy *m_codeProxy;
//     CodeDelegate *m_codeDelegate;
//     BackgroundParser *m_backgroundParser;
//     CppHighlighting *m_highlights;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
