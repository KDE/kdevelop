#include <QtGui/QtGui>
#include <kfiltermodel.h>
#include <kdevitemmodel.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

#if 0
    QTreeWidget view1;
    view1.setColumnCount(1);
    for (int i = 0; i < 100; ++i) {
        QTreeWidgetItem *i1 = new QTreeWidgetItem(&view1, QStringList("i1_" + QString::number(i)));
        QTreeWidgetItem *i2 = new QTreeWidgetItem(i1, QStringList("i2_" + QString::number(i)));
        new QTreeWidgetItem(i2, QStringList("i31_" + QString::number(i)));
        new QTreeWidgetItem(i2, QStringList("i32_" + QString::number(i)));
        new QTreeWidgetItem(i2, QStringList("i33_" + QString::number(i)));
    }
#else
    QTreeView view1;

    KDevItemCollection *c = new KDevItemCollection("parent1");
    c->add(new KDevItemCollection("child1"));
    KDevItemCollection *c2 = new KDevItemCollection("child2");
    c->add(c2);
    c2->add(new KDevItemCollection("child21"));

    KDevItemModel model;
    model.appendItem(c);

    view1.setModel(&model);
#endif

    view1.show();

    QWidget widget;
    QLineEdit edit;
    QVBoxLayout layout(&widget);

    KFilterModel filter(view1.model());
    QTreeView view2;
    view2.setModel(&filter);

    layout.addWidget(&edit);
    layout.addWidget(&view2);

    QObject::connect(&edit, SIGNAL(textChanged(QString)), &filter, SLOT(setFilter(QString)));

    widget.show();

    return app.exec();
}

