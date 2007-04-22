#ifndef KDEVTEAMWORK_MESSAGEHISTORY_H
#define KDEVTEAMWORK_MESSAGEHISTORY_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDateEdit>
#include <QtGui/QDockWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QListView>
#include <QtGui/QPushButton>
#include <QtGui/QTreeView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

class Ui_MessageHistory
{
public:
    QGridLayout *gridLayout;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout1;
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;
    QLabel *label_3;
    QDateEdit *fromDate;
    QLabel *label_2;
    QDateEdit *untilDate;
    QPushButton *allMessagesButton;
    QTreeView *messages;
    QDockWidget *messageFrame;
    QWidget *dockWidgetContents;
    QHBoxLayout *hboxLayout1;
    QLabel *label;
    QComboBox *typeFilter;
    QLabel *label_4;
    QComboBox *contextFilter;
    QVBoxLayout *vboxLayout1;
    QGroupBox *groupBox;
    QGridLayout *gridLayout2;
    QListView *developers;
    QPushButton *allDevelopersButton;
    QPushButton *exitButton;

    void setupUi(QWidget *MessageHistory)
    {
    MessageHistory->setObjectName(QString::fromUtf8("MessageHistory"));
    MessageHistory->resize(QSize(712, 616).expandedTo(MessageHistory->minimumSizeHint()));
    gridLayout = new QGridLayout(MessageHistory);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(9);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    groupBox_2 = new QGroupBox(MessageHistory);
    groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
    gridLayout1 = new QGridLayout(groupBox_2);
    gridLayout1->setSpacing(6);
    gridLayout1->setMargin(9);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    vboxLayout = new QVBoxLayout();
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(0);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    label_3 = new QLabel(groupBox_2);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
    label_3->setSizePolicy(sizePolicy);

    hboxLayout->addWidget(label_3);

    fromDate = new QDateEdit(groupBox_2);
    fromDate->setObjectName(QString::fromUtf8("fromDate"));

    hboxLayout->addWidget(fromDate);

    label_2 = new QLabel(groupBox_2);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
    label_2->setSizePolicy(sizePolicy1);

    hboxLayout->addWidget(label_2);

    untilDate = new QDateEdit(groupBox_2);
    untilDate->setObjectName(QString::fromUtf8("untilDate"));
    untilDate->setCurrentSection(QDateTimeEdit::DaySection);

    hboxLayout->addWidget(untilDate);

    allMessagesButton = new QPushButton(groupBox_2);
    allMessagesButton->setObjectName(QString::fromUtf8("allMessagesButton"));

    hboxLayout->addWidget(allMessagesButton);


    vboxLayout->addLayout(hboxLayout);

    messages = new QTreeView(groupBox_2);
    messages->setObjectName(QString::fromUtf8("messages"));

    vboxLayout->addWidget(messages);

    messageFrame = new QDockWidget(groupBox_2);
    messageFrame->setObjectName(QString::fromUtf8("messageFrame"));
    messageFrame->setContextMenuPolicy(Qt::NoContextMenu);
    messageFrame->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable|QDockWidget::NoDockWidgetFeatures);
    dockWidgetContents = new QWidget(messageFrame);
    dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
    messageFrame->setWidget(dockWidgetContents);

    vboxLayout->addWidget(messageFrame);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    label = new QLabel(groupBox_2);
    label->setObjectName(QString::fromUtf8("label"));
    QSizePolicy sizePolicy2(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
    label->setSizePolicy(sizePolicy2);

    hboxLayout1->addWidget(label);

    typeFilter = new QComboBox(groupBox_2);
    typeFilter->setObjectName(QString::fromUtf8("typeFilter"));

    hboxLayout1->addWidget(typeFilter);

    label_4 = new QLabel(groupBox_2);
    label_4->setObjectName(QString::fromUtf8("label_4"));
    QSizePolicy sizePolicy3(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
    label_4->setSizePolicy(sizePolicy3);

    hboxLayout1->addWidget(label_4);

    contextFilter = new QComboBox(groupBox_2);
    contextFilter->setObjectName(QString::fromUtf8("contextFilter"));

    hboxLayout1->addWidget(contextFilter);


    vboxLayout->addLayout(hboxLayout1);


    gridLayout1->addLayout(vboxLayout, 0, 0, 1, 1);


    gridLayout->addWidget(groupBox_2, 0, 0, 1, 1);

    vboxLayout1 = new QVBoxLayout();
    vboxLayout1->setSpacing(6);
    vboxLayout1->setMargin(0);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    groupBox = new QGroupBox(MessageHistory);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    QSizePolicy sizePolicy4(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
    groupBox->setSizePolicy(sizePolicy4);
    groupBox->setMaximumSize(QSize(140, 16777215));
    gridLayout2 = new QGridLayout(groupBox);
    gridLayout2->setSpacing(6);
    gridLayout2->setMargin(9);
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
    developers = new QListView(groupBox);
    developers->setObjectName(QString::fromUtf8("developers"));

    gridLayout2->addWidget(developers, 0, 0, 1, 1);

    allDevelopersButton = new QPushButton(groupBox);
    allDevelopersButton->setObjectName(QString::fromUtf8("allDevelopersButton"));

    gridLayout2->addWidget(allDevelopersButton, 1, 0, 1, 1);


    vboxLayout1->addWidget(groupBox);

    exitButton = new QPushButton(MessageHistory);
    exitButton->setObjectName(QString::fromUtf8("exitButton"));
    exitButton->setMaximumSize(QSize(200, 16777215));

    vboxLayout1->addWidget(exitButton);


    gridLayout->addLayout(vboxLayout1, 0, 1, 1, 1);

    retranslateUi(MessageHistory);
    QObject::connect(allDevelopersButton, SIGNAL(clicked()), developers, SLOT(clearSelection()));

    QMetaObject::connectSlotsByName(MessageHistory);
    } // setupUi

    void retranslateUi(QWidget *MessageHistory)
    {
    MessageHistory->setWindowTitle(QApplication::translate("MessageHistory", "Form", 0, QApplication::UnicodeUTF8));
    groupBox_2->setTitle(QApplication::translate("MessageHistory", "Messages", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("MessageHistory", "From:", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("MessageHistory", "Until:", 0, QApplication::UnicodeUTF8));
    allMessagesButton->setText(QApplication::translate("MessageHistory", "All", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("MessageHistory", "Type:", 0, QApplication::UnicodeUTF8));
    label_4->setText(QApplication::translate("MessageHistory", "Context:", 0, QApplication::UnicodeUTF8));
    groupBox->setTitle(QApplication::translate("MessageHistory", "Developers", 0, QApplication::UnicodeUTF8));
    allDevelopersButton->setText(QApplication::translate("MessageHistory", "All", 0, QApplication::UnicodeUTF8));
    exitButton->setText(QApplication::translate("MessageHistory", "Exit", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(MessageHistory);
    } // retranslateUi

};

namespace Ui {
    class MessageHistory: public Ui_MessageHistory {};
} // namespace Ui

#endif // KDEVTEAMWORK_MESSAGEHISTORY_H
