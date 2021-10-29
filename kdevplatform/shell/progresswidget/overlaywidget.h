/*
    SPDX-FileCopyrightText: 2004 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_OVERLAYWIDGET_H
#define KDEVPLATFORM_OVERLAYWIDGET_H

#include <QWidget>

namespace KDevelop {

/**
 * This is a widget that can align itself with another one, without using a layout,
 * so that it can actually be on top of other widgets.
 * Currently the only supported type of alignment is "right aligned, on top of the other widget".
 */
class OverlayWidget : public QWidget
{
    Q_OBJECT

public:
    OverlayWidget( QWidget* alignWidget, QWidget* parent, const char* name = nullptr );
    ~OverlayWidget() override;

    QWidget * alignWidget() { return mAlignWidget; }
    void setAlignWidget( QWidget * alignWidget );

protected:
    void resizeEvent( QResizeEvent* ev ) override;
    bool eventFilter( QObject* o, QEvent* e) override;

private:
    void reposition();

private:
    QWidget * mAlignWidget;
};

} // namespace

#endif /* OVERLAYWIDGET_H */

