#include "qttestdelegate.h"

#include <KDebug>

QtTestDelegate::QtTestDelegate(QObject* parent) : QItemDelegate(parent),
failBrush(KColorScheme::View, KColorScheme::NegativeText),
passBrush(KColorScheme::View, KColorScheme::PositiveText),
xFailBrush(KColorScheme::View, KColorScheme::NeutralText),
debugBrush(KColorScheme::View, KColorScheme::NormalText)
{

}

QtTestDelegate::~QtTestDelegate()
{

}

void QtTestDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString line = index.data().toString();
    kDebug() << line;
    QStyleOptionViewItem opt = option;
    if (line.startsWith("PASS"))
    {
        highlight(opt, passBrush);
    }
    else if (line.startsWith("FAIL"))
    {
        highlight(opt, failBrush);
    }
    else if (line.startsWith("XFAIL"))
    {
        highlight(opt, xFailBrush);
    }
    else if (line.startsWith("QDEBUG"))
    {
        highlight(opt, debugBrush);
    }
    QItemDelegate::paint(painter, opt, index);
}

void QtTestDelegate::highlight(QStyleOptionViewItem& option, const KStatefulBrush& brush, bool bold) const
{
    option.font.setBold(bold);
    option.palette.setBrush(QPalette::Text, brush.brush(option.palette));
}
