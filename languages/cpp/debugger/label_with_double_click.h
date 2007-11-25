
#ifndef LABEL_WITH_DOUBLE_CLICK_HPP_VP_2006_04_04
#define LABEL_WITH_DOUBLE_CLICK_HPP_VP_2006_04_04

#include <QLabel>
//Added by qt3to4:
#include <QMouseEvent>
#include <kvbox.h>

class LabelWithDoubleClick : public QLabel
{
    Q_OBJECT
public:
    LabelWithDoubleClick(const QString& s, QWidget* parent);

Q_SIGNALS:
    void doubleClicked();

protected:
    void mouseDoubleClickEvent(QMouseEvent*);
};

#endif
