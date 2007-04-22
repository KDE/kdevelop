#ifndef KDEVTEAMWORK_LIST_H
#define KDEVTEAMWORK_LIST_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QListView>
#include <QtGui/QWidget>

class Ui_List
{
public:
    QGridLayout *gridLayout;
    QListView *list;

    void setupUi(QWidget *widget)
    {
    widget->setObjectName(QString::fromUtf8("widget"));
    widget->resize(QSize(300, 319).expandedTo(widget->minimumSizeHint()));
    gridLayout = new QGridLayout(widget);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(9);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    list = new QListView(widget);
    list->setObjectName(QString::fromUtf8("list"));

    gridLayout->addWidget(list, 0, 0, 1, 1);

    retranslateUi(widget);

    QMetaObject::connectSlotsByName(widget);
    } // setupUi

    void retranslateUi(QWidget *widget)
    {
    widget->setWindowTitle(QApplication::translate("List", "Form", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(widget);
    } // retranslateUi

};

namespace Ui {
    class List: public Ui_List {};
} // namespace Ui

#endif // KDEVTEAMWORK_LIST_H
