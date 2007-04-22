#ifndef KDEVTEAMWORK_TIMESTAMPWIDGET_H
#define KDEVTEAMWORK_TIMESTAMPWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDateTimeEdit>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>
#include "knuminput.h"

class Ui_TimestampWidget
{
public:
    QGridLayout *gridLayout;
    QHBoxLayout *hboxLayout;
    QPushButton *tail;
    QPushButton *none;
    QPushButton *enableAll;
    KIntNumInput *stamp;
    QLabel *stampCount;
    QDateTimeEdit *date;
    QLabel *lastDate;
    QCheckBox *enableBox;
    QLabel *replacementText;

    void setupUi(QWidget *TimestampWidget)
    {
    TimestampWidget->setObjectName(QString::fromUtf8("TimestampWidget"));
    TimestampWidget->resize(QSize(950, 29).expandedTo(TimestampWidget->minimumSizeHint()));
    gridLayout = new QGridLayout(TimestampWidget);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(0);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    tail = new QPushButton(TimestampWidget);
    tail->setObjectName(QString::fromUtf8("tail"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(0));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(tail->sizePolicy().hasHeightForWidth());
    tail->setSizePolicy(sizePolicy);

    hboxLayout->addWidget(tail);

    none = new QPushButton(TimestampWidget);
    none->setObjectName(QString::fromUtf8("none"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(0));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(none->sizePolicy().hasHeightForWidth());
    none->setSizePolicy(sizePolicy1);

    hboxLayout->addWidget(none);

    enableAll = new QPushButton(TimestampWidget);
    enableAll->setObjectName(QString::fromUtf8("enableAll"));
    QSizePolicy sizePolicy2(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(0));
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(enableAll->sizePolicy().hasHeightForWidth());
    enableAll->setSizePolicy(sizePolicy2);

    hboxLayout->addWidget(enableAll);

    stamp = new KIntNumInput(TimestampWidget);
    stamp->setObjectName(QString::fromUtf8("stamp"));
    QSizePolicy sizePolicy3(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(stamp->sizePolicy().hasHeightForWidth());
    stamp->setSizePolicy(sizePolicy3);
    stamp->setMaximumSize(QSize(120, 16777215));
    stamp->setMinimum(0);

    hboxLayout->addWidget(stamp);

    stampCount = new QLabel(TimestampWidget);
    stampCount->setObjectName(QString::fromUtf8("stampCount"));
    QSizePolicy sizePolicy4(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(stampCount->sizePolicy().hasHeightForWidth());
    stampCount->setSizePolicy(sizePolicy4);

    hboxLayout->addWidget(stampCount);

    date = new QDateTimeEdit(TimestampWidget);
    date->setObjectName(QString::fromUtf8("date"));
    QSizePolicy sizePolicy5(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(0));
    sizePolicy5.setHorizontalStretch(0);
    sizePolicy5.setVerticalStretch(0);
    sizePolicy5.setHeightForWidth(date->sizePolicy().hasHeightForWidth());
    date->setSizePolicy(sizePolicy5);

    hboxLayout->addWidget(date);

    lastDate = new QLabel(TimestampWidget);
    lastDate->setObjectName(QString::fromUtf8("lastDate"));
    QSizePolicy sizePolicy6(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy6.setHorizontalStretch(0);
    sizePolicy6.setVerticalStretch(0);
    sizePolicy6.setHeightForWidth(lastDate->sizePolicy().hasHeightForWidth());
    lastDate->setSizePolicy(sizePolicy6);

    hboxLayout->addWidget(lastDate);

    enableBox = new QCheckBox(TimestampWidget);
    enableBox->setObjectName(QString::fromUtf8("enableBox"));
    QSizePolicy sizePolicy7(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(0));
    sizePolicy7.setHorizontalStretch(0);
    sizePolicy7.setVerticalStretch(0);
    sizePolicy7.setHeightForWidth(enableBox->sizePolicy().hasHeightForWidth());
    enableBox->setSizePolicy(sizePolicy7);
    enableBox->setTristate(true);

    hboxLayout->addWidget(enableBox);

    replacementText = new QLabel(TimestampWidget);
    replacementText->setObjectName(QString::fromUtf8("replacementText"));

    hboxLayout->addWidget(replacementText);


    gridLayout->addLayout(hboxLayout, 0, 0, 1, 1);

    retranslateUi(TimestampWidget);

    QMetaObject::connectSlotsByName(TimestampWidget);
    } // setupUi

    void retranslateUi(QWidget *TimestampWidget)
    {
    TimestampWidget->setWindowTitle(QApplication::translate("TimestampWidget", "Form", 0, QApplication::UnicodeUTF8));
    tail->setText(QApplication::translate("TimestampWidget", "Tail", 0, QApplication::UnicodeUTF8));
    none->setText(QApplication::translate("TimestampWidget", "None", 0, QApplication::UnicodeUTF8));
    enableAll->setText(QApplication::translate("TimestampWidget", "Enable All(x)", 0, QApplication::UnicodeUTF8));
    stampCount->setText(QApplication::translate("TimestampWidget", "(last stamp)", 0, QApplication::UnicodeUTF8));
    lastDate->setText(QApplication::translate("TimestampWidget", "(last date)", 0, QApplication::UnicodeUTF8));
    enableBox->setText(QApplication::translate("TimestampWidget", "Enable", 0, QApplication::UnicodeUTF8));
    replacementText->setText(QApplication::translate("TimestampWidget", "replacement-text", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(TimestampWidget);
    } // retranslateUi

};

namespace Ui {
    class TimestampWidget: public Ui_TimestampWidget {};
} // namespace Ui

#endif // KDEVTEAMWORK_TIMESTAMPWIDGET_H
