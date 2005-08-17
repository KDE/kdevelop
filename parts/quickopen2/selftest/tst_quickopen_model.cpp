#include "../quickopen_model.h"
#include <kdevelop/util/kfiltermodel.h>

#include <QtGui/QtGui>
#include <QtTest/QtTest>

class tst_QuickOpenModel: public QObject
{
    Q_OBJECT
private slots:
    void rowCount();
    void data();
    void emptyModel();

    void parent();

    void testFiltering();

private:
    QuickOpenModel *testModel()
    {
        QStringList list1;
        list1 << "model1, 0" << "model1, 1" << "model1, 2" << "model1, 3" << "model1, 4";
        QStringList list2;
        list2 << "model2, 0" << "model2, 1" << "model2, 2" << "model2, 3" << "model2, 4";
        QStringList list3;
        list3 << "model4, 0" << "model4, 1" << "model4, 2" << "model4, 3" << "model4, 4";

        QuickOpenModel *model = new QuickOpenModel(this);
        QStringListModel *model1 = new QStringListModel(list1, model);
        QStringListModel *model2 = new QStringListModel(list2, model);
        QStringListModel *model3 = new QStringListModel(QStringList(), model);
        QStringListModel *model4 = new QStringListModel(list3, model);

        model->addChildModel(model1, "title1");
        model->addChildModel(model2, "title2");
        model->addChildModel(model3, "title3");
        model->addChildModel(model4, "title4");

        return model;
    }
};

void tst_QuickOpenModel::rowCount()
{
    QuickOpenModel *model = testModel();

    COMPARE(model->rowCount(), 4);
    COMPARE(model->rowCount(model->index(0, 0)), 5);
    COMPARE(model->rowCount(model->index(1, 0)), 5);
    COMPARE(model->rowCount(model->index(2, 0)), 0);
    COMPARE(model->rowCount(model->index(3, 0)), 5);

    COMPARE(model->rowCount(model->index(0, 0, model->index(0, 0))), 0);
}

void tst_QuickOpenModel::parent()
{
    QuickOpenModel *model = testModel();

    COMPARE(model->parent(model->index(0, 0, model->index(0, 0))), model->index(0, 0));
    COMPARE(model->parent(model->index(1, 0, model->index(0, 0))), model->index(0, 0));
    COMPARE(model->parent(model->index(2, 0, model->index(0, 0))), model->index(0, 0));
    COMPARE(model->parent(model->index(3, 0, model->index(0, 0))), model->index(0, 0));

    COMPARE(model->parent(model->index(0, 0, model->index(1, 0))), model->index(1, 0));
    COMPARE(model->parent(model->index(1, 0, model->index(1, 0))), model->index(1, 0));
    COMPARE(model->parent(model->index(2, 0, model->index(1, 0))), model->index(1, 0));
    COMPARE(model->parent(model->index(3, 0, model->index(1, 0))), model->index(1, 0));
}

void tst_QuickOpenModel::data()
{
    QuickOpenModel *model = testModel();

    COMPARE(model->data(model->index(0, 0)).toString(), QString("title1"));
     COMPARE(model->data(model->index(0, 0, model->index(0, 0))).toString(), QString("model1, 0"));
     COMPARE(model->data(model->index(1, 0, model->index(0, 0))).toString(), QString("model1, 1"));
     COMPARE(model->data(model->index(2, 0, model->index(0, 0))).toString(), QString("model1, 2"));
     COMPARE(model->data(model->index(3, 0, model->index(0, 0))).toString(), QString("model1, 3"));
     COMPARE(model->data(model->index(4, 0, model->index(0, 0))).toString(), QString("model1, 4"));
     VERIFY(!model->index(5, 0, model->index(0, 0)).isValid());

    COMPARE(model->data(model->index(1, 0)).toString(), QString("title2"));
     COMPARE(model->data(model->index(0, 0, model->index(1, 0))).toString(), QString("model2, 0"));
     COMPARE(model->data(model->index(1, 0, model->index(1, 0))).toString(), QString("model2, 1"));
     COMPARE(model->data(model->index(2, 0, model->index(1, 0))).toString(), QString("model2, 2"));
     COMPARE(model->data(model->index(3, 0, model->index(1, 0))).toString(), QString("model2, 3"));
     COMPARE(model->data(model->index(4, 0, model->index(1, 0))).toString(), QString("model2, 4"));
     VERIFY(!model->index(5, 0, model->index(1, 0)).isValid());
    COMPARE(model->data(model->index(2, 0)).toString(), QString("title3"));
     VERIFY(!model->index(0, 0, model->index(2, 0)).isValid());
    COMPARE(model->data(model->index(3, 0)).toString(), QString("title4"));
     COMPARE(model->data(model->index(0, 0, model->index(3, 0))).toString(), QString("model4, 0"));
     COMPARE(model->data(model->index(1, 0, model->index(3, 0))).toString(), QString("model4, 1"));
     COMPARE(model->data(model->index(2, 0, model->index(3, 0))).toString(), QString("model4, 2"));
     COMPARE(model->data(model->index(3, 0, model->index(3, 0))).toString(), QString("model4, 3"));
     COMPARE(model->data(model->index(4, 0, model->index(3, 0))).toString(), QString("model4, 4"));
     VERIFY(!model->index(5, 0, model->index(3, 0)).isValid());

    VERIFY(!model->index(4, 0).isValid());
}

void tst_QuickOpenModel::emptyModel()
{
    QuickOpenModel model;
    COMPARE(model.rowCount(), 0);
}

void tst_QuickOpenModel::testFiltering()
{
    QuickOpenModel *source = testModel();

    KFilterModel model(source);
    model.setFilter("model4");

    COMPARE(model.rowCount(), 1);
    COMPARE(model.data(model.index(0, 0)).toString(), QString("title4"));
    VERIFY(!model.index(1, 0).isValid());

    QModelIndex parent = model.index(0, 0);
    COMPARE(model.rowCount(parent), 5);
    COMPARE(model.data(model.index(0, 0, parent)).toString(), QString("model4, 0"));
    COMPARE(model.data(model.index(1, 0, parent)).toString(), QString("model4, 1"));
    COMPARE(model.data(model.index(2, 0, parent)).toString(), QString("model4, 2"));
    COMPARE(model.data(model.index(3, 0, parent)).toString(), QString("model4, 3"));
    COMPARE(model.data(model.index(4, 0, parent)).toString(), QString("model4, 4"));
    VERIFY(!model.index(5, 0, parent).isValid());

    COMPARE(model.rowCount(model.index(0, 0, parent)), 0);
    COMPARE(model.rowCount(model.index(1, 0, parent)), 0);
    COMPARE(model.rowCount(model.index(2, 0, parent)), 0);
    COMPARE(model.rowCount(model.index(3, 0, parent)), 0);
    COMPARE(model.rowCount(model.index(4, 0, parent)), 0);

    COMPARE(model.parent(model.index(0, 0, parent)), model.index(0, 0));
    COMPARE(model.parent(model.index(1, 0, parent)), model.index(0, 0));
    COMPARE(model.parent(model.index(2, 0, parent)), model.index(0, 0));
    COMPARE(model.parent(model.index(3, 0, parent)), model.index(0, 0));
    COMPARE(model.parent(model.index(4, 0, parent)), model.index(0, 0));
}

QTTEST_MAIN(tst_QuickOpenModel)
#include "tst_quickopen_model.moc"
