#include "../quickopen_model.h"
#include "../quickopen_filtermodel.h"

#include <QtGui/QtGui>
#include <QtTest/QtTest>

class tst_QuickOpenModel: public QObject
{
    Q_OBJECT
private slots:
    void rowCount();
    void data();
    void emptyModel();
    void firstEmpty();
    void lastEmpty();

    void testFiltering();

private:
    QuickOpenModel *testModel()
    {
        QStringList list1;
        list1 << "model1, 0" << "model1, 1" << "model1, 2" << "model1, 3" << "model1, 4";
        QStringList list2;
        list2 << "model2, 0" << "model2, 1" << "model2, 2" << "model2, 3" << "model2, 4";
        QStringList list3;
        list3 << "model5, 0" << "model5, 1" << "model5, 2" << "model5, 3" << "model5, 4";

        QuickOpenModel *model = new QuickOpenModel(this);
        QStringListModel *model1 = new QStringListModel(list1, model);
        QStringListModel *model2 = new QStringListModel(list2, model);
        QStringListModel *model3 = new QStringListModel(QStringList(), model);
        QStringListModel *model4 = new QStringListModel(QStringList(), model);
        QStringListModel *model5 = new QStringListModel(list3, model);

        model->addChildModel(model1, "title1");
        model->addChildModel(model2, "title2");
        model->addChildModel(model3, "title3");
        model->addChildModel(model4, "title4");
        model->addChildModel(model5, "title5");

        return model;
    }
};

void tst_QuickOpenModel::rowCount()
{
    QuickOpenModel *model = testModel();

    COMPARE(model->rowCount(), 18);
}

static void checkTestModel(QAbstractItemModel *model)
{
    COMPARE(model->data(model->index(0, 0)).toString(), QString("title1"));
    COMPARE(model->data(model->index(1, 0)).toString(), QString("model1, 0"));
    COMPARE(model->data(model->index(2, 0)).toString(), QString("model1, 1"));
    COMPARE(model->data(model->index(3, 0)).toString(), QString("model1, 2"));
    COMPARE(model->data(model->index(4, 0)).toString(), QString("model1, 3"));
    COMPARE(model->data(model->index(5, 0)).toString(), QString("model1, 4"));
    COMPARE(model->data(model->index(6, 0)).toString(), QString("title2"));
    COMPARE(model->data(model->index(7, 0)).toString(), QString("model2, 0"));
    COMPARE(model->data(model->index(8, 0)).toString(), QString("model2, 1"));
    COMPARE(model->data(model->index(9, 0)).toString(), QString("model2, 2"));
    COMPARE(model->data(model->index(10, 0)).toString(), QString("model2, 3"));
    COMPARE(model->data(model->index(11, 0)).toString(), QString("model2, 4"));
    COMPARE(model->data(model->index(12, 0)).toString(), QString("title5"));
    COMPARE(model->data(model->index(13, 0)).toString(), QString("model5, 0"));
    COMPARE(model->data(model->index(14, 0)).toString(), QString("model5, 1"));
    COMPARE(model->data(model->index(15, 0)).toString(), QString("model5, 2"));
    COMPARE(model->data(model->index(16, 0)).toString(), QString("model5, 3"));
    COMPARE(model->data(model->index(17, 0)).toString(), QString("model5, 4"));
    VERIFY(!model->index(18, 0).isValid());
}

void tst_QuickOpenModel::data()
{
    QuickOpenModel *model = testModel();

    checkTestModel(model);
}

void tst_QuickOpenModel::emptyModel()
{
    QuickOpenModel model;
    QStringList list;
    QStringListModel cModel(list);
    model.addChildModel(&cModel, "title1");

    COMPARE(model.rowCount(), 0);
}

void tst_QuickOpenModel::lastEmpty()
{
    QuickOpenModel *model = testModel();

    QStringList list;
    QStringListModel cModel(list);

    model->addChildModel(&cModel, "titlex");
    checkTestModel(model);
}

void tst_QuickOpenModel::firstEmpty()
{
    QuickOpenModel model;

    QStringList list;
    QStringListModel cModel1(list);

    list << "hello";
    QStringListModel cModel2(list);

    model.addChildModel(&cModel1, "title1");
    model.addChildModel(&cModel2, "title2");

    COMPARE(model.data(model.index(0, 0)).toString(), QString("title2"));
    COMPARE(model.data(model.index(1, 0)).toString(), QString("hello"));
    VERIFY(!model.index(2, 0).isValid());
}

void tst_QuickOpenModel::testFiltering()
{
    QuickOpenModel *source = testModel();

    QuickOpenFilterModel model(source);
    checkTestModel(&model);

    model.setFilter("model5");
    COMPARE(model.rowCount(), 6);
    COMPARE(model.data(model.index(0, 0)).toString(), QString("title5"));
    COMPARE(model.data(model.index(1, 0)).toString(), QString("model5, 0"));
    COMPARE(model.data(model.index(2, 0)).toString(), QString("model5, 1"));
    COMPARE(model.data(model.index(3, 0)).toString(), QString("model5, 2"));
    COMPARE(model.data(model.index(4, 0)).toString(), QString("model5, 3"));
    COMPARE(model.data(model.index(5, 0)).toString(), QString("model5, 4"));
    VERIFY(!model.index(6, 0).isValid());

    model.setFilter("3");
    COMPARE(model.rowCount(), 6);
    COMPARE(model.data(model.index(0, 0)).toString(), QString("title1"));
    COMPARE(model.data(model.index(1, 0)).toString(), QString("model1, 3"));
    COMPARE(model.data(model.index(2, 0)).toString(), QString("title2"));
    COMPARE(model.data(model.index(3, 0)).toString(), QString("model2, 3"));
    COMPARE(model.data(model.index(4, 0)).toString(), QString("title5"));
    COMPARE(model.data(model.index(5, 0)).toString(), QString("model5, 3"));
    VERIFY(!model.index(6, 0).isValid());
}

QTTEST_MAIN(tst_QuickOpenModel)
#include "tst_quickopen_model.moc"
