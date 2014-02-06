/*
    This file is part of KDevelop

    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    Copyright 2013 Milian Wolff <mail@milianw.de>

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

#include "clanglanguagesupport.h"

#include "clangparsejob.h"
#include "version.h"

#include "duchain/clangtypes.h"

#include "codecompletion/model.h"

#include "documentfinderhelpers.h"

#include <KPluginFactory>
#include <KAboutData>

#include <language/codecompletion/codecompletion.h>
#include <language/highlighting/codehighlighting.h>

K_PLUGIN_FACTORY(KDevClangSupportFactory, registerPlugin<ClangLanguageSupport>(); )
K_EXPORT_PLUGIN(KDevClangSupportFactory(
    KAboutData("kdevclanglanguagesupport", 0, ki18n("Clang Language Plugin"), "0.1",
    ki18n("Support for C, C++ and Objective-C languages."), KAboutData::License_GPL)))

using namespace KDevelop;

ClangLanguageSupport::ClangLanguageSupport(QObject* parent, const QVariantList& )
: IPlugin( KDevClangSupportFactory::componentData(), parent )
, ILanguageSupport()
, m_highlighting(new KDevelop::CodeHighlighting(this))
, m_index(new ClangIndex)
{
    KDEV_USE_EXTENSION_INTERFACE(ILanguageSupport)

    new KDevelop::CodeCompletion( this, new ClangCodeCompletionModel(this), name() );
    for(const auto& type : DocumentFinderHelpers::mimeTypesList()){
        KDevelop::IBuddyDocumentFinder::addFinder(type, this);
    }
}

ClangLanguageSupport::~ClangLanguageSupport()
{
    for(const auto& type : DocumentFinderHelpers::mimeTypesList()) {
        KDevelop::IBuddyDocumentFinder::removeFinder(type);
    }
}

ParseJob* ClangLanguageSupport::createParseJob(const IndexedString& url)
{
    return new ClangParseJob(url, this);
}

QString ClangLanguageSupport::name() const
{
    return "clang";
}

ICodeHighlighting* ClangLanguageSupport::codeHighlighting() const
{
    return m_highlighting;
}

ClangIndex* ClangLanguageSupport::index()
{
    return m_index.data();
}

bool ClangLanguageSupport::areBuddies(const KUrl& url1, const KUrl& url2)
{
    return DocumentFinderHelpers::areBuddies(url1, url2);
}

bool ClangLanguageSupport::buddyOrder(const KUrl& url1, const KUrl& url2)
{
    return DocumentFinderHelpers::buddyOrder(url1, url2);
}

QVector< KUrl > ClangLanguageSupport::getPotentialBuddies(const KUrl& url) const
{
    return DocumentFinderHelpers::getPotentialBuddies(url);
}

#include "clanglanguagesupport.moc"
