
#include "label_with_double_click.h"
//Added by qt3to4:
#include <QMouseEvent>
#include <QLabel>

LabelWithDoubleClick::LabelWithDoubleClick(const QString& s, QWidget* parent)
: QLabel(s, parent)
{}

void LabelWithDoubleClick::mouseDoubleClickEvent(QMouseEvent*)
{
    emit doubleClicked();
}


#include "label_with_double_click.moc"
