/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "macronavigationcontext.h"

#include <QtGui/QTextDocument>

#include <klocale.h>

#include <language/duchain/duchain.h>

#include "../../parser/rpp/macrorepository.h"
#include "../../parser/rpp/chartools.h"

namespace Cpp {
using namespace KDevelop;
using namespace rpp;

MacroNavigationContext::MacroNavigationContext(const pp_macro& macro, QString preprocessedBody)
  : AbstractNavigationContext(TopDUContextPointer(0)),
    m_macro(new rpp::pp_macro(macro)), m_body(preprocessedBody)
{}

MacroNavigationContext::~MacroNavigationContext()
{
  delete m_macro;
}

QString MacroNavigationContext::name() const
{
  return m_macro->name.str();
}

QString MacroNavigationContext::html(bool shorten)
{
  clear();
  modifyHtml() += "<html><body><p>" + fontSizePrefix(shorten);
  addExternalHtml(m_prefix);

  QString args;

  if(m_macro->formalsSize()) {
    args = "(";

    bool first = true;
    FOREACH_CUSTOM(const IndexedString& b, m_macro->formals(), m_macro->formalsSize()) {
      if(!first)
        args += ", ";
      first = false;
      args += b.str();
    }

    args += ')';
  }

  modifyHtml() += (m_macro->function_like ? i18n("Function macro") : i18n("Macro")) + " " + importantHighlight(m_macro->name.str()) + " " + args +  "<br />";

  const KUrl url = m_macro->file.toUrl();
  NavigationAction action(url, KTextEditor::Cursor(m_macro->sourceLine,0));
  QList<TopDUContext*> duchains = DUChain::self()->chainsForDocument(m_macro->file);

  if(!shorten) {
    modifyHtml() += "<br />";

    if(!m_body.isEmpty()) {
    modifyHtml() += labelHighlight(i18n("Preprocessed body:")) + "<br />";
    modifyHtml() += codeHighlight(Qt::escape(m_body));
    modifyHtml() += "<br />";
    }


    modifyHtml() += labelHighlight(i18n("Body:")) + "<br />";

    modifyHtml() += codeHighlight(Qt::escape(QString::fromUtf8(stringFromContents((uint*)m_macro->definition(), m_macro->definitionSize()))));
    modifyHtml() += "<br />";
  }

  const QString path = url.pathOrUrl();
  makeLink(path, path, action);

  modifyHtml() += fontSizeSuffix(shorten) + "</p></body></html>";
  return currentHtml();
}

}
