/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "problemnavigationcontext.h"

#include <debug.h>

#include <KIconLoader>
#include <KLocalizedString>

#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/problem.h>

#include <language/duchain/duchain.h>
#include <language/editor/documentrange.h>

using namespace KDevelop;

namespace {

QString KEY_INVOKE_ACTION(int num) { return QStringLiteral("invoke_action_%1").arg(num); }

QString iconForSeverity(IProblem::Severity severity)
{
  switch (severity) {
  case IProblem::Hint:
    return QStringLiteral("dialog-information");
  case IProblem::Warning:
    return QStringLiteral("dialog-warning");
  case IProblem::Error:
    return QStringLiteral("dialog-error");
  case IProblem::NoSeverity:
    return {};
  }
  Q_UNREACHABLE();
  return {};
}

QString htmlImg(const QString& iconName, KIconLoader::Group group)
{
  auto* loader = KIconLoader::global();
  const int size = loader->currentSize(group);
  return QStringLiteral("<img width='%1' height='%1' src='%2'/>")
    .arg(size)
    .arg(loader->iconPath(iconName, group));
}

}

ProblemNavigationContext::ProblemNavigationContext(const QVector<IProblem::Ptr>& problems, const Flags flags)
  : m_problems(problems)
  , m_flags(flags)
  , m_widget(nullptr)
{
  // Sort problems vector:
  // 1) By severity
  // 2) By sourceString, if severities are equals
  std::sort(m_problems.begin(), m_problems.end(), [](const IProblem::Ptr a, const IProblem::Ptr b) {
    if (a->severity() < b->severity())
      return true;

    if (a->severity() > b->severity())
      return false;

    if (a->sourceString() < b->sourceString())
      return true;

    return false;
  });
}

ProblemNavigationContext::~ProblemNavigationContext()
{
  delete m_widget;
}

QWidget* ProblemNavigationContext::widget() const
{
    return m_widget;
}

bool ProblemNavigationContext::isWidgetMaximized() const
{
    return false;
}

QString ProblemNavigationContext::name() const
{
  return i18n("Problem");
}

QString ProblemNavigationContext::escapedHtml(const QString& text) const
{
  static const QString htmlStart = QStringLiteral("<html>");
  static const QString htmlEnd = QStringLiteral("</html>");

  QString result = text.trimmed();

  if (!result.startsWith(htmlStart))
    return result.toHtmlEscaped();

  result.remove(htmlStart);
  result.remove(htmlEnd);

  return result;
}

void ProblemNavigationContext::html(IProblem::Ptr problem)
{
  auto iconPath = iconForSeverity(problem->severity());


  modifyHtml() += QStringLiteral("<table><tr>");

  modifyHtml() += QStringLiteral("<td valign=\"middle\">%1</td>").arg(htmlImg(iconPath, KIconLoader::Panel));

  // BEGIN: right column
  modifyHtml() += QStringLiteral("<td>");

  modifyHtml() += i18n("Problem in <i>%1</i>", problem->sourceString());
  modifyHtml() += QStringLiteral("<br/>");

  if (m_flags & ShowLocation) {
    modifyHtml() += labelHighlight(i18n("Location: "));
    makeLink(QStringLiteral("%1 :%2")
        .arg(problem->finalLocation().document.toUrl().fileName())
        .arg(problem->finalLocation().start().line() + 1),
      QString(),
      NavigationAction(problem->finalLocation().document.toUrl(), problem->finalLocation().start())
    );

    modifyHtml() += QStringLiteral("<br/>");
  }

  QString description = escapedHtml(problem->description());
  QString explanation = escapedHtml(problem->explanation());

  modifyHtml() += description;

  // Add only non-empty explanation which differs from the problem description.
  // Skip this if we have more than one problem.
  if (m_problems.size() == 1 && !explanation.isEmpty() && explanation != description)
    modifyHtml() += "<p><i style=\"white-space:pre-wrap\">" + explanation + "</i></p>";

  modifyHtml() += QStringLiteral("</td>");
  // END: right column

  modifyHtml() += QStringLiteral("</tr></table>");

  auto diagnostics = problem->diagnostics();
  if (!diagnostics.isEmpty()) {
    DUChainReadLocker lock;
    for (auto diagnostic : diagnostics) {
      modifyHtml() += QStringLiteral("<p>");
      modifyHtml() += labelHighlight(QStringLiteral("%1: ").arg(diagnostic->severityString()));
      modifyHtml() += escapedHtml(diagnostic->description());

      const DocumentRange range = diagnostic->finalLocation();
      Declaration* declaration = DUChainUtils::itemUnderCursor(range.document.toUrl(), range.start()).declaration;
      if (declaration) {
        modifyHtml() += i18n("<br>See: ");
        makeLink(declaration->toString(), DeclarationPointer(declaration), NavigationAction::NavigateDeclaration);
        modifyHtml() += i18n(" in ");
        makeLink(QStringLiteral("%1 :%2")
                  .arg(declaration->url().toUrl().fileName())
                  .arg(declaration->rangeInCurrentRevision().start().line() + 1),
                DeclarationPointer(declaration), NavigationAction::NavigateDeclaration);
      }

      else if (range.start().isValid()) {
        modifyHtml() += i18n("<br>See: ");
        const auto url = range.document.toUrl();
        makeLink(QStringLiteral("%1 :%2")
                  .arg(url.fileName())
                  .arg(range.start().line() + 1),
                url.toDisplayString(QUrl::PreferLocalFile), NavigationAction(url, range.start()));
      }

      modifyHtml() += QStringLiteral("</p>");
    }
  }

  auto assistant = problem->solutionAssistant();
  if (assistant && !assistant->actions().isEmpty()) {
    modifyHtml() += QStringLiteral("<table width='100%' style='border: 1px solid black; background-color: %1;'>").arg(QStringLiteral("#b3d4ff"));
    modifyHtml() += QStringLiteral("<tr><td valign='middle'>%1</td><td width='100%'>").arg(htmlImg(QStringLiteral("dialog-ok-apply"), KIconLoader::Panel));

    const int startIndex = m_assistantsActions.size();
    int currentIndex = startIndex;
    foreach (auto assistantAction, assistant->actions()) {
      m_assistantsActions.append(assistantAction);

      if (currentIndex != startIndex)
        modifyHtml() += QStringLiteral("<br/>");

      makeLink(i18n("Solution (%1)", currentIndex + 1), KEY_INVOKE_ACTION( currentIndex ),
               NavigationAction(KEY_INVOKE_ACTION( currentIndex )));
      modifyHtml() += ": " + assistantAction->description().toHtmlEscaped();

      ++currentIndex;
    }

    modifyHtml() += QStringLiteral("</td></tr>");
    modifyHtml() += QStringLiteral("</table>");
  }
}

QString ProblemNavigationContext::html(bool shorten)
{
  AbstractNavigationContext::html(shorten);

  clear();
  m_assistantsActions.clear();

  int problemIndex = 0;
  foreach (auto problem, m_problems) {
    html(problem);

    if (++problemIndex != m_problems.size())
      modifyHtml() += QStringLiteral("<hr>");
  }

  return currentHtml();
}

NavigationContextPointer ProblemNavigationContext::executeKeyAction(QString key)
{
  if (key.startsWith(QLatin1String("invoke_action_"))) {
    const int index = key.replace(QLatin1String("invoke_action_"), QString()).toInt();
    executeAction(index);
  }

  return {};
}

void ProblemNavigationContext::executeAction(int index)
{
  if (index < 0 || index >= m_assistantsActions.size())
    return;

  auto action = m_assistantsActions.at(index);
  Q_ASSERT(action);

  if (action) {
    action->execute();
    if ( topContext() )
      DUChain::self()->updateContextForUrl(topContext()->url(), TopDUContext::ForceUpdate);
  } else {
    qCWarning(LANGUAGE()) << "No such action";
    return;
  }
}
