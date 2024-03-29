/*
    SPDX-FileCopyrightText: 2007 Vladimir Prus
    SPDX-FileCopyrightText: 2009-2010 David Nolden

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ACTIVE_TOOLTIP_H
#define KDEVPLATFORM_ACTIVE_TOOLTIP_H

#include <QWidget>
#include "utilexport.h"

namespace KDevelop {
class ActiveToolTipPrivate;

/** This class implements a tooltip that can contain arbitrary
    widgets that the user can interact with.

    Usage example:
    @code
    KDevelop::ActiveToolTip* tooltip = new KDevelop::ActiveToolTip(mainWindow, QCursor::pos());
    QVBoxLayout* layout = new QVBoxLayout(tooltip);
    layout->addWidget(widget);
    tooltip->resize( tooltip->sizeHint() );
    ActiveToolTip::showToolTip(tooltip);
    @endcode
 */
class KDEVPLATFORMUTIL_EXPORT ActiveToolTip : public QWidget
{
    Q_OBJECT

public:
    ///@param parent Parent widget. Must not be zero, else the widget won't be shown.
    /// @param position Position where to show the tooltip, in global coordinates.
    ActiveToolTip(QWidget* parent, const QPoint& position);
    ~ActiveToolTip() override;

    ///Shows and registers the given tool-tip.
    ///This should be used instead of just calling show() to make multiple different
    ///tooltips work together.
    ///The tooltip is owned by the manager after this is called. It will delete itself.
    ///@param tooltip  The tooltip to show. It should not be visible yet, show() will eventually be called from here, with some delay.
    ///                The ownership stays with the caller.
    ///@param priority The priority of this tooltip. Lower is better. Multiple tooltips will be stacked down in the given order.
    ///                If it is zero, the given tooltip will be shown exclusively.
    ///@param uniqueId If this is nonempty, ActiveTooltip will make sure that only one tooltip with the given id is shown at a time
    static void showToolTip(ActiveToolTip* tooltip, float priority = 100, const QString& uniqueId = QString());

    bool eventFilter(QObject* object, QEvent* e) override;

    bool insideThis(QObject* object);

    void showEvent(QShowEvent*) override;

    void resizeEvent(QResizeEvent*) override;

    void paintEvent(QPaintEvent*) override;

    void adjustRect();

    ///Clicks within the friend widget are allowed
    void addFriendWidget(QWidget* widget);

    ///Set rect of handle (object) this tool tip is created for
    ///Moving mouse inside this rect, and between this and bounding geometry won't hide the tooltip
    void setHandleRect(const QRect& rect);

    ///Set the area within which the mouse can be moved freely without hiding the tooltip
    void setBoundingGeometry(const QRect& geometry);
Q_SIGNALS:
    void resized();
    // Emitted whenever mouse-activity is noticed within the tooltip area
    void mouseIn();
    // Emitted whenever mouse-activity is noticed outside of the tooltip area
    void mouseOut();

private:
    void closeEvent(QCloseEvent*) override;

private:
    const QScopedPointer<class ActiveToolTipPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ActiveToolTip)
};

}

#endif
