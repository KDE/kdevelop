/*
    SPDX-FileCopyrightText: 2017-2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "vcsannotationitemdelegate.h"

#include <models/vcsannotationmodel.h>
#include <vcsannotation.h>
#include <debug.h>

#include <KTextEditor/AnnotationInterface>
#include <KTextEditor/View>
#include <KTextEditor/Attribute>
#include <KLocalizedString>

#include <QHelpEvent>
#include <QPainter>
#include <QAction>
#include <QMenu>
#include <QToolTip>
#include <QFontMetricsF>
#include <QDate>
#include <QStyle>
#include <QApplication>

#include <cmath>

using namespace KDevelop;

VcsAnnotationItemDelegate::VcsAnnotationItemDelegate(KTextEditor::View* view, KTextEditor::AnnotationModel* model,
                                                     QObject* parent)
    : KTextEditor::AbstractAnnotationItemDelegate(parent)
    , m_model(model)
{
    // dump background brushes on schema change
    connect(view, &KTextEditor::View::configChanged, this, &VcsAnnotationItemDelegate::resetBackgrounds);

    view->installEventFilter(this);
}

VcsAnnotationItemDelegate::~VcsAnnotationItemDelegate() = default;

static QString ageOfDate(const QDate& date)
{
    const auto now = QDate::currentDate();
    int ageInYears = now.year() - date.year();
    if (now < date.addYears(ageInYears)) {
        --ageInYears;
    }
    if (ageInYears > 0) {
        return i18ncp("@item age", "%1 year", "%1 years", ageInYears);
    }
    int ageInMonths = now.month() - date.month();
    if (now.day() < date.day()) {
        --ageInMonths;
    }
    if (ageInMonths < 0) {
        ageInMonths += 12;
    }
    if (ageInMonths > 0) {
        return i18ncp("@item age", "%1 month", "%1 months", ageInMonths);
    }
    const int ageInDays = date.daysTo(now);
    if (ageInDays > 0) {
        return i18ncp("@item age", "%1 day", "%1 days", ageInDays);
    }
    return i18nc("@item age", "Today");
}

void VcsAnnotationItemDelegate::doMessageLineLayout(const KTextEditor::StyleOptionAnnotationItem& option,
                                                    QRect* messageRect, QRect* ageRect) const
{
    Q_ASSERT(messageRect && messageRect->isValid());
    Q_ASSERT(ageRect);

    const QWidget* const widget = option.view;
    QStyle* const style = widget ? widget->style() : QApplication::style();
    const bool hasAge = ageRect->isValid();
    // "+ 1" as used in QItemDelegate
    const int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, widget) + 1;
    const int ageMargin = hasAge ? textMargin : 0;

    const int x = option.rect.left();
    const int y = option.rect.top();
    const int w = option.rect.width();
    const int h = option.rect.height();

    // add margins for fixed elements
    QSize ageSize(0, 0); // ageRect could be invalid, so use separate object for calculation
    if (hasAge) {
        ageSize = ageRect->size();
        ageSize.rwidth() += 2 * ageMargin;
    }

    // distribute space among layout items
    QRect message;
    QRect age;
    if (option.direction == Qt::LeftToRight) {
        message.setRect(x, y, w - ageSize.width(), h);
        age.setRect(message.right() + 1, y, ageSize.width(), h);
    } else {
        age.setRect(x, y, ageSize.width(), h);
        message.setRect(age.right() + 1, y, w - ageSize.width(), h);
    }
    // remove margins here, so renderMessageAndAge does not have to
    message.adjust(textMargin, 0, -textMargin, 0);
    age.adjust(ageMargin, 0, -ageMargin, 0);

    // return result
    *ageRect = age;
    *messageRect = QStyle::alignedRect(option.direction, Qt::AlignLeading,
                                       messageRect->size().boundedTo(message.size()), message);
}

void VcsAnnotationItemDelegate::doAuthorLineLayout(const KTextEditor::StyleOptionAnnotationItem& option,
                                                   QRect* authorRect) const
{
    Q_ASSERT(authorRect);

    // if invalid, nothing to be done, keep as is
    if (!authorRect->isValid()) {
        return;
    }

    const QWidget* const widget = option.view;
    QStyle* const style = widget ? widget->style() : QApplication::style();
    // "+ 1" as used in QItemDelegate
    const int authorMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, widget) + 1;

    QRect author = option.rect;
    // remove margins here, so renderAuthor does not have to
    author.adjust(authorMargin, 0, -authorMargin, 0);

    // return result
    *authorRect = QStyle::alignedRect(option.direction, Qt::AlignLeading,
                                      authorRect->size().boundedTo(author.size()), author);
}

void VcsAnnotationItemDelegate::renderBackground(QPainter* painter,
                                                 const KTextEditor::StyleOptionAnnotationItem& option,
                                                 const VcsAnnotationLine& annotationLine) const
{
    Q_UNUSED(option);

    const auto revision = annotationLine.revision();
    auto brushIt = m_backgrounds.find(revision);
    if (brushIt == m_backgrounds.end()) {
        const auto normalStyle = option.view->defaultStyleAttribute(KSyntaxHighlighting::Theme::TextStyle::Normal);
        const auto background = (normalStyle->hasProperty(QTextFormat::BackgroundBrush)) ? normalStyle->background().color() : QColor(Qt::white);
        const int background_y = background.red()*0.299 + 0.587*background.green()
                                                        + 0.114*background.blue();
        // get random, but reproducible 8-bit values from last two bytes of the revision hash
        const auto revisionHash = qHash(revision);
        const int u = static_cast<int>((0xFF & revisionHash));
        const int v = static_cast<int>((0xFF00 & revisionHash) >> 8);
        const int r = qRound(qMin(255.0, qMax(0.0, background_y + 1.402*(v-128))));
        const int g = qRound(qMin(255.0, qMax(0.0, background_y - 0.344*(u-128) - 0.714*(v-128))));
        const int b = qRound(qMin(255.0, qMax(0.0, background_y + 1.772*(u-128))));
        brushIt = m_backgrounds.insert(revision, QBrush(QColor(r, g, b)));
    }

    painter->fillRect(option.rect, brushIt.value());
}

void VcsAnnotationItemDelegate::renderMessageAndAge(QPainter* painter,
                                                    const KTextEditor::StyleOptionAnnotationItem& option,
                                                    const QRect& messageRect, const QString& messageText,
                                                    const QRect& ageRect, const QString& ageText) const
{
    Q_UNUSED(option);

    painter->save();

    const auto normalStyle = option.view->defaultStyleAttribute(KSyntaxHighlighting::Theme::TextStyle::Normal);
    painter->setPen(normalStyle->foreground().color());
    painter->drawText(messageRect, Qt::AlignLeading | Qt::AlignVCenter,
                      painter->fontMetrics().elidedText(messageText, Qt::ElideRight, messageRect.width()));

    // TODO: defaultStyleAttribute only returns reliably for TextStyle::Normal, so what to do for a comment-like color?
    const auto commentStyle = option.view->defaultStyleAttribute(KSyntaxHighlighting::Theme::TextStyle::Normal);
    painter->setPen(commentStyle->foreground().color());
    painter->drawText(ageRect, Qt::AlignTrailing | Qt::AlignVCenter, ageText);

    painter->restore();
}

void VcsAnnotationItemDelegate::renderAuthor(QPainter* painter,
                                             const KTextEditor::StyleOptionAnnotationItem& option,
                                             const QRect& authorRect, const QString& authorText) const
{
    Q_UNUSED(option);

    painter->save();

    // TODO: defaultStyleAttribute only returns reliably for TextStyle::Normal, so what to do for a comment-like color?
    const auto commentStyle = option.view->defaultStyleAttribute(KSyntaxHighlighting::Theme::TextStyle::Normal);
    painter->setPen(commentStyle->foreground().color());
    painter->drawText(authorRect, Qt::AlignLeading | Qt::AlignVCenter,
                      painter->fontMetrics().elidedText(authorText, Qt::ElideRight, authorRect.width()));

    painter->restore();
}

void VcsAnnotationItemDelegate::renderHighlight(QPainter* painter,
                                                const KTextEditor::StyleOptionAnnotationItem& option) const
{
    // Draw a border around all adjacent entries that have the same text as the currently hovered one
    if ((option.state & QStyle::State_MouseOver) &&
        (option.annotationItemGroupingPosition & KTextEditor::StyleOptionAnnotationItem::InGroup)) {
        const auto style = option.view->defaultStyleAttribute(KSyntaxHighlighting::Theme::TextStyle::Normal);
        painter->setPen(style->foreground().color());
        // Use floating point coordinates to support scaled rendering
        QRectF rect(option.rect);
        rect.adjust(0.5, 0.5, -0.5, -0.5);
        // draw left and right highlight borders
        painter->drawLine(rect.topLeft(), rect.bottomLeft());
        painter->drawLine(rect.topRight(), rect.bottomRight());

        if ((option.annotationItemGroupingPosition & KTextEditor::StyleOptionAnnotationItem::GroupBegin) &&
            (option.wrappedLine == 0)) {
            painter->drawLine(rect.topLeft(), rect.topRight());
        }

        if ((option.annotationItemGroupingPosition & KTextEditor::StyleOptionAnnotationItem::GroupEnd) &&
            (option.wrappedLine == (option.wrappedLineCount-1))) {
            painter->drawLine(rect.bottomLeft(), rect.bottomRight());
        }
    }
}

void VcsAnnotationItemDelegate::paint(QPainter* painter, const KTextEditor::StyleOptionAnnotationItem& option,
                                      KTextEditor::AnnotationModel* model, int line) const
{
    Q_ASSERT(painter);
    // we cannot use custom roles and data() API (cmp. VcsAnnotationModel dox), so accessing custom API instead
    auto* vcsModel = qobject_cast<VcsAnnotationModel*>(model);
    Q_ASSERT(vcsModel);
    if (!painter || !vcsModel) {
        return;
    }
    // test of line just for sake of completeness skipped here

    // Fetch data from the model
    const VcsAnnotationLine annotationLine = vcsModel->annotationLine(line);

    if (annotationLine.revision().revisionType() == VcsRevision::Invalid) {
        return;
    }

    // prepare
    painter->save();

    renderBackground(painter, option, annotationLine);

    // We use the normal UI font here, which usually is a proportimal one,
    // so more text fits into the available space.
    // Though we do this at the cost of not adapting to any scaled content font size,
    // as there is no zooming state info available, so we cannot adapt.
    // Tooltip font also is not scaled, and annotations could be considered to fall into
    // that category, so might be fine.
    painter->setFont(option.view->font());

    if (option.visibleWrappedLineInGroup == 0) {
        QRect ageRect;
        QString ageText;
        const auto date = annotationLine.date();
        if (date.isValid()) {
            ageText = ageOfDate(date.date());
            ageRect = QRect(QPoint(0, 0), QSize(option.fontMetrics.horizontalAdvance(ageText), option.rect.height()));
        }
        const auto messageText = annotationLine.commitMessage();
        auto messageRect =
            QRect(QPoint(0, 0), QSize(option.fontMetrics.horizontalAdvance(messageText), option.rect.height()));

        doMessageLineLayout(option, &messageRect, &ageRect);

        renderMessageAndAge(painter, option, messageRect, messageText, ageRect, ageText);
    } else if (option.visibleWrappedLineInGroup == 1) {
        const auto author = annotationLine.author();
        if (!author.isEmpty()) {
            const auto authorText = i18nc("By: commit author", "By: %1", author);
            auto authorRect =
                QRect(QPoint(0, 0), QSize(option.fontMetrics.horizontalAdvance(authorText), option.rect.height()));

            doAuthorLineLayout(option, &authorRect);

            renderAuthor(painter, option, authorRect, authorText);
        }
    }

    renderHighlight(painter, option);

    // done
    painter->restore();
}

bool VcsAnnotationItemDelegate::helpEvent(QHelpEvent* event, KTextEditor::View* view,
                                          const KTextEditor::StyleOptionAnnotationItem& option,
                                          KTextEditor::AnnotationModel* model, int line)
{
    Q_UNUSED(option);
    if (!model || event->type() != QEvent::ToolTip) {
        return false;
    }

    const QVariant data = model->data(line, Qt::ToolTipRole);
    if (!data.isValid()) {
        return false;
    }

    const QString toolTipText = data.toString();
    if (toolTipText.isEmpty()) {
        return false;
    }

    QToolTip::showText(event->globalPos(), toolTipText, view, option.rect);

    return true;
}

void VcsAnnotationItemDelegate::hideTooltip(KTextEditor::View *view)
{
    Q_UNUSED(view);
    QToolTip::hideText();
}

QSize VcsAnnotationItemDelegate::sizeHint(const KTextEditor::StyleOptionAnnotationItem& option,
                                          KTextEditor::AnnotationModel* model, int line) const
{
    Q_UNUSED(line);
    Q_ASSERT(model);
    if (!model) {
        return QSize(0, 0);
    }

    // Ideally the user could configure the width of the annotations, best interactively.
    // Until this is possible, the sizehint is: roughly 40 chars, but maximal 25 % of the view
    // See eventFilter for making sure we adapt the max 25 % to a changed width.

    const QFontMetricsF& fm(option.fontMetrics);
    // if only averageCharWidth would yield sane values,
    // multiply by 40 in average seemed okayish at least with english, showing enough of message
    m_lastCharBasedWidthHint = ceil(40 * fm.averageCharWidth());
    m_lastViewBasedWidthHint = widthHintFromViewWidth(option.view->width());
    return QSize(qMin(m_lastCharBasedWidthHint, m_lastViewBasedWidthHint), fm.height());
}

bool VcsAnnotationItemDelegate::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::Resize) {
        auto resizeEvent = static_cast<QResizeEvent*>(event);
        const int viewBasedWidthHint = widthHintFromViewWidth(resizeEvent->size().width());
        if ((viewBasedWidthHint < m_lastCharBasedWidthHint) &&
            (viewBasedWidthHint != m_lastViewBasedWidthHint)) {
            // emit for first line only, assuming uniformAnnotationItemSizes is set to true
            emit sizeHintChanged(m_model, 0);
        }
    }

    return KTextEditor::AbstractAnnotationItemDelegate::eventFilter(object, event);
}

void VcsAnnotationItemDelegate::resetBackgrounds()
{
    m_backgrounds.clear();
}

int VcsAnnotationItemDelegate::widthHintFromViewWidth(int viewWidth) const
{
    return viewWidth * m_maxWidthViewPercent / 100;
}

#include "moc_vcsannotationitemdelegate.cpp"
