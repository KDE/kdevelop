/*
 *
 */

#include "testcaseswidget.h"

#include <KEditListWidget>
#include <QLayout>
#include <QVBoxLayout>

TestCasesWidget::TestCasesWidget (QWidget* parent, Qt::WindowFlags f) : QWidget (parent, f)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    ui = new KEditListWidget(this);
    layout->addWidget(ui);
    setLayout(layout);
}

TestCasesWidget::~TestCasesWidget()
{

}

void TestCasesWidget::setTestCases (const QStringList& testCases)
{
    ui->setItems(testCases);
}

QStringList TestCasesWidget::testCases() const
{
    return ui->items();
}
