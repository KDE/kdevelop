/*
* KDevelop xUnit integration
* Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#include "documentaccess.h"
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <ktexteditor/document.h>

using QTest::DocumentAccess;
using namespace KDevelop;

namespace
{
KTextEditor::Document* textDocumentForUrl(const KUrl& url)
{
    ICore* c = ICore::self();
    IDocumentController* dc = c->documentController();
    IDocument* doc = dc->documentForUrl(url);
    if (!doc || !doc->isTextDocument() ) return 0;
    KTextEditor::Document* tdoc = doc->textDocument();
    return tdoc;
}
}

DocumentAccess::DocumentAccess(QObject* parent)
  : QObject(parent)
{}

DocumentAccess::~DocumentAccess()
{}

QString DocumentAccess::text(const KUrl& url, const KDevelop::SimpleRange& range)
{
    KTextEditor::Document* tdoc = textDocumentForUrl(url);
    if (!tdoc) return QString();
    return tdoc->text(range.textRange());
}

QString DocumentAccess::text(const KUrl& url)
{
    KTextEditor::Document* tdoc = textDocumentForUrl(url);
    if (!tdoc) return QString();
    return tdoc->text();
}

#include "documentaccess.moc"

