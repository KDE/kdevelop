/*
 * KDevelop Fake Language Support
 *
 * Copyright (c) 2006 Adam Treat <treat@kde.org>
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

#include "fakesupport_part.h"
#include "fakesupport_factory.h"

FakeLanguageSupport::FakeLanguageSupport( QObject *parent, const char *name,
        const QStringList & )
        : KDevLanguageSupport( FakeSupportFactory::info(), parent )
{
    setObjectName( QString::fromUtf8( name ) );
}

FakeLanguageSupport::~FakeLanguageSupport()
{}

KDevCodeModel *FakeLanguageSupport::codeModel( const KUrl &url ) const
{
    Q_UNUSED( url );
    return 0;
}

KDevCodeProxy *FakeLanguageSupport::codeProxy() const
{
    return 0;
}

KDevCodeDelegate *FakeLanguageSupport::codeDelegate() const
{
    return 0;
}

KDevCodeRepository *FakeLanguageSupport::codeRepository() const
{
    return 0;
}

KDevParseJob *FakeLanguageSupport::createParseJob( const KUrl &url )
{
    return 0;
}

KDevParseJob *FakeLanguageSupport::createParseJob( KDevDocument *document,
        KTextEditor::SmartRange *highlight )
{
    return 0;
}

QStringList FakeLanguageSupport::mimeTypes() const
{
    return QStringList();
}

#include "fakesupport_part.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
