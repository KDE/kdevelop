#ifndef KDEVTEAMWORK_USERINFO_H
#define KDEVTEAMWORK_USERINFO_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>

class Ui_UserInfo
{
public:
    QGridLayout *gridLayout;
    QFrame *frame;
    QGridLayout *gridLayout1;
    QHBoxLayout *hboxLayout;
    QPushButton *menuButton;
    QSpacerItem *spacerItem;
    QPushButton *historyButton;
    QGroupBox *groupBox;
    QGridLayout *gridLayout2;
    QLabel *description;
    QFrame *frame_2;
    QGridLayout *gridLayout3;
    QHBoxLayout *hboxLayout1;
    QLabel *icon;
    QLabel *name_2;
    QLabel *name;
    QLabel *email_2;
    QLabel *email;
    QGridLayout *gridLayout4;
    QLabel *state_2;
    QLabel *ipadress;
    QLabel *state;
    QLabel *ipadress_2;
    QHBoxLayout *hboxLayout2;
    QLabel *rights_2;
    QLabel *rights;

    void setupUi(QWidget *UserInfo)
    {
    UserInfo->setObjectName(QString::fromUtf8("UserInfo"));
    UserInfo->resize(QSize(419, 202).expandedTo(UserInfo->minimumSizeHint()));
    gridLayout = new QGridLayout(UserInfo);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(9);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    frame = new QFrame(UserInfo);
    frame->setObjectName(QString::fromUtf8("frame"));
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Raised);
    gridLayout1 = new QGridLayout(frame);
    gridLayout1->setSpacing(6);
    gridLayout1->setMargin(1);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    menuButton = new QPushButton(frame);
    menuButton->setObjectName(QString::fromUtf8("menuButton"));

    hboxLayout->addWidget(menuButton);

    spacerItem = new QSpacerItem(121, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addItem(spacerItem);

    historyButton = new QPushButton(frame);
    historyButton->setObjectName(QString::fromUtf8("historyButton"));

    hboxLayout->addWidget(historyButton);


    gridLayout1->addLayout(hboxLayout, 0, 0, 1, 1);


    gridLayout->addWidget(frame, 2, 0, 1, 1);

    groupBox = new QGroupBox(UserInfo);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(5), static_cast<QSizePolicy::Policy>(5));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
    groupBox->setSizePolicy(sizePolicy);
    QFont font;
    font.setFamily(QString::fromUtf8("Sans Serif"));
    font.setPointSize(9);
    font.setBold(true);
    font.setItalic(false);
    font.setUnderline(false);
    font.setWeight(75);
    font.setStrikeOut(false);
    groupBox->setFont(font);
    gridLayout2 = new QGridLayout(groupBox);
    gridLayout2->setSpacing(6);
    gridLayout2->setMargin(9);
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
    description = new QLabel(groupBox);
    description->setObjectName(QString::fromUtf8("description"));
    QFont font1;
    font1.setFamily(QString::fromUtf8("Sans Serif"));
    font1.setPointSize(9);
    font1.setBold(false);
    font1.setItalic(false);
    font1.setUnderline(false);
    font1.setWeight(50);
    font1.setStrikeOut(false);
    description->setFont(font1);
    description->setWordWrap(true);

    gridLayout2->addWidget(description, 0, 0, 1, 1);


    gridLayout->addWidget(groupBox, 1, 0, 1, 1);

    frame_2 = new QFrame(UserInfo);
    frame_2->setObjectName(QString::fromUtf8("frame_2"));
    frame_2->setFrameShape(QFrame::StyledPanel);
    frame_2->setFrameShadow(QFrame::Raised);
    gridLayout3 = new QGridLayout(frame_2);
    gridLayout3->setSpacing(6);
    gridLayout3->setMargin(9);
    gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));
    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    icon = new QLabel(frame_2);
    icon->setObjectName(QString::fromUtf8("icon"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(icon->sizePolicy().hasHeightForWidth());
    icon->setSizePolicy(sizePolicy1);

    hboxLayout1->addWidget(icon);

    name_2 = new QLabel(frame_2);
    name_2->setObjectName(QString::fromUtf8("name_2"));
    QSizePolicy sizePolicy2(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(name_2->sizePolicy().hasHeightForWidth());
    name_2->setSizePolicy(sizePolicy2);

    hboxLayout1->addWidget(name_2);

    name = new QLabel(frame_2);
    name->setObjectName(QString::fromUtf8("name"));

    hboxLayout1->addWidget(name);

    email_2 = new QLabel(frame_2);
    email_2->setObjectName(QString::fromUtf8("email_2"));
    QSizePolicy sizePolicy3(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(email_2->sizePolicy().hasHeightForWidth());
    email_2->setSizePolicy(sizePolicy3);

    hboxLayout1->addWidget(email_2);

    email = new QLabel(frame_2);
    email->setObjectName(QString::fromUtf8("email"));

    hboxLayout1->addWidget(email);


    gridLayout3->addLayout(hboxLayout1, 0, 0, 1, 1);

    gridLayout4 = new QGridLayout();
    gridLayout4->setSpacing(6);
    gridLayout4->setMargin(0);
    gridLayout4->setObjectName(QString::fromUtf8("gridLayout4"));
    state_2 = new QLabel(frame_2);
    state_2->setObjectName(QString::fromUtf8("state_2"));
    QSizePolicy sizePolicy4(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(state_2->sizePolicy().hasHeightForWidth());
    state_2->setSizePolicy(sizePolicy4);

    gridLayout4->addWidget(state_2, 0, 0, 1, 1);

    ipadress = new QLabel(frame_2);
    ipadress->setObjectName(QString::fromUtf8("ipadress"));

    gridLayout4->addWidget(ipadress, 0, 3, 1, 1);

    state = new QLabel(frame_2);
    state->setObjectName(QString::fromUtf8("state"));

    gridLayout4->addWidget(state, 0, 1, 1, 1);

    ipadress_2 = new QLabel(frame_2);
    ipadress_2->setObjectName(QString::fromUtf8("ipadress_2"));
    QSizePolicy sizePolicy5(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy5.setHorizontalStretch(0);
    sizePolicy5.setVerticalStretch(0);
    sizePolicy5.setHeightForWidth(ipadress_2->sizePolicy().hasHeightForWidth());
    ipadress_2->setSizePolicy(sizePolicy5);

    gridLayout4->addWidget(ipadress_2, 0, 2, 1, 1);


    gridLayout3->addLayout(gridLayout4, 1, 0, 1, 1);

    hboxLayout2 = new QHBoxLayout();
    hboxLayout2->setSpacing(6);
    hboxLayout2->setMargin(0);
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    rights_2 = new QLabel(frame_2);
    rights_2->setObjectName(QString::fromUtf8("rights_2"));
    QSizePolicy sizePolicy6(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy6.setHorizontalStretch(0);
    sizePolicy6.setVerticalStretch(0);
    sizePolicy6.setHeightForWidth(rights_2->sizePolicy().hasHeightForWidth());
    rights_2->setSizePolicy(sizePolicy6);

    hboxLayout2->addWidget(rights_2);

    rights = new QLabel(frame_2);
    rights->setObjectName(QString::fromUtf8("rights"));

    hboxLayout2->addWidget(rights);


    gridLayout3->addLayout(hboxLayout2, 2, 0, 1, 1);


    gridLayout->addWidget(frame_2, 0, 0, 1, 1);

    retranslateUi(UserInfo);

    QMetaObject::connectSlotsByName(UserInfo);
    } // setupUi

    void retranslateUi(QWidget *UserInfo)
    {
    UserInfo->setWindowTitle(QApplication::translate("UserInfo", "Form", 0, QApplication::UnicodeUTF8));
    menuButton->setText(QApplication::translate("UserInfo", "Menu", 0, QApplication::UnicodeUTF8));
    historyButton->setText(QApplication::translate("UserInfo", "History", 0, QApplication::UnicodeUTF8));
    groupBox->setTitle(QApplication::translate("UserInfo", "Description", 0, QApplication::UnicodeUTF8));
    description->setText(QApplication::translate("UserInfo", "TextLabel", 0, QApplication::UnicodeUTF8));
    icon->setText(QApplication::translate("UserInfo", "", 0, QApplication::UnicodeUTF8));
    name_2->setText(QApplication::translate("UserInfo", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">Name</span>:</p></body></html>", 0, QApplication::UnicodeUTF8));
    name->setText(QApplication::translate("UserInfo", "Name", 0, QApplication::UnicodeUTF8));
    email_2->setText(QApplication::translate("UserInfo", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">E-Mail</span>:</p></body></html>", 0, QApplication::UnicodeUTF8));
    email->setText(QApplication::translate("UserInfo", "E-Mail", 0, QApplication::UnicodeUTF8));
    state_2->setText(QApplication::translate("UserInfo", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">State</span>:</p></body></html>", 0, QApplication::UnicodeUTF8));
    ipadress->setText(QApplication::translate("UserInfo", "IP-Adress", 0, QApplication::UnicodeUTF8));
    state->setText(QApplication::translate("UserInfo", "State", 0, QApplication::UnicodeUTF8));
    ipadress_2->setText(QApplication::translate("UserInfo", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">IP-Adress</span>:</p></body></html>", 0, QApplication::UnicodeUTF8));
    rights_2->setText(QApplication::translate("UserInfo", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">Rights</span>: </p></body></html>", 0, QApplication::UnicodeUTF8));
    rights->setText(QApplication::translate("UserInfo", "Rights", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(UserInfo);
    } // retranslateUi

};

namespace Ui {
    class UserInfo: public Ui_UserInfo {};
} // namespace Ui

#endif // KDEVTEAMWORK_USERINFO_H
