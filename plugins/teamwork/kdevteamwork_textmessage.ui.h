#ifndef KDEVTEAMWORK_TEXTMESSAGE_H
#define KDEVTEAMWORK_TEXTMESSAGE_H

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

class Ui_TextMessageShower
{
public:
    QGridLayout *gridLayout;
    QFrame *bottomSpace;
    QFrame *frame_2;
    QGridLayout *gridLayout1;
    QHBoxLayout *hboxLayout;
    QPushButton *menuButton;
    QPushButton *historyButton;
    QSpacerItem *spacerItem;
    QFrame *buttonSpace;
    QPushButton *isReplyToButton;
    QFrame *topSpace;
    QGroupBox *textBox;
    QGridLayout *gridLayout2;
    QLabel *text;
    QFrame *middleSpace;
    QFrame *frame;
    QGridLayout *gridLayout3;
    QHBoxLayout *hboxLayout1;
    QLabel *state_2;
    QLabel *state;
    QHBoxLayout *hboxLayout2;
    QLabel *time_2;
    QLabel *time;
    QLabel *user_2;
    QLabel *userIcon;
    QLabel *user;
    QLabel *direction_2;
    QLabel *direction;
    QFrame *stateSpace;

    void setupUi(QWidget *TextMessageShower)
    {
    TextMessageShower->setObjectName(QString::fromUtf8("TextMessageShower"));
    TextMessageShower->resize(QSize(582, 173).expandedTo(TextMessageShower->minimumSizeHint()));
    gridLayout = new QGridLayout(TextMessageShower);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(9);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    bottomSpace = new QFrame(TextMessageShower);
    bottomSpace->setObjectName(QString::fromUtf8("bottomSpace"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(5), static_cast<QSizePolicy::Policy>(4));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(bottomSpace->sizePolicy().hasHeightForWidth());
    bottomSpace->setSizePolicy(sizePolicy);
    bottomSpace->setMinimumSize(QSize(16, 0));
    bottomSpace->setFrameShape(QFrame::NoFrame);
    bottomSpace->setFrameShadow(QFrame::Raised);

    gridLayout->addWidget(bottomSpace, 4, 0, 1, 1);

    frame_2 = new QFrame(TextMessageShower);
    frame_2->setObjectName(QString::fromUtf8("frame_2"));
    frame_2->setSizeIncrement(QSize(0, 0));
    frame_2->setFrameShape(QFrame::StyledPanel);
    frame_2->setFrameShadow(QFrame::Raised);
    frame_2->setLineWidth(1);
    gridLayout1 = new QGridLayout(frame_2);
    gridLayout1->setSpacing(6);
    gridLayout1->setMargin(2);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    menuButton = new QPushButton(frame_2);
    menuButton->setObjectName(QString::fromUtf8("menuButton"));

    hboxLayout->addWidget(menuButton);

    historyButton = new QPushButton(frame_2);
    historyButton->setObjectName(QString::fromUtf8("historyButton"));

    hboxLayout->addWidget(historyButton);

    spacerItem = new QSpacerItem(331, 27, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addItem(spacerItem);

    buttonSpace = new QFrame(frame_2);
    buttonSpace->setObjectName(QString::fromUtf8("buttonSpace"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(buttonSpace->sizePolicy().hasHeightForWidth());
    buttonSpace->setSizePolicy(sizePolicy1);
    buttonSpace->setMinimumSize(QSize(0, 0));
    buttonSpace->setFrameShape(QFrame::NoFrame);
    buttonSpace->setFrameShadow(QFrame::Raised);

    hboxLayout->addWidget(buttonSpace);

    isReplyToButton = new QPushButton(frame_2);
    isReplyToButton->setObjectName(QString::fromUtf8("isReplyToButton"));

    hboxLayout->addWidget(isReplyToButton);


    gridLayout1->addLayout(hboxLayout, 0, 0, 1, 1);


    gridLayout->addWidget(frame_2, 5, 0, 1, 1);

    topSpace = new QFrame(TextMessageShower);
    topSpace->setObjectName(QString::fromUtf8("topSpace"));
    topSpace->setEnabled(true);
    QSizePolicy sizePolicy2(static_cast<QSizePolicy::Policy>(5), static_cast<QSizePolicy::Policy>(4));
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(topSpace->sizePolicy().hasHeightForWidth());
    topSpace->setSizePolicy(sizePolicy2);
    topSpace->setMinimumSize(QSize(16, 0));
    topSpace->setFrameShape(QFrame::NoFrame);
    topSpace->setFrameShadow(QFrame::Raised);

    gridLayout->addWidget(topSpace, 0, 0, 1, 1);

    textBox = new QGroupBox(TextMessageShower);
    textBox->setObjectName(QString::fromUtf8("textBox"));
    QFont font;
    font.setFamily(QString::fromUtf8("Sans Serif"));
    font.setPointSize(9);
    font.setBold(true);
    font.setItalic(false);
    font.setUnderline(false);
    font.setWeight(75);
    font.setStrikeOut(false);
    textBox->setFont(font);
    gridLayout2 = new QGridLayout(textBox);
    gridLayout2->setSpacing(6);
    gridLayout2->setMargin(2);
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
    text = new QLabel(textBox);
    text->setObjectName(QString::fromUtf8("text"));
    QFont font1;
    font1.setFamily(QString::fromUtf8("Sans Serif"));
    font1.setPointSize(9);
    font1.setBold(false);
    font1.setItalic(false);
    font1.setUnderline(false);
    font1.setWeight(50);
    font1.setStrikeOut(false);
    text->setFont(font1);

    gridLayout2->addWidget(text, 0, 0, 1, 1);


    gridLayout->addWidget(textBox, 1, 0, 1, 1);

    middleSpace = new QFrame(TextMessageShower);
    middleSpace->setObjectName(QString::fromUtf8("middleSpace"));
    QSizePolicy sizePolicy3(static_cast<QSizePolicy::Policy>(5), static_cast<QSizePolicy::Policy>(4));
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(middleSpace->sizePolicy().hasHeightForWidth());
    middleSpace->setSizePolicy(sizePolicy3);
    middleSpace->setMinimumSize(QSize(16, 0));
    middleSpace->setFrameShape(QFrame::NoFrame);
    middleSpace->setFrameShadow(QFrame::Raised);

    gridLayout->addWidget(middleSpace, 2, 0, 1, 1);

    frame = new QFrame(TextMessageShower);
    frame->setObjectName(QString::fromUtf8("frame"));
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Raised);
    gridLayout3 = new QGridLayout(frame);
    gridLayout3->setSpacing(6);
    gridLayout3->setMargin(2);
    gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));
    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    state_2 = new QLabel(frame);
    state_2->setObjectName(QString::fromUtf8("state_2"));
    QSizePolicy sizePolicy4(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(state_2->sizePolicy().hasHeightForWidth());
    state_2->setSizePolicy(sizePolicy4);

    hboxLayout1->addWidget(state_2);

    state = new QLabel(frame);
    state->setObjectName(QString::fromUtf8("state"));

    hboxLayout1->addWidget(state);


    gridLayout3->addLayout(hboxLayout1, 1, 0, 1, 1);

    hboxLayout2 = new QHBoxLayout();
    hboxLayout2->setSpacing(6);
    hboxLayout2->setMargin(0);
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    time_2 = new QLabel(frame);
    time_2->setObjectName(QString::fromUtf8("time_2"));
    QSizePolicy sizePolicy5(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy5.setHorizontalStretch(0);
    sizePolicy5.setVerticalStretch(0);
    sizePolicy5.setHeightForWidth(time_2->sizePolicy().hasHeightForWidth());
    time_2->setSizePolicy(sizePolicy5);

    hboxLayout2->addWidget(time_2);

    time = new QLabel(frame);
    time->setObjectName(QString::fromUtf8("time"));

    hboxLayout2->addWidget(time);

    user_2 = new QLabel(frame);
    user_2->setObjectName(QString::fromUtf8("user_2"));
    QSizePolicy sizePolicy6(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy6.setHorizontalStretch(0);
    sizePolicy6.setVerticalStretch(0);
    sizePolicy6.setHeightForWidth(user_2->sizePolicy().hasHeightForWidth());
    user_2->setSizePolicy(sizePolicy6);

    hboxLayout2->addWidget(user_2);

    userIcon = new QLabel(frame);
    userIcon->setObjectName(QString::fromUtf8("userIcon"));
    QSizePolicy sizePolicy7(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy7.setHorizontalStretch(0);
    sizePolicy7.setVerticalStretch(0);
    sizePolicy7.setHeightForWidth(userIcon->sizePolicy().hasHeightForWidth());
    userIcon->setSizePolicy(sizePolicy7);

    hboxLayout2->addWidget(userIcon);

    user = new QLabel(frame);
    user->setObjectName(QString::fromUtf8("user"));

    hboxLayout2->addWidget(user);

    direction_2 = new QLabel(frame);
    direction_2->setObjectName(QString::fromUtf8("direction_2"));
    QSizePolicy sizePolicy8(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy8.setHorizontalStretch(0);
    sizePolicy8.setVerticalStretch(0);
    sizePolicy8.setHeightForWidth(direction_2->sizePolicy().hasHeightForWidth());
    direction_2->setSizePolicy(sizePolicy8);

    hboxLayout2->addWidget(direction_2);

    direction = new QLabel(frame);
    direction->setObjectName(QString::fromUtf8("direction"));

    hboxLayout2->addWidget(direction);


    gridLayout3->addLayout(hboxLayout2, 0, 0, 1, 1);

    stateSpace = new QFrame(frame);
    stateSpace->setObjectName(QString::fromUtf8("stateSpace"));
    QSizePolicy sizePolicy9(static_cast<QSizePolicy::Policy>(5), static_cast<QSizePolicy::Policy>(4));
    sizePolicy9.setHorizontalStretch(0);
    sizePolicy9.setVerticalStretch(0);
    sizePolicy9.setHeightForWidth(stateSpace->sizePolicy().hasHeightForWidth());
    stateSpace->setSizePolicy(sizePolicy9);
    stateSpace->setMinimumSize(QSize(0, 1));
    stateSpace->setFrameShape(QFrame::NoFrame);
    stateSpace->setFrameShadow(QFrame::Raised);
    stateSpace->setLineWidth(0);

    gridLayout3->addWidget(stateSpace, 2, 0, 1, 1);


    gridLayout->addWidget(frame, 3, 0, 1, 1);

    retranslateUi(TextMessageShower);

    QMetaObject::connectSlotsByName(TextMessageShower);
    } // setupUi

    void retranslateUi(QWidget *TextMessageShower)
    {
    TextMessageShower->setWindowTitle(QApplication::translate("TextMessageShower", "Form", 0, QApplication::UnicodeUTF8));
    menuButton->setText(QApplication::translate("TextMessageShower", "Menu", 0, QApplication::UnicodeUTF8));
    historyButton->setText(QApplication::translate("TextMessageShower", "History", 0, QApplication::UnicodeUTF8));
    isReplyToButton->setText(QApplication::translate("TextMessageShower", "Is Reply To", 0, QApplication::UnicodeUTF8));
    textBox->setTitle(QApplication::translate("TextMessageShower", "Text", 0, QApplication::UnicodeUTF8));
    text->setText(QApplication::translate("TextMessageShower", "Text", 0, QApplication::UnicodeUTF8));
    state_2->setText(QApplication::translate("TextMessageShower", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">State</span>: </p></body></html>", 0, QApplication::UnicodeUTF8));
    state->setText(QApplication::translate("TextMessageShower", "State", 0, QApplication::UnicodeUTF8));
    time_2->setText(QApplication::translate("TextMessageShower", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">Time</span>: </p></body></html>", 0, QApplication::UnicodeUTF8));
    time->setText(QApplication::translate("TextMessageShower", "Time", 0, QApplication::UnicodeUTF8));
    user_2->setText(QApplication::translate("TextMessageShower", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">User</span>:</p></body></html>", 0, QApplication::UnicodeUTF8));
    userIcon->setText(QApplication::translate("TextMessageShower", "", 0, QApplication::UnicodeUTF8));
    user->setText(QApplication::translate("TextMessageShower", "User", 0, QApplication::UnicodeUTF8));
    direction_2->setText(QApplication::translate("TextMessageShower", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">Direction</span>: </p></body></html>", 0, QApplication::UnicodeUTF8));
    direction->setText(QApplication::translate("TextMessageShower", "Direction", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(TextMessageShower);
    } // retranslateUi

};

namespace Ui {
    class TextMessageShower: public Ui_TextMessageShower {};
} // namespace Ui

#endif // KDEVTEAMWORK_TEXTMESSAGE_H
