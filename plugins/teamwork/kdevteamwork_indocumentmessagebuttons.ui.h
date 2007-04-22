#ifndef KDEVTEAMWORK_INDOCUMENTMESSAGEBUTTONS_H
#define KDEVTEAMWORK_INDOCUMENTMESSAGEBUTTONS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

class Ui_InDocumentButtons
{
public:
    QGridLayout *gridLayout;
    QPushButton *jumpToPosition;

    void setupUi(QWidget *InDocumentButtons)
    {
    InDocumentButtons->setObjectName(QString::fromUtf8("InDocumentButtons"));
    InDocumentButtons->resize(QSize(333, 51).expandedTo(InDocumentButtons->minimumSizeHint()));
    gridLayout = new QGridLayout(InDocumentButtons);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(9);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    jumpToPosition = new QPushButton(InDocumentButtons);
    jumpToPosition->setObjectName(QString::fromUtf8("jumpToPosition"));

    gridLayout->addWidget(jumpToPosition, 0, 0, 1, 1);

    retranslateUi(InDocumentButtons);

    QMetaObject::connectSlotsByName(InDocumentButtons);
    } // setupUi

    void retranslateUi(QWidget *InDocumentButtons)
    {
    InDocumentButtons->setWindowTitle(QApplication::translate("InDocumentButtons", "Form", 0, QApplication::UnicodeUTF8));
    jumpToPosition->setText(QApplication::translate("InDocumentButtons", "Jump to Position", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(InDocumentButtons);
    } // retranslateUi

};

namespace Ui {
    class InDocumentButtons: public Ui_InDocumentButtons {};
} // namespace Ui

#endif // KDEVTEAMWORK_INDOCUMENTMESSAGEBUTTONS_H
