/*************************************************************************************
 *  Copyright (C) 2013 by Milian Wolff <mail@milianw.de>                             *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "clanglanguagesupport.h"

#include "clangparsejob.h"
#include "clanghighlighting.h"
#include "version.h"

#include <KPluginFactory>
#include <KAboutData>

#include <language/codecompletion/codecompletion.h>

K_PLUGIN_FACTORY(KDevClangSupportFactory, registerPlugin<ClangLanguageSupport>(); )
K_EXPORT_PLUGIN(KDevClangSupportFactory(
    KAboutData("kdevclanglanguagesupport", 0, ki18n("Clang Language Plugin"), "0.1",
    ki18n("Support for C, C++ and Objective-C languages."), KAboutData::License_GPL)))

using namespace KDevelop;

ClangLanguageSupport::ClangLanguageSupport(QObject* parent, const QVariantList& )
: IPlugin( KDevClangSupportFactory::componentData(), parent )
, ILanguageSupport()
, m_highlighting(new ClangHighlighting(this))
//FIXME:
, m_fileManager(0)
{
    KDEV_USE_EXTENSION_INTERFACE(ILanguageSupport)
}

ClangLanguageSupport::~ClangLanguageSupport()
{

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

#include "clanglanguagesupport.moc"
