/* This file is part of KDevelop
 *  Copyright 2008 Hamish Rodda <rodda@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

 */

#include "valgrindtree.h"

#include <QApplication>

#include <KIO/NetAccess>

#include <icore.h>
#include <idocumentcontroller.h>

#include "valgrindmodel.h"

ValgrindTree::ValgrindTree()
{
}

void ValgrindTree::expanded(const QModelIndex & index)
{
    if (index.isValid() && !index.parent().isValid()) {
        QModelIndex child = index.child(0,0);
        if (child.isValid())
            expand(child);
    }
}

void ValgrindTree::activated(const QModelIndex & index)
{
    if (ValgrindFrame* frame = dynamic_cast<ValgrindFrame*>(static_cast<ValgrindModel*>(model())->itemForIndex(index))) {
        KUrl doc = frame->url();
        if (doc.isValid() && KIO::NetAccess::exists(doc, KIO::NetAccess::SourceSide, qApp->activeWindow())) {
            KDevelop::ICore::self()->documentController()->openDocument(doc, KTextEditor::Cursor(frame->line, 0));
        }
    }
}

#include "valgrindtree.moc"
