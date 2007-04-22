#ifndef KDEVTEAMWORK_INTERNALMESSAGE_H
#define KDEVTEAMWORK_INTERNALMESSAGE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QToolButton>
#include <QtGui/QTreeView>
#include <QtGui/QWidget>

class Ui_InternalMessage
{
public:
    QGridLayout *gridLayout;
    QFrame *frame;
    QGridLayout *gridLayout1;
    QToolButton *actionsButton;
    QComboBox *talkingUsers;
    QTreeView *messages;
    QLineEdit *message;

    void setupUi(QWidget *InternalMessage)
    {
    InternalMessage->setObjectName(QString::fromUtf8("InternalMessage"));
    InternalMessage->resize(QSize(463, 193).expandedTo(InternalMessage->minimumSizeHint()));
    gridLayout = new QGridLayout(InternalMessage);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(9);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    frame = new QFrame(InternalMessage);
    frame->setObjectName(QString::fromUtf8("frame"));
    frame->setAutoFillBackground(true);
    frame->setFrameShape(QFrame::Panel);
    frame->setFrameShadow(QFrame::Plain);
    gridLayout1 = new QGridLayout(frame);
    gridLayout1->setSpacing(6);
    gridLayout1->setMargin(9);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    actionsButton = new QToolButton(frame);
    actionsButton->setObjectName(QString::fromUtf8("actionsButton"));

    gridLayout1->addWidget(actionsButton, 0, 0, 1, 1);

    talkingUsers = new QComboBox(frame);
    talkingUsers->setObjectName(QString::fromUtf8("talkingUsers"));

    gridLayout1->addWidget(talkingUsers, 0, 1, 1, 1);

    messages = new QTreeView(frame);
    messages->setObjectName(QString::fromUtf8("messages"));
    messages->setEditTriggers(QAbstractItemView::NoEditTriggers);
    messages->setAlternatingRowColors(true);
    messages->setIndentation(2);

    gridLayout1->addWidget(messages, 1, 0, 1, 2);

    message = new QLineEdit(frame);
    message->setObjectName(QString::fromUtf8("message"));

    gridLayout1->addWidget(message, 2, 0, 1, 2);


    gridLayout->addWidget(frame, 0, 0, 1, 1);

    retranslateUi(InternalMessage);

    QMetaObject::connectSlotsByName(InternalMessage);
    } // setupUi

    void retranslateUi(QWidget *InternalMessage)
    {
    InternalMessage->setWindowTitle(QApplication::translate("InternalMessage", "Internal Message", 0, QApplication::UnicodeUTF8));
    actionsButton->setText(QApplication::translate("InternalMessage", "actions", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(InternalMessage);
    } // retranslateUi

};

namespace Ui {
    class InternalMessage: public Ui_InternalMessage {};
} // namespace Ui

#endif // KDEVTEAMWORK_INTERNALMESSAGE_H
