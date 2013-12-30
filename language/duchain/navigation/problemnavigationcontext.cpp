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

#include <klocalizedstring.h>
#include <language/interfaces/iproblem.h>
#include <qtextdocument.h>
#include <qboxlayout.h>
#include <interfaces/iassistant.h>
#include <qmenu.h>
#include <kaction.h>
#include <util/richtextpushbutton.h>

using namespace KDevelop;

ProblemNavigationContext::ProblemNavigationContext(ProblemPointer problem): m_problem(problem)
{
  m_widget = 0;

  KSharedPtr< IAssistant > solution = problem->solutionAssistant();
  if(solution && !solution->actions().isEmpty()) {
    m_widget = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout(m_widget);
    RichTextPushButton* button = new RichTextPushButton;
//     button->setPopupMode(QToolButton::InstantPopup);
    if(!solution->title().isEmpty())
      button->setHtml(i18n("Solve: %1", solution->title()));
    else
      button->setHtml(i18n("Solve"));

    QMenu* menu = new QMenu;
    menu->setFocusPolicy(Qt::NoFocus);
    foreach(IAssistantAction::Ptr action, solution->actions()) {
      menu->addAction(action->toKAction());
      kDebug() << "adding action" << action->description();
    }
    button->setMenu(menu);

    layout->addWidget(button);
    layout->setAlignment(button, Qt::AlignLeft);
    m_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
  }
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


QString ProblemNavigationContext::html(bool shorten)
{
  clear();
  m_shorten = shorten;
  modifyHtml() += "<html><body><p><small><small>";

  modifyHtml() += i18n("Problem in <b>%1</b>:<br/>", m_problem->sourceString());
  modifyHtml() += Qt::escape(m_problem->description());
  modifyHtml() += "<br/>";
  modifyHtml() += "<i style=\"white-space:pre-wrap\">" + Qt::escape(m_problem->explanation()) + "</i>";

  modifyHtml() += "</small></small></p></body></html>";

  return currentHtml();
}


