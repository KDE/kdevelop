#include "../quickopen_model.h"

#include <QtGui/QtGui>
#include <QtTest/QtTest>

class tst_QuickOpen_Model: public QObject
{
    Q_OBJECT
private slots:
    void rowCount();

private:
    QuickOpenModel *testModel()
    {
        QStringList list1;
        list1 << "model1, 0" << "model1, 1" << "model1, 2" << "model1, 3" << "model1, 4";
        QStringList list2;
        list2 << "model2, 0" << "model2, 1" << "model2, 2" << "model2, 3" << "model2, 4";

        QuickOpenModel *model = new QuickOpenModel(this);
        QStringListModel *model1 = new QStringListModel(list1, model);
        QStringListModel *model2 = new QStringListModel(list2, model);

        model->addChildModel(model1, "title1");
        model->addChildModel(model2, "title2");

        return model;
    }
};

void tst_QuickOpen_Model::rowCount()
{
    QuickOpenModel *model = testModel();

    COMPARE(model->rowCount(), 12);
}


QTTEST_MAIN(tst_QuickOpen_Model)
#include "tst_quickopen_model.moc"
