
#ifndef LABEL_WITH_DOUBLE_CLICK_HPP_VP_2006_04_04
#define LABEL_WITH_DOUBLE_CLICK_HPP_VP_2006_04_04

#include <qlabel.h>

class LabelWithDoubleClick : public QLabel
{
    Q_OBJECT
public:
    LabelWithDoubleClick(const QString& s, QWidget* parent);

signals:
    void doubleClicked();

protected:
    void mouseDoubleClickEvent(QMouseEvent*);
};

#endif
