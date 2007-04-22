#ifndef KDEVTEAMWORK_INDOCUMENTCONTEXTLINES_H
#define KDEVTEAMWORK_INDOCUMENTCONTEXTLINES_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

class Ui_InDocumentContextLines
{
public:
    QGridLayout *gridLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout1;
    QTextEdit *contextLines;

    void setupUi(QWidget *InDocumentContextLines)
    {
    InDocumentContextLines->setObjectName(QString::fromUtf8("InDocumentContextLines"));
    InDocumentContextLines->resize(QSize(611, 137).expandedTo(InDocumentContextLines->minimumSizeHint()));
    gridLayout = new QGridLayout(InDocumentContextLines);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(9);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    groupBox = new QGroupBox(InDocumentContextLines);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    QFont font;
    font.setFamily(QString::fromUtf8("Sans Serif"));
    font.setPointSize(9);
    font.setBold(true);
    font.setItalic(false);
    font.setUnderline(false);
    font.setWeight(75);
    font.setStrikeOut(false);
    groupBox->setFont(font);
    gridLayout1 = new QGridLayout(groupBox);
    gridLayout1->setSpacing(6);
    gridLayout1->setMargin(2);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    contextLines = new QTextEdit(groupBox);
    contextLines->setObjectName(QString::fromUtf8("contextLines"));

    gridLayout1->addWidget(contextLines, 0, 0, 1, 1);


    gridLayout->addWidget(groupBox, 0, 0, 1, 1);

    retranslateUi(InDocumentContextLines);

    QMetaObject::connectSlotsByName(InDocumentContextLines);
    } // setupUi

    void retranslateUi(QWidget *InDocumentContextLines)
    {
    InDocumentContextLines->setWindowTitle(QApplication::translate("InDocumentContextLines", "Form", 0, QApplication::UnicodeUTF8));
    groupBox->setTitle(QApplication::translate("InDocumentContextLines", "Context-Lines:", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(InDocumentContextLines);
    } // retranslateUi

};

namespace Ui {
    class InDocumentContextLines: public Ui_InDocumentContextLines {};
} // namespace Ui

#endif // KDEVTEAMWORK_INDOCUMENTCONTEXTLINES_H
