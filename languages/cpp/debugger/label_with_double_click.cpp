
#include "label_with_double_click.h"

LabelWithDoubleClick::LabelWithDoubleClick(const QString& s, QWidget* parent)
: QLabel(s, parent)
{}

void LabelWithDoubleClick::mouseDoubleClickEvent(QMouseEvent*)
{
    emit doubleClicked();
}


#include "label_with_double_click.moc"
