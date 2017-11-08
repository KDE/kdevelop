/* This file is part of KDevelop
 *
 * Copyright 2017-2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_VCSANNITATIONITEMDELEGATE_H
#define KDEVPLATFORM_VCSANNITATIONITEMDELEGATE_H

// KDev
#include <vcsrevision.h>
// KF
#include <KTextEditor/AbstractAnnotationItemDelegate>
// Qt
#include <QHash>
#include <QBrush>

namespace KDevelop
{
class VcsAnnotationLine;

class VcsAnnotationItemDelegate : public KTextEditor::AbstractAnnotationItemDelegate
{
    Q_OBJECT

public:
    VcsAnnotationItemDelegate(KTextEditor::View* view, KTextEditor::AnnotationModel* model, QObject* parent);
    ~VcsAnnotationItemDelegate() override;

public: // AbstractAnnotationItemDelegate APO
    void paint(QPainter* painter, const KTextEditor::StyleOptionAnnotationItem& option,
               KTextEditor::AnnotationModel* model, int line) const override;
    QSize sizeHint(const KTextEditor::StyleOptionAnnotationItem& option,
                   KTextEditor::AnnotationModel* model, int line) const override;
    bool helpEvent(QHelpEvent* event, KTextEditor::View* view,
                   const KTextEditor::StyleOptionAnnotationItem& option,
                   KTextEditor::AnnotationModel *model, int line) override;
    void hideTooltip(KTextEditor::View *view) override;

private:
    void renderBackground(QPainter* painter,
                          const KTextEditor::StyleOptionAnnotationItem& option,
                          const VcsAnnotationLine& annotationLine) const;
    void renderMessageAndAge(QPainter* painter,
                             const KTextEditor::StyleOptionAnnotationItem& option,
                             const QRect& messageRect, const QString& messageText,
                             const QRect& ageRect, const QString& ageText) const;
    void renderAuthor(QPainter* painter,
                      const KTextEditor::StyleOptionAnnotationItem& option,
                      const QRect& authorRect, const QString& authorText) const;
    void renderHighlight(QPainter* painter,
                         const KTextEditor::StyleOptionAnnotationItem& option) const;
    void doMessageLineLayout(const KTextEditor::StyleOptionAnnotationItem& option,
                             QRect* messageRect, QRect* ageRect) const;
    void doAuthorLineLayout(const KTextEditor::StyleOptionAnnotationItem& option,
                            QRect* authorRect) const;

protected: // QObject API
    bool eventFilter(QObject* object, QEvent* event) override;

private Q_SLOTS:
    void resetBackgrounds();

private:
    int widthHintFromViewWidth(int viewWidth) const;

private:
    KTextEditor::AnnotationModel* const m_model;

    // TODO: make this configurable
    const int m_maxWidthViewPercent = 25;

    mutable QHash<VcsRevision, QBrush> m_backgrounds;

    mutable int m_lastCharBasedWidthHint = 0;
    mutable int m_lastViewBasedWidthHint = 0;
};

}

#endif
