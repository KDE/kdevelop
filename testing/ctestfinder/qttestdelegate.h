#ifndef QTTESTDELEGATE_H
#define QTTESTDELEGATE_H

#include <QItemDelegate>
#include <KColorScheme>

class QtTestDelegate : public QItemDelegate
{
    Q_OBJECT
    
public:
    explicit QtTestDelegate(QObject* parent = 0);
    virtual ~QtTestDelegate();
    
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    
private:
    void highlight(QStyleOptionViewItem& option, const KStatefulBrush& brush, bool bold = true) const;
    
    KStatefulBrush passBrush;
    KStatefulBrush failBrush;
    KStatefulBrush xFailBrush;
    KStatefulBrush debugBrush;
};

#endif // QTTESTDELEGATE_H
