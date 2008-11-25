
#ifndef ACTIVE_TOOLTIP_H_a21320d5b6a374c6b0be33532dd89703
#define ACTIVE_TOOLTIP_H_a21320d5b6a374c6b0be33532dd89703

#include <QWidget>
#include <QPoint>
#include <QPalette>
#include <QApplication>
#include <QEvent>
#include <QMouseEvent>

/** This class implements a tooltip that can contain arbitrary
    widgets that the user can interact with.

    There is a two-step activation scheme for the widget.  When
    created, it records the parent widget and the position, and
    arranges for the object to be closed and deleted if mouse
    leaves the region around the original position, or if we click
    in different widget.

    The widget is not originally shown, so we can fetch whatever data
    is necessary to be displayed.  When we are ready to show the
    data, the 'show()' method must be called.  It will show the widget
    and extend the area we can interact with to cover the widget.  */
class ActiveToolTip : public QWidget
{
public:
    /* position must be in global coordinates.  */
    ActiveToolTip(QWidget *parent, const QPoint& position)
    : QWidget(parent, Qt::ToolTip), mouseOut_(0)
    {
        setAttribute(Qt::WA_DeleteOnClose);
        setMouseTracking(true);
        rect_ = QRect(position, position);
        rect_.adjust(-10, -10, 10, 10);
        move(position);

        QPalette p;
        p.setColor(backgroundRole(), p.color(QPalette::ToolTipBase));
        p.setColor(QPalette::Base, p.color(QPalette::ToolTipBase));
        setPalette(p);

        qApp->installEventFilter(this);
    }

    bool eventFilter(QObject *object, QEvent *e)
    {
        switch (e->type()) {

        case QEvent::WindowActivate:
        case QEvent::WindowDeactivate:
            close();
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::Wheel:
            /* If the click is within tooltip, it's fine.
               Clicks outside close it.  */
            if (!insideThis(object))
                close();

        // FIXME: revisit this code later.
#if 0
        case QEvent::FocusIn:
        case QEvent::FocusOut:
            close();
            break;
#endif
        case QEvent::MouseMove:
            if (!rect_.isNull() 
                && !rect_.contains(static_cast<QMouseEvent*>(e)->globalPos()))
                // On X, when the cursor leaves the tooltip and enters
                // the parent, we sometimes get some wrong Y coordinate.
                // Don't know why, so wait for two out-of-range mouse
                // positions before closing.
                ++mouseOut_;
            else               
                mouseOut_ = 0;
            if (mouseOut_ == 2)
                close();
        default:
            break;
        }
        return false;
    }

    bool insideThis(QObject* object)
    {
        while (object)
        {
            if (object == this)
            {
                return true;
            }
            object = object->parent();
        }
        return false;
    }

    void showEvent(QShowEvent*)
    {        
        adjustRect();
    }

    void resizeEvent(QResizeEvent*)
    {
        adjustRect();
    }

    void adjustRect()
    {
        // For tooltip widget, geometry() returns global coordinates.
        QRect r = geometry();
        r.adjust(-10, -10, 10, 10);
        rect_ = r;
    }

private:
    QRect rect_;
    int mouseOut_;
};



#endif
