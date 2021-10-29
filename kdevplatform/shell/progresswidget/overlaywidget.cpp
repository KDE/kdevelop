/*
    SPDX-FileCopyrightText: 2004 David Faure <faure@kde.org>

    SPDX-License-Identifier: GPL-2.0-only
*/

#include "overlaywidget.h"

#include <QHBoxLayout>
#include <QResizeEvent>
#include <QEvent>
#include <QApplication>

using namespace KDevelop;

OverlayWidget::OverlayWidget( QWidget* alignWidget, QWidget* parent, const char* name )
    : QWidget( parent, Qt::Window | Qt::FramelessWindowHint | Qt::ToolTip ), mAlignWidget( nullptr )
{
    auto hboxHBoxLayout = new QHBoxLayout(this);
    hboxHBoxLayout->setContentsMargins(0, 0, 0, 0);

    setObjectName(QString::fromUtf8(name));
    setAlignWidget( alignWidget );

    setWindowFlags(Qt::WindowDoesNotAcceptFocus | windowFlags());

    qApp->installEventFilter(this);
}

OverlayWidget::~OverlayWidget()
{
}

void OverlayWidget::reposition()
{
    if ( !mAlignWidget )
        return;
    // p is in the alignWidget's coordinates
    QPoint p;
    // We are always above the alignWidget, right-aligned with it.
    p.setX( mAlignWidget->width() - width() );
    p.setY( -height() );
    // Position in the global coordinates
    QPoint global = mAlignWidget->mapToGlobal( p );
    // Move 'this' to that position.
    move( global );
}

void OverlayWidget::setAlignWidget( QWidget * w )
{
    if (w == mAlignWidget)
        return;

    mAlignWidget = w;

    reposition();
}

bool OverlayWidget::eventFilter( QObject* o, QEvent* e)
{
    if (e->type() == QEvent::Move || e->type() == QEvent::Resize) {
        reposition();
    } else if (e->type() == QEvent::Close) {
        close();
    }

    return QWidget::eventFilter(o,e);
}

void OverlayWidget::resizeEvent( QResizeEvent* ev )
{
    reposition();
    QWidget::resizeEvent( ev );
}

