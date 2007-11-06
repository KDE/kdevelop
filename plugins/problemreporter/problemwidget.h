/*
 * KDevelop Problem Reporter
 *
 * Copyright (c) 2007 Hamish Rodda <rodda@kde.org>
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

#ifndef DUCHAINTREE_H
#define DUCHAINTREE_H

#include <QtGui/QTreeWidget>

#include <KUrl>
#include <KTextEditor/Range>

class ProblemReporterPart;

class ProblemWidget : public QTreeWidget
{
  Q_OBJECT

public:
  ProblemWidget(QWidget* parent, ProblemReporterPart* part);
  virtual ~ProblemWidget();

private Q_SLOTS:
  void problemEncountered(const KUrl& url, const KTextEditor::Range& range, const QString& problem);

  void itemActivated(QTreeWidgetItem* item);

private:
  ProblemReporterPart* m_part;
};

#endif

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
