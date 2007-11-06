/*
 * KDevelop Problem Reporter
 *
 * Copyright (c) 2006-2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2006 Adam Treat <treat@kde.org>
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

#include "problemwidget.h"

#include <QHeaderView>
#include <QMenu>
#include <QCursor>
#include <QContextMenuEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>

#include <klocale.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <icore.h>
#include <idocumentcontroller.h>
#include <idocument.h>

#include "duchain.h"
#include "duchainobserver.h"

#include "problemreporterpart.h"

using namespace KDevelop;

ProblemWidget::ProblemWidget(QWidget* parent, ProblemReporterPart* part)
  : QTreeWidget(parent)
  , m_part(part)
{
  setObjectName("Problem Reporter Tree");
  setWindowTitle(i18n("Problem Reporter"));
  setHeaderLabels(QStringList() << i18n("File") << i18n("Line") << i18n("Column") << i18n("Problem"));
  setRootIsDecorated(false);
  setWhatsThis( i18n( "Problem Reporter" ) );

  connect(this, SIGNAL(itemActivated(QTreeWidgetItem*, int)), SLOT(itemActivated(QTreeWidgetItem*)));
  connect(DUChain::self()->notifier(), SIGNAL(problemEncountered(const KUrl&, const KTextEditor::Range&, const QString&)), SLOT(problemEncountered(const KUrl&, const KTextEditor::Range&, const QString&)));
}

ProblemWidget::~ProblemWidget()
{
}

void ProblemWidget::problemEncountered(const KUrl& url, const KTextEditor::Range& range, const QString& problem)
{
  QTreeWidgetItem* item = new QTreeWidgetItem(this);
  item->setText(0, url.prettyUrl());
  item->setText(1, QString::number(range.start().line() + 1));
  item->setText(2, QString::number(range.start().column()));
  item->setText(3, problem);
}

void ProblemWidget::itemActivated(QTreeWidgetItem* item)
{
  KUrl url = item->text(0);
  KTextEditor::Cursor cursor(item->text(1).toInt() - 1, item->text(2).toInt());

  m_part->core()->documentController()->openDocument(url, cursor);
}

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on

#include "problemwidget.moc"
