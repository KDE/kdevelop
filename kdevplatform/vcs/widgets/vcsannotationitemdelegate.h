/*
    SPDX-FileCopyrightText: 2017-2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_VCSANNOTATIONITEMDELEGATE_H
#define KDEVPLATFORM_VCSANNOTATIONITEMDELEGATE_H

// KDev
#include <vcsrevision.h>
// KF
#include <KTextEditor/AbstractAnnotationItemDelegate>
// Qt
#include <QHash>
#include <QBrush>

#include <memory>

namespace KDevelop
{
class VcsAnnotationLine;

class VcsAnnotationItemDelegate : public KTextEditor::AbstractAnnotationItemDelegate
{
    Q_OBJECT

public:
    using AnnotationModelPtr = std::shared_ptr<KTextEditor::AnnotationModel>;

    /**
     * Constructor
     *
     * @param view the view for which this delegate is set, also used as the parent of this delegate object.
     *
     * @warning This delegate relies on @p view staying its parent, so do not change it.
     */
    explicit VcsAnnotationItemDelegate(KTextEditor::View* view);
    ~VcsAnnotationItemDelegate() override;

    /**
     * Enable the delegate and associate an annotation model with it.
     *
     * @param model a non-null annotation model that is used by this delegate's KTextEditor::View
     */
    void enable(AnnotationModelPtr model);

    /**
     * Disable the delegate and prevent it from using the associated annotation model.
     *
     * This delegate's shared pointer to the model is reset,
     * so the model is destroyed unless used/shared by another delegate.
     */
    void disable();

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
    AnnotationModelPtr m_model;

    // TODO: make this configurable
    const int m_maxWidthViewPercent = 25;

    mutable QHash<VcsRevision, QBrush> m_backgrounds;

    mutable int m_lastCharBasedWidthHint = 0;
    mutable int m_lastViewBasedWidthHint = 0;
};

}

#endif
