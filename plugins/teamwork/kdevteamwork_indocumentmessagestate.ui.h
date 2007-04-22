#ifndef KDEVTEAMWORK_INDOCUMENTMESSAGESTATE_H
#define KDEVTEAMWORK_INDOCUMENTMESSAGESTATE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QWidget>
#include "kurllabel.h"

class Ui_InDocumentState
{
public:
    QGridLayout *gridLayout;
    QHBoxLayout *hboxLayout;
    QLabel *label;
    KUrlLabel *file;
    QLabel *label_2;
    QLabel *position;
    QHBoxLayout *hboxLayout1;
    QLabel *label_5;
    KUrlLabel *context;
    QLabel *label_3;
    QLabel *referenceType;
    QFrame *line_2;
    QFrame *line;

    void setupUi(QWidget *InDocumentState)
    {
    InDocumentState->setObjectName(QString::fromUtf8("InDocumentState"));
    InDocumentState->resize(QSize(609, 80).expandedTo(InDocumentState->minimumSizeHint()));
    gridLayout = new QGridLayout(InDocumentState);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(9);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    label = new QLabel(InDocumentState);
    label->setObjectName(QString::fromUtf8("label"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
    label->setSizePolicy(sizePolicy);

    hboxLayout->addWidget(label);

    file = new KUrlLabel(InDocumentState);
    file->setObjectName(QString::fromUtf8("file"));

    hboxLayout->addWidget(file);

    label_2 = new QLabel(InDocumentState);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
    label_2->setSizePolicy(sizePolicy1);

    hboxLayout->addWidget(label_2);

    position = new QLabel(InDocumentState);
    position->setObjectName(QString::fromUtf8("position"));

    hboxLayout->addWidget(position);


    gridLayout->addLayout(hboxLayout, 1, 0, 1, 1);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    label_5 = new QLabel(InDocumentState);
    label_5->setObjectName(QString::fromUtf8("label_5"));
    QSizePolicy sizePolicy2(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
    label_5->setSizePolicy(sizePolicy2);

    hboxLayout1->addWidget(label_5);

    context = new KUrlLabel(InDocumentState);
    context->setObjectName(QString::fromUtf8("context"));

    hboxLayout1->addWidget(context);

    label_3 = new QLabel(InDocumentState);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    QSizePolicy sizePolicy3(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
    label_3->setSizePolicy(sizePolicy3);

    hboxLayout1->addWidget(label_3);

    referenceType = new QLabel(InDocumentState);
    referenceType->setObjectName(QString::fromUtf8("referenceType"));

    hboxLayout1->addWidget(referenceType);


    gridLayout->addLayout(hboxLayout1, 3, 0, 1, 1);

    line_2 = new QFrame(InDocumentState);
    line_2->setObjectName(QString::fromUtf8("line_2"));
    line_2->setFrameShape(QFrame::HLine);

    gridLayout->addWidget(line_2, 2, 0, 1, 1);

    line = new QFrame(InDocumentState);
    line->setObjectName(QString::fromUtf8("line"));
    line->setFrameShape(QFrame::HLine);

    gridLayout->addWidget(line, 0, 0, 1, 1);

    retranslateUi(InDocumentState);

    QMetaObject::connectSlotsByName(InDocumentState);
    } // setupUi

    void retranslateUi(QWidget *InDocumentState)
    {
    InDocumentState->setWindowTitle(QApplication::translate("InDocumentState", "Form", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("InDocumentState", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">File</span>:</p></body></html>", 0, QApplication::UnicodeUTF8));
    file->setText(QApplication::translate("InDocumentState", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration: underline;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" text-decoration:none;\">file</span></p></body></html>", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("InDocumentState", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Sans;\"><span style=\" font-weight:600;\">Position</span>: </p></body></html>", 0, QApplication::UnicodeUTF8));
    position->setText(QApplication::translate("InDocumentState", "position", 0, QApplication::UnicodeUTF8));
    label_5->setText(QApplication::translate("InDocumentState", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Sans;\"><span style=\" font-weight:600;\">Context</span>: </p></body></html>", 0, QApplication::UnicodeUTF8));
    context->setText(QApplication::translate("InDocumentState", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration: underline;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" text-decoration:none;\">Context</span></p></body></html>", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("InDocumentState", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">Reference-Type</span>: </p></body></html>", 0, QApplication::UnicodeUTF8));
    referenceType->setText(QApplication::translate("InDocumentState", "referenceType", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(InDocumentState);
    } // retranslateUi

};

namespace Ui {
    class InDocumentState: public Ui_InDocumentState {};
} // namespace Ui

#endif // KDEVTEAMWORK_INDOCUMENTMESSAGESTATE_H
