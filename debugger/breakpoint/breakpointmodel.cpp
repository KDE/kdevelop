/* This file is part of the KDE project
   Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2006, 2008 Vladimir Prus <ghost@cs.msu.su>
   Copyright (C) 2007 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2009 Niko Sams <niko.sams@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/>.
*/

#include "breakpointmodel.h"

#include <QPixmap>
#include <KIcon>
#include <KParts/PartManager>
#include <KDebug>
#include <KLocale>
#include <KTextEditor/Document>
#include <KTextEditor/SmartInterface>

#include "../interfaces/icore.h"
#include "../interfaces/idocumentcontroller.h"
#include "../interfaces/idocument.h"
#include "breakpoint.h"
#include "breakpoints.h"
#include <KTextEditor/SmartCursorNotifier>


using namespace KDevelop;
using namespace KTextEditor;

BreakpointModel::BreakpointModel(QObject* parent)
    : TreeModel(QVector<QString>() << "" << "" << "Type" << "Location" << "Condition", parent),
      universe_(new Breakpoints(this)), m_dontUpdateMarks(false)
{
    setRootItem(universe_);
    universe_->load();
    universe_->createHelperBreakpoint();

    connect(this, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            universe_, SLOT(save()));
    connect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            universe_, SLOT(save()));
    connect(this, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
            universe_, SLOT(save()));
    connect(this, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(updateMarks()));

    //new ModelTest(this, this);

    if (KDevelop::ICore::self()->partController()) { //TODO remove if
        foreach(KParts::Part* p, KDevelop::ICore::self()->partController()->parts())
            slotPartAdded(p);
        connect(KDevelop::ICore::self()->partController(),
                SIGNAL(partAdded(KParts::Part*)),
                this,
                SLOT(slotPartAdded(KParts::Part*)));
    }


    connect (KDevelop::ICore::self()->documentController(),
             SIGNAL(textDocumentCreated(KDevelop::IDocument*)),
             this,
             SLOT(textDocumentCreated(KDevelop::IDocument*)));
    connect (KDevelop::ICore::self()->documentController(),
                SIGNAL(documentSaved(KDevelop::IDocument*)),
                SLOT(documentSaved(KDevelop::IDocument*)));
}

void BreakpointModel::slotPartAdded(KParts::Part* part)
{
    if (KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>(part))
    {
        MarkInterface *iface = dynamic_cast<MarkInterface*>(doc);
        if( !iface )
            return;
        
        iface->setMarkDescription((MarkInterface::MarkTypes)BreakpointMark, i18n("Breakpoint"));
        iface->setMarkPixmap((MarkInterface::MarkTypes)BreakpointMark, *inactiveBreakpointPixmap());
        iface->setMarkPixmap((MarkInterface::MarkTypes)ActiveBreakpointMark, *activeBreakpointPixmap());
        iface->setMarkPixmap((MarkInterface::MarkTypes)ReachedBreakpointMark, *reachedBreakpointPixmap());
        iface->setMarkPixmap((MarkInterface::MarkTypes)DisabledBreakpointMark, *disabledBreakpointPixmap());
        iface->setMarkPixmap((MarkInterface::MarkTypes)ExecutionPointMark, *executionPointPixmap());
        iface->setEditableMarks( BookmarkMark | BreakpointMark );
        
        updateMarks();
    }
}

void BreakpointModel::textDocumentCreated(KDevelop::IDocument* doc)
{
    KTextEditor::MarkInterface *iface =
        qobject_cast<KTextEditor::MarkInterface*>(doc->textDocument());

    if( iface ) {
        connect (doc->textDocument(), SIGNAL(
                     markChanged(KTextEditor::Document*,
                                 KTextEditor::Mark,
                                 KTextEditor::MarkInterface::MarkChangeAction)),
                 this,
                 SLOT(markChanged(KTextEditor::Document*,
                                 KTextEditor::Mark,
                                  KTextEditor::MarkInterface::MarkChangeAction)));
    }
}

QVariant 
BreakpointModel::headerData(int section, Qt::Orientation orientation,
                                 int role) const
{ 
    if (orientation == Qt::Horizontal && role == Qt::DecorationRole
        && section == 0)
        return KIcon("dialog-ok-apply");
    else if (orientation == Qt::Horizontal && role == Qt::DecorationRole
             && section == 1)
        return KIcon("system-switch-user");

    return TreeModel::headerData(section, orientation, role);
}

Qt::ItemFlags BreakpointModel::flags(const QModelIndex &index) const
{
    /* FIXME: all this logic must be in item */
    if (!index.isValid())
        return 0;

    if (index.column() == 0)
        return static_cast<Qt::ItemFlags>(
            Qt::ItemIsEnabled | Qt::ItemIsSelectable 
            | Qt::ItemIsEditable | Qt::ItemIsUserCheckable);

    if (index.column() == Breakpoint::LocationColumn
        || index.column() == Breakpoint::ConditionColumn)
        return static_cast<Qt::ItemFlags>(
            Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);

    return static_cast<Qt::ItemFlags>(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

void BreakpointModel::markChanged(
    KTextEditor::Document *document, 
    KTextEditor::Mark mark, 
    KTextEditor::MarkInterface::MarkChangeAction action)
{
    int type = mark.type;
    /* Is this a breakpoint mark, to begin with? */
    if (type != (MarkInterface::MarkTypes)BreakpointMark
        && type != (MarkInterface::MarkTypes)ActiveBreakpointMark
        && type != (MarkInterface::MarkTypes)ReachedBreakpointMark
        && type != (MarkInterface::MarkTypes)DisabledBreakpointMark)
        return;

    if (action == KTextEditor::MarkInterface::MarkAdded) {
        bool alreadyExists = false;
        for(int i=0; i<universe_->breakpointCount(); ++i) {
            Breakpoint *b = universe_->breakpoint(i);
            if (b->url() == document->url() && b->line() == mark.line && !b->deleted()) {
                alreadyExists = true;
                break;
            }
        }
        if (!alreadyExists) {
            Breakpoint *breakpoint = universe_->addCodeBreakpoint(document->url(), mark.line);
            KTextEditor::SmartInterface *smart = qobject_cast<KTextEditor::SmartInterface*>(document);
            if (smart) {
                KTextEditor::SmartCursor* cursor = smart->newSmartCursor(KTextEditor::Cursor(mark.line, 0));
                connect(cursor->notifier(), SIGNAL(deleted(KTextEditor::SmartCursor*)),
                            SLOT(cursorDeleted(KTextEditor::SmartCursor*)));
                breakpoint->setSmartCursor(cursor);
            }
            
        }
    } else {
        // Find this breakpoint and delete it
        for(int i=0; i<universe_->breakpointCount(); ++i) {
            Breakpoint *b = universe_->breakpoint(i);
            if (b->url() == document->url() && b->line() == mark.line && !b->deleted()) {
                b->setDeleted();
            }
        }
    }
#if 0
    if ( KDevelop::ICore::self()->documentController()->activeDocument() && KDevelop::ICore::self()->documentController()->activeDocument()->textDocument() == document )
    {
        //bring focus back to the editor
        // TODO probably want a different command here
        KDevelop::ICore::self()->documentController()->activateDocument(KDevelop::ICore::self()->documentController()->activeDocument());
    }
#endif
}

const QPixmap* BreakpointModel::inactiveBreakpointPixmap()
{
  static QPixmap pixmap=KIcon("script-error").pixmap(QSize(22,22), QIcon::Normal, QIcon::Off);
  return &pixmap;
}

const QPixmap* BreakpointModel::activeBreakpointPixmap()
{
  static QPixmap pixmap=KIcon("script-error").pixmap(QSize(22,22), QIcon::Active, QIcon::Off);
  return &pixmap;
}

const QPixmap* BreakpointModel::reachedBreakpointPixmap()
{
  static QPixmap pixmap=KIcon("script-error").pixmap(QSize(22,22), QIcon::Selected, QIcon::Off);
  return &pixmap;
}

const QPixmap* BreakpointModel::disabledBreakpointPixmap()
{
  static QPixmap pixmap=KIcon("script-error").pixmap(QSize(22,22), QIcon::Disabled, QIcon::Off);
  return &pixmap;
}

const QPixmap* BreakpointModel::executionPointPixmap()
{
  static QPixmap pixmap=KIcon("go-next").pixmap(QSize(22,22), QIcon::Normal, QIcon::Off);
  return &pixmap;
}

Breakpoints* BreakpointModel::breakpointsItem() { return universe_; }

void BreakpointModel::toggleBreakpoint(const KUrl& url, const KTextEditor::Cursor& cursor)
{
    for(int i=0; i<universe_->breakpointCount(); ++i) {
        Breakpoint *b = universe_->breakpoint(i);
        if (b->url() == url && b->line() == cursor.line() && !b->deleted()) {
            b->setDeleted();
            return;
        }
    }
    universe_->addCodeBreakpoint(url, cursor.line());
}

void KDevelop::BreakpointModel::_breakpointDeleted(KDevelop::Breakpoint* breakpoint)
{
    kDebug() << breakpoint;
    updateMarks();
    emit breakpointDeleted(breakpoint);
}


void KDevelop::BreakpointModel::updateMarks()
{
    if (m_dontUpdateMarks) return;

    QMap<KUrl, QSet<int> > breakpoints;
    for (int i=0; i<breakpointsItem()->breakpointCount(); ++i) {
        Breakpoint *breakpoint = breakpointsItem()->breakpoint(i);
        if (breakpoint->pleaseEnterLocation()) continue;
        if (breakpoint->deleted()) continue;
        if (breakpoint->kind() != Breakpoint::CodeBreakpoint) continue;
        breakpoints[breakpoint->url()] << breakpoint->line();
    }
    kDebug() << breakpoints;
    foreach (IDocument *doc, ICore::self()->documentController()->openDocuments()) {
        KTextEditor::MarkInterface *mark = qobject_cast<KTextEditor::MarkInterface*>(doc->textDocument());
        if (!mark) continue;
        foreach (KTextEditor::Mark *m, mark->marks()) {
            kDebug() << m->line << m->type;
            if (m->type & BreakpointMark) {
                if (!breakpoints.contains(doc->url()) || !breakpoints[doc->url()].contains(m->line)) {
                    kDebug() << "removeMark";
                    mark->removeMark(m->line, BreakpointMark);
                } else {
                    breakpoints[doc->url()].remove(m->line);
                }
            }
        }
    }
    kDebug() << breakpoints;
    
    QMapIterator<KUrl, QSet<int> > i(breakpoints);
    while (i.hasNext()) {
        i.next();
        foreach (int line, i.value()) {
            IDocument *doc = ICore::self()->documentController()->documentForUrl(i.key());
            if (!doc) continue;
            KTextEditor::MarkInterface *mark = qobject_cast<KTextEditor::MarkInterface*>(doc->textDocument());
            if (!mark) continue;
            mark->addMark(line, BreakpointMark);
            kDebug() << "addMark" << line;
            KTextEditor::SmartInterface *smart = qobject_cast<KTextEditor::SmartInterface*>(doc->textDocument());
            if (!smart) continue;
            for (int j=0; j<breakpointsItem()->breakpointCount(); ++j) {
                Breakpoint *breakpoint = breakpointsItem()->breakpoint(j);
                if (breakpoint->pleaseEnterLocation()) continue;
                if (breakpoint->deleted()) continue;
                if (breakpoint->kind() != Breakpoint::CodeBreakpoint) continue;
                if (i.key() == breakpoint->url() && line == breakpoint->line()) {
                    KTextEditor::SmartCursor* cursor = smart->newSmartCursor(KTextEditor::Cursor(line, 0));
                    connect(cursor->notifier(), SIGNAL(deleted(KTextEditor::SmartCursor*)),
                                SLOT(cursorDeleted(KTextEditor::SmartCursor*)));
                    breakpoint->setSmartCursor(cursor);
                }
            }
        }
    }    
}

void BreakpointModel::documentSaved(KDevelop::IDocument* doc)
{
    kDebug();
    for (int i=0; i<breakpointsItem()->breakpointCount(); ++i) {
        Breakpoint *breakpoint = breakpointsItem()->breakpoint(i);
        if (breakpoint->smartCursor()) {
            if (breakpoint->smartCursor()->document() != doc->textDocument()) continue;
            if (breakpoint->smartCursor()->line() == breakpoint->line()) continue;
            m_dontUpdateMarks = true;
            breakpoint->setLine(breakpoint->smartCursor()->line());
            m_dontUpdateMarks = false;
        }
    }
}
void BreakpointModel::cursorDeleted(KTextEditor::SmartCursor* cursor)
{
    for (int i=0; i<breakpointsItem()->breakpointCount(); ++i) {
        Breakpoint *breakpoint = breakpointsItem()->breakpoint(i);
        if (breakpoint->smartCursor() == cursor) {
            breakpoint->setSmartCursor(0);
        }
    }
}

#include "breakpointmodel.moc"
