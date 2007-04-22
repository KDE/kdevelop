#ifndef KDEVTEAMWORK_VECTORTIMESTAMPWIDGET_H
#define KDEVTEAMWORK_VECTORTIMESTAMPWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QListView>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

class Ui_VectorTimestampWidget
{
public:
    QGridLayout *gridLayout;
    QVBoxLayout *vboxLayout;
    QGroupBox *timestampsGroup;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout1;
    QListView *log;
    QHBoxLayout *hboxLayout;
    QPushButton *toTailTimestamp;
    QPushButton *clearLog;
    QLabel *stateInfo;
    QSpacerItem *spacerItem;
    QPushButton *finish;

    void setupUi(QWidget *VectorTimestampWidget)
    {
    VectorTimestampWidget->setObjectName(QString::fromUtf8("VectorTimestampWidget"));
    VectorTimestampWidget->resize(QSize(760, 172).expandedTo(VectorTimestampWidget->minimumSizeHint()));
    gridLayout = new QGridLayout(VectorTimestampWidget);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(0);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    vboxLayout = new QVBoxLayout();
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(0);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    timestampsGroup = new QGroupBox(VectorTimestampWidget);
    timestampsGroup->setObjectName(QString::fromUtf8("timestampsGroup"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(5), static_cast<QSizePolicy::Policy>(7));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(timestampsGroup->sizePolicy().hasHeightForWidth());
    timestampsGroup->setSizePolicy(sizePolicy);

    vboxLayout->addWidget(timestampsGroup);

    groupBox_2 = new QGroupBox(VectorTimestampWidget);
    groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
    gridLayout1 = new QGridLayout(groupBox_2);
    gridLayout1->setSpacing(6);
    gridLayout1->setMargin(9);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    log = new QListView(groupBox_2);
    log->setObjectName(QString::fromUtf8("log"));
    log->setSelectionMode(QAbstractItemView::NoSelection);
    log->setSelectionBehavior(QAbstractItemView::SelectRows);
    log->setResizeMode(QListView::Fixed);
    log->setLayoutMode(QListView::Batched);
    log->setBatchSize(50);

    gridLayout1->addWidget(log, 0, 0, 1, 1);


    vboxLayout->addWidget(groupBox_2);

    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    toTailTimestamp = new QPushButton(VectorTimestampWidget);
    toTailTimestamp->setObjectName(QString::fromUtf8("toTailTimestamp"));

    hboxLayout->addWidget(toTailTimestamp);

    clearLog = new QPushButton(VectorTimestampWidget);
    clearLog->setObjectName(QString::fromUtf8("clearLog"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(0));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(clearLog->sizePolicy().hasHeightForWidth());
    clearLog->setSizePolicy(sizePolicy1);

    hboxLayout->addWidget(clearLog);

    stateInfo = new QLabel(VectorTimestampWidget);
    stateInfo->setObjectName(QString::fromUtf8("stateInfo"));

    hboxLayout->addWidget(stateInfo);

    spacerItem = new QSpacerItem(301, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addItem(spacerItem);

    finish = new QPushButton(VectorTimestampWidget);
    finish->setObjectName(QString::fromUtf8("finish"));

    hboxLayout->addWidget(finish);


    vboxLayout->addLayout(hboxLayout);


    gridLayout->addLayout(vboxLayout, 0, 0, 1, 1);

    retranslateUi(VectorTimestampWidget);

    QMetaObject::connectSlotsByName(VectorTimestampWidget);
    } // setupUi

    void retranslateUi(QWidget *VectorTimestampWidget)
    {
    VectorTimestampWidget->setWindowTitle(QApplication::translate("VectorTimestampWidget", "Form", 0, QApplication::UnicodeUTF8));
    timestampsGroup->setTitle(QApplication::translate("VectorTimestampWidget", "Timestamps", 0, QApplication::UnicodeUTF8));
    groupBox_2->setTitle(QApplication::translate("VectorTimestampWidget", "Log", 0, QApplication::UnicodeUTF8));
    toTailTimestamp->setText(QApplication::translate("VectorTimestampWidget", "To Tail State", 0, QApplication::UnicodeUTF8));
    clearLog->setText(QApplication::translate("VectorTimestampWidget", "Clear Log", 0, QApplication::UnicodeUTF8));
    stateInfo->setText(QApplication::translate("VectorTimestampWidget", "Current State:    Tail State: ", 0, QApplication::UnicodeUTF8));
    finish->setText(QApplication::translate("VectorTimestampWidget", "Finish", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(VectorTimestampWidget);
    } // retranslateUi

};

namespace Ui {
    class VectorTimestampWidget: public Ui_VectorTimestampWidget {};
} // namespace Ui

#endif // KDEVTEAMWORK_VECTORTIMESTAMPWIDGET_H
