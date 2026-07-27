#include <QPoint>
#include <QRect>
#include <QSize>

int main()
{
    QPoint defaultPoint;
    QPoint point(1, 2);
    QPoint negativePoint(-3, -4);

    QSize defaultSize;
    QSize size(10, 20);

    QRect defaultRect;
    QRect rect(1, 2, 30, 40);
    QRect negativeRect(-5, -6, 7, 8);
    QRect emptyRect(1, 2, 0, 0);

    Q_UNUSED(defaultPoint);
    Q_UNUSED(point);
    Q_UNUSED(negativePoint);
    Q_UNUSED(defaultSize);
    Q_UNUSED(size);
    Q_UNUSED(defaultRect);
    Q_UNUSED(rect);
    Q_UNUSED(negativeRect);
    Q_UNUSED(emptyRect);

    return 0; // line 29
}
