/***************************************************************************
 *   Copyright 2008 David Nolden  <david.nolden.kdevelop@art-master.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "ilanguagesupport.h"
#include "../duchain/duchain.h"
#include "../editor/simplerange.h"

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <backgroundparser/documentchangetracker.h>

namespace KDevelop {

TopDUContext* ILanguageSupport::standardContext(const KUrl& url, bool proxyContext) {
  Q_UNUSED(proxyContext)
  return DUChain::self()->chainForDocument(url);
}

SimpleRange ILanguageSupport::specialLanguageObjectRange(const KUrl& url, const SimpleCursor& position) {
    Q_UNUSED(url)
    Q_UNUSED(position)
    return SimpleRange::invalid();
}

QPair<KUrl, SimpleCursor> ILanguageSupport::specialLanguageObjectJumpCursor(const KUrl& url, const SimpleCursor& position) {
    Q_UNUSED(url)
    Q_UNUSED(position)
    return QPair<KUrl, SimpleCursor>(KUrl(), SimpleCursor::invalid());
}

QWidget* ILanguageSupport::specialLanguageObjectNavigationWidget(const KUrl& url, const SimpleCursor& position) {
    Q_UNUSED(url)
    Q_UNUSED(position)
    return 0;
}

ICodeHighlighting* ILanguageSupport::codeHighlighting() const {
    return 0;
}

ICreateClassHelper* ILanguageSupport::createClassHelper() const {
    return 0;
}


ILanguage* ILanguageSupport::language() {
    return ICore::self()->languageController()->language(name());
}

DocumentChangeTracker* ILanguageSupport::createChangeTrackerForDocument ( KTextEditor::Document* document ) const
{
    return new DocumentChangeTracker(document);
}

ILanguageSupport::WhitespaceSensitivity ILanguageSupport::whitespaceSensititivy() const
{
    return ILanguageSupport::Insensitive;
}

SourceFormatterItemList ILanguageSupport::sourceFormatterItems() const
{
    return SourceFormatterItemList();
}

QString ILanguageSupport::indentationSample() const
{
    return "";
}

}
