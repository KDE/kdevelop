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

#ifndef SELECTIONJUMPER_H
#define SELECTIONJUMPER_H

#include <QtCore/QObject>

#include <KTextEditor/Attribute>

namespace KTextEditor {
class View;
class Document;
class Range;
class SmartRange;
}

/**
 * This class creates - if possible - smart ranges
 * for a given set of ranges that resemble "interesting" positions.
 * 
 * The first range will get selected. Tab & Shift-Tab allows you to
 * quickly navigate to the next/previous range. Escape will delete all ranges.
 */
class SelectionJumper : public QObject
{
    Q_OBJECT

public:
    SelectionJumper(QList<KTextEditor::Range> ranges, KTextEditor::Document* document);
    ~SelectionJumper();

public slots:
    void deleteSelectionJumper();
    void slotViewCreated(KTextEditor::Document* document, KTextEditor::View* view);

protected:
    virtual bool eventFilter(QObject* object, QEvent* event);

private:
    void setupViewEventFilter(KTextEditor::View* view);
    void setCurrentRange(KTextEditor::SmartRange* currentRange);

    KTextEditor::SmartRange* m_topRange;
    KTextEditor::SmartRange* m_currentRange;
    KTextEditor::Document* m_doc;
    KTextEditor::Attribute::Ptr m_rangeAttribute;
};

#endif // SELECTIONJUMPER_H
