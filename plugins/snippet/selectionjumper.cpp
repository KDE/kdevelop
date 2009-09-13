/**
 * This file is part of KDevelop
 *
 * Copyright 2009 Milian Wolff <mail@milianw.de>
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

#include "selectionjumper.h"

#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/SmartInterface>
#include <KTextEditor/SmartRangeNotifier>
#include <KColorScheme>

#include <QtGui/QKeyEvent>
#include <QApplication>

using namespace KTextEditor;

SelectionJumper::SelectionJumper( QList< Range > ranges, Document* document )
    : m_topRange(0), m_currentRange(0), m_doc(document), m_rangeAttribute(new Attribute)
{
    Q_ASSERT(!ranges.isEmpty());
    Q_ASSERT(document);

    KColorScheme scheme(QApplication::palette().currentColorGroup());

    m_rangeAttribute->setBackground(scheme.background(KColorScheme::NeutralBackground));

    if ( SmartInterface * smartInterface =
        qobject_cast<SmartInterface*>( m_doc ) ) {
        m_topRange = smartInterface->newSmartRange();
        foreach ( const Range &range, ranges ) {
            SmartRange* smartRange = smartInterface->newSmartRange(range, m_topRange);
            smartRange->setAttribute(m_rangeAttribute);
        }
        setCurrentRange(m_topRange->childRanges().first());
        smartInterface->addHighlightToDocument(m_topRange);
    }

    // install event filter
    foreach ( View* view, m_doc->views() ) {
        setupViewEventFilter( view );
    }

    connect(m_doc, SIGNAL(aboutToClose(KTextEditor::Document*)),
            this, SLOT(deleteLater()));
    connect(m_doc, SIGNAL(viewCreated(KTextEditor::Document*,KTextEditor::View*)),
            this, SLOT(slotViewCreated(KTextEditor::Document*,KTextEditor::View*)));
}

SelectionJumper::~SelectionJumper()
{
}

void SelectionJumper::deleteSelectionJumper()
{
    SmartInterface *smartInterface = qobject_cast<SmartInterface*>( m_doc );
    if ( m_topRange && smartInterface ) {
        smartInterface->removeHighlightFromDocument(m_topRange);
        m_topRange->deleteChildRanges();
        SmartCursor* start = &m_topRange->smartStart();
        SmartCursor* end = &m_topRange->smartEnd();
        smartInterface->unbindSmartRange( m_topRange );
        delete start;
        delete end;
        m_topRange = 0;
    }
    foreach ( View* view, m_doc->views() ) {
        view->removeEventFilter(this);
    }

    this->deleteLater();
}

void SelectionJumper::setupViewEventFilter( View* view )
{
    view->focusProxy()->installEventFilter(this);
}

void SelectionJumper::slotViewCreated( Document* document, View* view )
{
    Q_ASSERT(document == m_doc);
    setupViewEventFilter(view);
}

bool SelectionJumper::eventFilter( QObject* object, QEvent* event )
{
    if ( event->type() == QEvent::ShortcutOverride ) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if ( keyEvent->key() == Qt::Key_Escape ) {
            deleteSelectionJumper();
            return true;
        } else if ( keyEvent->modifiers() & Qt::AltModifier &&
                        ( keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right ) ) {
            SmartRange* nextRange;
            if ( keyEvent->key() == Qt::Key_Left ) {
                nextRange = m_topRange->childBefore(m_currentRange);
                if ( !nextRange ) {
                    nextRange = m_topRange->childRanges().last();
                }
            } else {
                nextRange = m_topRange->childAfter(m_currentRange);
                if ( !nextRange ) {
                    nextRange = m_topRange->childRanges().first();
                }
            }
            setCurrentRange(nextRange);
            return true;
        }
    }
    return QObject::eventFilter(object, event);
}

void SelectionJumper::setCurrentRange( SmartRange* currentRange )
{
    m_currentRange = currentRange;
    if ( m_doc->activeView() ) {
        m_doc->activeView()->setSelection(*m_currentRange);
        m_doc->activeView()->setCursorPosition(m_currentRange->start());
    }
}

#include "selectionjumper.moc"
