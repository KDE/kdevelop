#ifndef VERITAS_OVERLAYBUTTON_H
#define VERITAS_OVERLAYBUTTON_H

#include <QAbstractButton>
#include <QModelIndex>

namespace Veritas
{
class Test;
class OverlayButton : public QAbstractButton
{
Q_OBJECT

public:
    OverlayButton(QWidget* parent) : QAbstractButton(parent) {}
    virtual ~OverlayButton() {}

    virtual void reset() = 0;
    virtual int offset() = 0;
    virtual QModelIndex index() = 0;
    virtual void setIndex(const QModelIndex&) = 0;
    virtual bool shouldShow(Test*) = 0;
};

}

#endif // VERITAS_OVERLAYBUTTON_H
