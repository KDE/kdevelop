#ifndef KDEVTEAMWORK_EDITPATCH_H
#define KDEVTEAMWORK_EDITPATCH_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDockWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListView>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTabWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "kcombobox.h"
#include "kurlrequester.h"

class Ui_EditPatch
{
public:
    QGridLayout *gridLayout;
    QTabWidget *tabWidget;
    QWidget *fileTab;
    QGridLayout *gridLayout1;
    QLabel *mimetype;
    QLabel *label_2;
    KUrlRequester *filename;
    QWidget *commandTab;
    QGridLayout *gridLayout2;
    QHBoxLayout *hboxLayout;
    QLabel *label_9;
    QLineEdit *command;
    QPushButton *commandToFile;
    QGroupBox *filesGroup;
    QGridLayout *gridLayout3;
    QListView *filesList;
    QVBoxLayout *vboxLayout;
    QPushButton *highlightFiles;
    QPushButton *previousHunk;
    QPushButton *nextHunk;
    QHBoxLayout *hboxLayout1;
    QLabel *label_11;
    QLineEdit *author;
    QPushButton *userButton;
    QHBoxLayout *hboxLayout2;
    QPushButton *determineState;
    QLabel *label_10;
    QComboBox *state;
    QLabel *label;
    KComboBox *accessRights;
    QTextEdit *description;
    QLabel *label_6;
    QLineEdit *unapplyCommand;
    QLabel *label_7;
    QLineEdit *applyCommand;
    QLabel *label_5;
    QHBoxLayout *hboxLayout3;
    QLabel *label_8;
    QLineEdit *name;
    QLabel *label_3;
    QLineEdit *type;
    QPushButton *chooseType;
    QLabel *label_4;
    QLineEdit *dependencies;
    QDockWidget *konsoleDock;
    QWidget *dockWidgetContents;
    QHBoxLayout *hboxLayout4;
    QPushButton *applyButton;
    QPushButton *unApplyButton;
    QPushButton *showButton;
    QSpacerItem *spacerItem;
    QPushButton *okButton;

    void setupUi(QDialog *EditPatch)
    {
    EditPatch->setObjectName(QString::fromUtf8("EditPatch"));
    EditPatch->resize(QSize(502, 688).expandedTo(EditPatch->minimumSizeHint()));
    gridLayout = new QGridLayout(EditPatch);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(9);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    tabWidget = new QTabWidget(EditPatch);
    tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
    fileTab = new QWidget();
    fileTab->setObjectName(QString::fromUtf8("fileTab"));
    gridLayout1 = new QGridLayout(fileTab);
    gridLayout1->setSpacing(6);
    gridLayout1->setMargin(9);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    mimetype = new QLabel(fileTab);
    mimetype->setObjectName(QString::fromUtf8("mimetype"));

    gridLayout1->addWidget(mimetype, 1, 0, 1, 2);

    label_2 = new QLabel(fileTab);
    label_2->setObjectName(QString::fromUtf8("label_2"));

    gridLayout1->addWidget(label_2, 0, 0, 1, 1);

    filename = new KUrlRequester(fileTab);
    filename->setObjectName(QString::fromUtf8("filename"));

    gridLayout1->addWidget(filename, 0, 1, 1, 1);

    tabWidget->addTab(fileTab, QApplication::translate("EditPatch", "File", 0, QApplication::UnicodeUTF8));
    commandTab = new QWidget();
    commandTab->setObjectName(QString::fromUtf8("commandTab"));
    gridLayout2 = new QGridLayout(commandTab);
    gridLayout2->setSpacing(6);
    gridLayout2->setMargin(9);
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    label_9 = new QLabel(commandTab);
    label_9->setObjectName(QString::fromUtf8("label_9"));

    hboxLayout->addWidget(label_9);

    command = new QLineEdit(commandTab);
    command->setObjectName(QString::fromUtf8("command"));

    hboxLayout->addWidget(command);

    commandToFile = new QPushButton(commandTab);
    commandToFile->setObjectName(QString::fromUtf8("commandToFile"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(0));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(commandToFile->sizePolicy().hasHeightForWidth());
    commandToFile->setSizePolicy(sizePolicy);

    hboxLayout->addWidget(commandToFile);


    gridLayout2->addLayout(hboxLayout, 0, 0, 1, 1);

    tabWidget->addTab(commandTab, QApplication::translate("EditPatch", "command-output", 0, QApplication::UnicodeUTF8));

    gridLayout->addWidget(tabWidget, 0, 0, 1, 1);

    filesGroup = new QGroupBox(EditPatch);
    filesGroup->setObjectName(QString::fromUtf8("filesGroup"));
    gridLayout3 = new QGridLayout(filesGroup);
    gridLayout3->setSpacing(4);
    gridLayout3->setMargin(2);
    gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));
    filesList = new QListView(filesGroup);
    filesList->setObjectName(QString::fromUtf8("filesList"));
    filesList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    gridLayout3->addWidget(filesList, 0, 1, 1, 1);

    vboxLayout = new QVBoxLayout();
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(0);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    highlightFiles = new QPushButton(filesGroup);
    highlightFiles->setObjectName(QString::fromUtf8("highlightFiles"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(5), static_cast<QSizePolicy::Policy>(0));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(highlightFiles->sizePolicy().hasHeightForWidth());
    highlightFiles->setSizePolicy(sizePolicy1);

    vboxLayout->addWidget(highlightFiles);

    previousHunk = new QPushButton(filesGroup);
    previousHunk->setObjectName(QString::fromUtf8("previousHunk"));
    QSizePolicy sizePolicy2(static_cast<QSizePolicy::Policy>(5), static_cast<QSizePolicy::Policy>(0));
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(previousHunk->sizePolicy().hasHeightForWidth());
    previousHunk->setSizePolicy(sizePolicy2);

    vboxLayout->addWidget(previousHunk);

    nextHunk = new QPushButton(filesGroup);
    nextHunk->setObjectName(QString::fromUtf8("nextHunk"));
    QSizePolicy sizePolicy3(static_cast<QSizePolicy::Policy>(5), static_cast<QSizePolicy::Policy>(0));
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(nextHunk->sizePolicy().hasHeightForWidth());
    nextHunk->setSizePolicy(sizePolicy3);

    vboxLayout->addWidget(nextHunk);


    gridLayout3->addLayout(vboxLayout, 0, 0, 1, 1);


    gridLayout->addWidget(filesGroup, 12, 0, 1, 1);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    label_11 = new QLabel(EditPatch);
    label_11->setObjectName(QString::fromUtf8("label_11"));

    hboxLayout1->addWidget(label_11);

    author = new QLineEdit(EditPatch);
    author->setObjectName(QString::fromUtf8("author"));

    hboxLayout1->addWidget(author);

    userButton = new QPushButton(EditPatch);
    userButton->setObjectName(QString::fromUtf8("userButton"));

    hboxLayout1->addWidget(userButton);


    gridLayout->addLayout(hboxLayout1, 10, 0, 1, 1);

    hboxLayout2 = new QHBoxLayout();
    hboxLayout2->setSpacing(6);
    hboxLayout2->setMargin(0);
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    determineState = new QPushButton(EditPatch);
    determineState->setObjectName(QString::fromUtf8("determineState"));
    QSizePolicy sizePolicy4(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(0));
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(determineState->sizePolicy().hasHeightForWidth());
    determineState->setSizePolicy(sizePolicy4);

    hboxLayout2->addWidget(determineState);

    label_10 = new QLabel(EditPatch);
    label_10->setObjectName(QString::fromUtf8("label_10"));
    QSizePolicy sizePolicy5(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy5.setHorizontalStretch(0);
    sizePolicy5.setVerticalStretch(0);
    sizePolicy5.setHeightForWidth(label_10->sizePolicy().hasHeightForWidth());
    label_10->setSizePolicy(sizePolicy5);

    hboxLayout2->addWidget(label_10);

    state = new QComboBox(EditPatch);
    state->setObjectName(QString::fromUtf8("state"));

    hboxLayout2->addWidget(state);

    label = new QLabel(EditPatch);
    label->setObjectName(QString::fromUtf8("label"));
    QSizePolicy sizePolicy6(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy6.setHorizontalStretch(0);
    sizePolicy6.setVerticalStretch(0);
    sizePolicy6.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
    label->setSizePolicy(sizePolicy6);

    hboxLayout2->addWidget(label);

    accessRights = new KComboBox(EditPatch);
    accessRights->setObjectName(QString::fromUtf8("accessRights"));

    hboxLayout2->addWidget(accessRights);


    gridLayout->addLayout(hboxLayout2, 11, 0, 1, 1);

    description = new QTextEdit(EditPatch);
    description->setObjectName(QString::fromUtf8("description"));

    gridLayout->addWidget(description, 9, 0, 1, 1);

    label_6 = new QLabel(EditPatch);
    label_6->setObjectName(QString::fromUtf8("label_6"));

    gridLayout->addWidget(label_6, 4, 0, 1, 1);

    unapplyCommand = new QLineEdit(EditPatch);
    unapplyCommand->setObjectName(QString::fromUtf8("unapplyCommand"));

    gridLayout->addWidget(unapplyCommand, 5, 0, 1, 1);

    label_7 = new QLabel(EditPatch);
    label_7->setObjectName(QString::fromUtf8("label_7"));

    gridLayout->addWidget(label_7, 6, 0, 1, 1);

    applyCommand = new QLineEdit(EditPatch);
    applyCommand->setObjectName(QString::fromUtf8("applyCommand"));

    gridLayout->addWidget(applyCommand, 3, 0, 1, 1);

    label_5 = new QLabel(EditPatch);
    label_5->setObjectName(QString::fromUtf8("label_5"));

    gridLayout->addWidget(label_5, 2, 0, 1, 1);

    hboxLayout3 = new QHBoxLayout();
    hboxLayout3->setSpacing(6);
    hboxLayout3->setMargin(0);
    hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
    label_8 = new QLabel(EditPatch);
    label_8->setObjectName(QString::fromUtf8("label_8"));

    hboxLayout3->addWidget(label_8);

    name = new QLineEdit(EditPatch);
    name->setObjectName(QString::fromUtf8("name"));

    hboxLayout3->addWidget(name);

    label_3 = new QLabel(EditPatch);
    label_3->setObjectName(QString::fromUtf8("label_3"));

    hboxLayout3->addWidget(label_3);

    type = new QLineEdit(EditPatch);
    type->setObjectName(QString::fromUtf8("type"));

    hboxLayout3->addWidget(type);

    chooseType = new QPushButton(EditPatch);
    chooseType->setObjectName(QString::fromUtf8("chooseType"));

    hboxLayout3->addWidget(chooseType);


    gridLayout->addLayout(hboxLayout3, 1, 0, 1, 1);

    label_4 = new QLabel(EditPatch);
    label_4->setObjectName(QString::fromUtf8("label_4"));

    gridLayout->addWidget(label_4, 8, 0, 1, 1);

    dependencies = new QLineEdit(EditPatch);
    dependencies->setObjectName(QString::fromUtf8("dependencies"));

    gridLayout->addWidget(dependencies, 7, 0, 1, 1);

    konsoleDock = new QDockWidget(EditPatch);
    konsoleDock->setObjectName(QString::fromUtf8("konsoleDock"));
    konsoleDock->setFeatures(QDockWidget::AllDockWidgetFeatures);
    dockWidgetContents = new QWidget(konsoleDock);
    dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
    konsoleDock->setWidget(dockWidgetContents);

    gridLayout->addWidget(konsoleDock, 13, 0, 1, 1);

    hboxLayout4 = new QHBoxLayout();
    hboxLayout4->setSpacing(6);
    hboxLayout4->setMargin(0);
    hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));
    applyButton = new QPushButton(EditPatch);
    applyButton->setObjectName(QString::fromUtf8("applyButton"));

    hboxLayout4->addWidget(applyButton);

    unApplyButton = new QPushButton(EditPatch);
    unApplyButton->setObjectName(QString::fromUtf8("unApplyButton"));

    hboxLayout4->addWidget(unApplyButton);

    showButton = new QPushButton(EditPatch);
    showButton->setObjectName(QString::fromUtf8("showButton"));

    hboxLayout4->addWidget(showButton);

    spacerItem = new QSpacerItem(40, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout4->addItem(spacerItem);

    okButton = new QPushButton(EditPatch);
    okButton->setObjectName(QString::fromUtf8("okButton"));

    hboxLayout4->addWidget(okButton);


    gridLayout->addLayout(hboxLayout4, 14, 0, 1, 1);

    retranslateUi(EditPatch);
    QObject::connect(okButton, SIGNAL(clicked()), EditPatch, SLOT(accept()));

    tabWidget->setCurrentIndex(1);


    QMetaObject::connectSlotsByName(EditPatch);
    } // setupUi

    void retranslateUi(QDialog *EditPatch)
    {
    EditPatch->setWindowTitle(QApplication::translate("EditPatch", "Edit Patch", 0, QApplication::UnicodeUTF8));
    mimetype->setText(QApplication::translate("EditPatch", "", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("EditPatch", "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">File:</span></p></body></html>", 0, QApplication::UnicodeUTF8));
    tabWidget->setTabText(tabWidget->indexOf(fileTab), QApplication::translate("EditPatch", "File", 0, QApplication::UnicodeUTF8));
    label_9->setText(QApplication::translate("EditPatch", "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">Command:</span></p></body></html>", 0, QApplication::UnicodeUTF8));
    commandToFile->setText(QApplication::translate("EditPatch", "To File", 0, QApplication::UnicodeUTF8));
    tabWidget->setTabText(tabWidget->indexOf(commandTab), QApplication::translate("EditPatch", "command-output", 0, QApplication::UnicodeUTF8));
    filesGroup->setTitle(QApplication::translate("EditPatch", "Files", 0, QApplication::UnicodeUTF8));
    highlightFiles->setText(QApplication::translate("EditPatch", "Highlight", 0, QApplication::UnicodeUTF8));
    previousHunk->setText(QApplication::translate("EditPatch", "Previous Hunk", 0, QApplication::UnicodeUTF8));
    nextHunk->setText(QApplication::translate("EditPatch", "Next Hunk", 0, QApplication::UnicodeUTF8));
    label_11->setText(QApplication::translate("EditPatch", "Author:", 0, QApplication::UnicodeUTF8));
    userButton->setText(QApplication::translate("EditPatch", "User", 0, QApplication::UnicodeUTF8));
    determineState->setText(QApplication::translate("EditPatch", "Determine", 0, QApplication::UnicodeUTF8));
    label_10->setText(QApplication::translate("EditPatch", "State:", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("EditPatch", "Access:", 0, QApplication::UnicodeUTF8));
    label_6->setText(QApplication::translate("EditPatch", "recommended unapply-command:", 0, QApplication::UnicodeUTF8));
    label_7->setText(QApplication::translate("EditPatch", "Dependencies:", 0, QApplication::UnicodeUTF8));
    label_5->setText(QApplication::translate("EditPatch", "recommended apply-command:", 0, QApplication::UnicodeUTF8));
    label_8->setText(QApplication::translate("EditPatch", "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">N<span style=\" font-weight:600;\">ame:</span></p></body></html>", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("EditPatch", "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">Type:</span></p></body></html>", 0, QApplication::UnicodeUTF8));
    chooseType->setText(QApplication::translate("EditPatch", "Choose", 0, QApplication::UnicodeUTF8));
    label_4->setText(QApplication::translate("EditPatch", "Description:", 0, QApplication::UnicodeUTF8));
    applyButton->setText(QApplication::translate("EditPatch", "Apply", 0, QApplication::UnicodeUTF8));
    unApplyButton->setText(QApplication::translate("EditPatch", "Unapply", 0, QApplication::UnicodeUTF8));
    showButton->setText(QApplication::translate("EditPatch", "Show", 0, QApplication::UnicodeUTF8));
    okButton->setText(QApplication::translate("EditPatch", "Close", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(EditPatch);
    } // retranslateUi

};

namespace Ui {
    class EditPatch: public Ui_EditPatch {};
} // namespace Ui

#endif // KDEVTEAMWORK_EDITPATCH_H
