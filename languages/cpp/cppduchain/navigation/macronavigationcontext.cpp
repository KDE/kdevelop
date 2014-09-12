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

#include <QTextDocument>

#include <KLocalizedString>

#include <KTextEditor/Editor>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/ConfigInterface>

#include <language/duchain/duchain.h>
#include <interfaces/icore.h>
#include <interfaces/ipartcontroller.h>

#include <QVBoxLayout>
#include <QLabel>

#include "../../parser/rpp/macrorepository.h"
#include "../../parser/rpp/chartools.h"


namespace Cpp {
using namespace KDevelop;
using namespace rpp;

KTextEditor::View* createDocAndView(const QString& data, KTextEditor::Document** docPtr)
{
  if (data.isEmpty()) {
    return 0;
  }
  KTextEditor::Document* doc = ICore::self()->partController()->editorPart()->createDocument(0);
  *docPtr = doc;
  doc->setText(data);
  doc->setMode("C++");
  doc->setReadWrite(false);
  KTextEditor::View* view = doc->createView(0);
  view->setStatusBarEnabled(false);
  if (KTextEditor::ConfigInterface* config = qobject_cast<KTextEditor::ConfigInterface*>(view)) {
    config->setConfigValue("icon-bar", false);
    config->setConfigValue("folding-bar", false);
    config->setConfigValue("line-numbers", false);
    config->setConfigValue("dynamic-word-wrap", true);
  }
  return view;
}

MacroNavigationContext::MacroNavigationContext(const pp_macro& macro, QString preprocessedBody)
: AbstractNavigationContext(TopDUContextPointer(0))
, m_macro(new rpp::pp_macro(macro))
, m_body(preprocessedBody)
, m_preprocessed(0)
, m_definition(0)
, m_widget(0)
{
  KTextEditor::View* preprocessedView = createDocAndView(preprocessedBody.trimmed(), &m_preprocessed);
  QString definition = QString::fromUtf8(stringFromContents((uint*)m_macro->definition(), m_macro->definitionSize()).trimmed());
  KTextEditor::View* definitionView = createDocAndView(definition, &m_definition);

  m_widget = new QWidget;
  QVBoxLayout* layout = new QVBoxLayout(m_widget);
  if (m_preprocessed) {
    layout->addWidget(new QLabel(i18n("Preprocessed Body:")));
    layout->addWidget(preprocessedView);
  } else {
    layout->addWidget(new QLabel(i18n("Preprocessed Body: (empty)")));
  }
  if (m_definition) {
    layout->addWidget(new QLabel(i18n("Body:")));
    layout->addWidget(definitionView);
  } else {
    layout->addWidget(new QLabel(i18n("Body: (empty)")));
  }
  m_widget->setLayout(layout);
}

MacroNavigationContext::~MacroNavigationContext()
{
  delete m_preprocessed;
  delete m_definition;
  delete m_widget;
  delete m_macro;
}

QString MacroNavigationContext::name() const
{
  return m_macro->name.str();
}

QWidget* MacroNavigationContext::widget() const
{
  return m_widget;
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

  const KUrl url = m_macro->file.toUrl();
  const QString path = url.pathOrUrl();
  KTextEditor::Cursor cursor(m_macro->sourceLine, 0);
  NavigationAction action(url, cursor);
  modifyHtml() += i18nc("%1: macro type, i.e.: 'Function macro' or just 'Macro'"
                        "%2: the macro name and arguments"
                        "%3 the link to the definition",
                        "%1: %2, defined in %3",
                        (m_macro->function_like ? i18n("Function macro") : i18n("Macro")),
                        importantHighlight(m_macro->name.str()) + args,
                        createLink(QString("%1 :%2").arg(path).arg(cursor.line()+1), path, action));

  modifyHtml() += fontSizeSuffix(shorten) + "</p></body></html>";
  return currentHtml();
}

QString MacroNavigationContext::body() const
{
  return m_body;
}

}
