/***************************************************************************
copyright            : (C) 2006 by David Nolden
email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KDEVTEAMWORK_INTERFACE_H
#define KDEVTEAMWORK_INTERFACE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListView>
#include <QtGui/QPushButton>
#include <QtGui/QTabWidget>
#include <QtGui/QToolBox>
#include <QtGui/QToolButton>
#include <QtGui/QTreeView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <kcombobox.h>
#include <klineedit.h>
#include <ktextedit.h>

class Ui_Teamwork
{
public:
    QToolBox *toolBox;
    QWidget *connection;
    QGridLayout *gridLayout;
    QTabWidget *connectionTab;
    QWidget *localTab;
    QGridLayout *gridLayout1;
    QListView *connectedClients;
    QLabel *connectedClientsLabel;
    QCheckBox *allowIncoming;
    QWidget *remoteTab;
    QGridLayout *gridLayout2;
    QListView *connectedServers;
    KLineEdit *loginPassword;
    QPushButton *connectServer;
    QComboBox *serverBox;
    QLabel *connectedServersLabel;
    KLineEdit *loginName;
    QWidget *developers;
    QGridLayout *gridLayout3;
    QLabel *availableLabel;
    QTreeView *availableDevelopers;
    QTreeView *connectedDevelopers;
    QLabel *connectedLabel;
    QWidget *messaging;
    QGridLayout *gridLayout4;
    KTextEdit *messageText;
    QPushButton *sendMessage;
    QWidget *widget;
    QHBoxLayout *hboxLayout;
    KComboBox *messageTargetUser;
    QLabel *typeLabel;
    QComboBox *messageType;
    QWidget *widget1;
    QHBoxLayout *hboxLayout1;
    QLabel *referenceLabel;
    QLineEdit *reference;
    QLabel *contextLabel;
    QLineEdit *context;
    QWidget *localstate;
    QGridLayout *gridLayout5;
    QPushButton *managePatches;
    QCheckBox *enableCollaboration;
    QTabWidget *historyTab;
    QWidget *messagesTab;
    QWidget *layoutWidget;
    QHBoxLayout *hboxLayout2;
    QPushButton *clearMessages;
    QPushButton *messageHistory;
    QToolButton *messageFilter;
    QListView *messageList;
    QWidget *logTab;
    QGridLayout *gridLayout6;
    QVBoxLayout *vboxLayout;
    QListView *logList;
    QHBoxLayout *hboxLayout3;
    QPushButton *clearLog;
    QPushButton *saveLog;
    QToolButton *logFilter;

    void setupUi(QWidget *Teamwork)
    {
    Teamwork->setObjectName(QString::fromUtf8("Teamwork"));
    Teamwork->resize(QSize(409, 695).expandedTo(Teamwork->minimumSizeHint()));
    toolBox = new QToolBox(Teamwork);
    toolBox->setObjectName(QString::fromUtf8("toolBox"));
    toolBox->setGeometry(QRect(9, 34, 391, 323));
    connection = new QWidget();
    connection->setObjectName(QString::fromUtf8("connection"));
    connection->setGeometry(QRect(0, 0, 96, 26));
    gridLayout = new QGridLayout(connection);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(9);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    connectionTab = new QTabWidget(connection);
    connectionTab->setObjectName(QString::fromUtf8("connectionTab"));
    connectionTab->setEnabled(true);
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(5));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(connectionTab->sizePolicy().hasHeightForWidth());
    connectionTab->setSizePolicy(sizePolicy);
    connectionTab->setMinimumSize(QSize(0, 0));
    connectionTab->setMaximumSize(QSize(16777215, 16777215));
    localTab = new QWidget();
    localTab->setObjectName(QString::fromUtf8("localTab"));
    gridLayout1 = new QGridLayout(localTab);
    gridLayout1->setSpacing(6);
    gridLayout1->setMargin(9);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    connectedClients = new QListView(localTab);
    connectedClients->setObjectName(QString::fromUtf8("connectedClients"));

    gridLayout1->addWidget(connectedClients, 2, 0, 1, 1);

    connectedClientsLabel = new QLabel(localTab);
    connectedClientsLabel->setObjectName(QString::fromUtf8("connectedClientsLabel"));

    gridLayout1->addWidget(connectedClientsLabel, 1, 0, 1, 1);

    allowIncoming = new QCheckBox(localTab);
    allowIncoming->setObjectName(QString::fromUtf8("allowIncoming"));

    gridLayout1->addWidget(allowIncoming, 0, 0, 1, 1);

    connectionTab->addTab(localTab, QApplication::translate("Teamwork", "Local server", 0, QApplication::UnicodeUTF8));
    remoteTab = new QWidget();
    remoteTab->setObjectName(QString::fromUtf8("remoteTab"));
    gridLayout2 = new QGridLayout(remoteTab);
    gridLayout2->setSpacing(6);
    gridLayout2->setMargin(9);
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
    connectedServers = new QListView(remoteTab);
    connectedServers->setObjectName(QString::fromUtf8("connectedServers"));

    gridLayout2->addWidget(connectedServers, 3, 0, 1, 3);

    loginPassword = new KLineEdit(remoteTab);
    loginPassword->setObjectName(QString::fromUtf8("loginPassword"));

    gridLayout2->addWidget(loginPassword, 1, 1, 1, 1);

    connectServer = new QPushButton(remoteTab);
    connectServer->setObjectName(QString::fromUtf8("connectServer"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(connectServer->sizePolicy().hasHeightForWidth());
    connectServer->setSizePolicy(sizePolicy1);

    gridLayout2->addWidget(connectServer, 1, 2, 1, 1);

    serverBox = new QComboBox(remoteTab);
    serverBox->setObjectName(QString::fromUtf8("serverBox"));
    serverBox->setEditable(true);

    gridLayout2->addWidget(serverBox, 0, 0, 1, 3);

    connectedServersLabel = new QLabel(remoteTab);
    connectedServersLabel->setObjectName(QString::fromUtf8("connectedServersLabel"));

    gridLayout2->addWidget(connectedServersLabel, 2, 0, 1, 3);

    loginName = new KLineEdit(remoteTab);
    loginName->setObjectName(QString::fromUtf8("loginName"));

    gridLayout2->addWidget(loginName, 1, 0, 1, 1);

    connectionTab->addTab(remoteTab, QApplication::translate("Teamwork", "Servers", 0, QApplication::UnicodeUTF8));

    gridLayout->addWidget(connectionTab, 0, 0, 1, 1);

    toolBox->addItem(connection, QApplication::translate("Teamwork", "Connection", 0, QApplication::UnicodeUTF8));
    developers = new QWidget();
    developers->setObjectName(QString::fromUtf8("developers"));
    developers->setGeometry(QRect(0, 0, 96, 26));
    gridLayout3 = new QGridLayout(developers);
    gridLayout3->setSpacing(6);
    gridLayout3->setMargin(9);
    gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));
    availableLabel = new QLabel(developers);
    availableLabel->setObjectName(QString::fromUtf8("availableLabel"));

    gridLayout3->addWidget(availableLabel, 0, 0, 1, 1);

    availableDevelopers = new QTreeView(developers);
    availableDevelopers->setObjectName(QString::fromUtf8("availableDevelopers"));
    availableDevelopers->setEnabled(true);
    QSizePolicy sizePolicy2(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(7));
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(availableDevelopers->sizePolicy().hasHeightForWidth());
    availableDevelopers->setSizePolicy(sizePolicy2);
    availableDevelopers->setMaximumSize(QSize(16777215, 16777215));
    availableDevelopers->setSizeIncrement(QSize(0, 0));
    availableDevelopers->setBaseSize(QSize(0, 0));
    availableDevelopers->setLineWidth(1);

    gridLayout3->addWidget(availableDevelopers, 1, 0, 1, 1);

    connectedDevelopers = new QTreeView(developers);
    connectedDevelopers->setObjectName(QString::fromUtf8("connectedDevelopers"));
    QSizePolicy sizePolicy3(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(7));
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(connectedDevelopers->sizePolicy().hasHeightForWidth());
    connectedDevelopers->setSizePolicy(sizePolicy3);
    connectedDevelopers->setMaximumSize(QSize(16777215, 16777215));

    gridLayout3->addWidget(connectedDevelopers, 3, 0, 1, 1);

    connectedLabel = new QLabel(developers);
    connectedLabel->setObjectName(QString::fromUtf8("connectedLabel"));

    gridLayout3->addWidget(connectedLabel, 2, 0, 1, 1);

    toolBox->addItem(developers, QApplication::translate("Teamwork", "Developers", 0, QApplication::UnicodeUTF8));
    messaging = new QWidget();
    messaging->setObjectName(QString::fromUtf8("messaging"));
    messaging->setGeometry(QRect(0, 0, 96, 26));
    gridLayout4 = new QGridLayout(messaging);
    gridLayout4->setSpacing(6);
    gridLayout4->setMargin(9);
    gridLayout4->setObjectName(QString::fromUtf8("gridLayout4"));
    messageText = new KTextEdit(messaging);
    messageText->setObjectName(QString::fromUtf8("messageText"));

    gridLayout4->addWidget(messageText, 2, 0, 1, 1);

    sendMessage = new QPushButton(messaging);
    sendMessage->setObjectName(QString::fromUtf8("sendMessage"));

    gridLayout4->addWidget(sendMessage, 3, 0, 1, 1);

    widget = new QWidget(messaging);
    widget->setObjectName(QString::fromUtf8("widget"));
    hboxLayout = new QHBoxLayout(widget);
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    messageTargetUser = new KComboBox(widget);
    messageTargetUser->setObjectName(QString::fromUtf8("messageTargetUser"));
    messageTargetUser->setEditable(true);

    hboxLayout->addWidget(messageTargetUser);

    typeLabel = new QLabel(widget);
    typeLabel->setObjectName(QString::fromUtf8("typeLabel"));
    QSizePolicy sizePolicy4(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(typeLabel->sizePolicy().hasHeightForWidth());
    typeLabel->setSizePolicy(sizePolicy4);
    typeLabel->setMaximumSize(QSize(50, 16777215));
    typeLabel->setLayoutDirection(Qt::LeftToRight);
    typeLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    hboxLayout->addWidget(typeLabel);

    messageType = new QComboBox(widget);
    messageType->setObjectName(QString::fromUtf8("messageType"));

    hboxLayout->addWidget(messageType);


    gridLayout4->addWidget(widget, 0, 0, 1, 1);

    widget1 = new QWidget(messaging);
    widget1->setObjectName(QString::fromUtf8("widget1"));
    hboxLayout1 = new QHBoxLayout(widget1);
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    referenceLabel = new QLabel(widget1);
    referenceLabel->setObjectName(QString::fromUtf8("referenceLabel"));

    hboxLayout1->addWidget(referenceLabel);

    reference = new QLineEdit(widget1);
    reference->setObjectName(QString::fromUtf8("reference"));

    hboxLayout1->addWidget(reference);

    contextLabel = new QLabel(widget1);
    contextLabel->setObjectName(QString::fromUtf8("contextLabel"));

    hboxLayout1->addWidget(contextLabel);

    context = new QLineEdit(widget1);
    context->setObjectName(QString::fromUtf8("context"));

    hboxLayout1->addWidget(context);


    gridLayout4->addWidget(widget1, 1, 0, 1, 1);

    toolBox->addItem(messaging, QApplication::translate("Teamwork", "Messaging", 0, QApplication::UnicodeUTF8));
    localstate = new QWidget();
    localstate->setObjectName(QString::fromUtf8("localstate"));
    localstate->setGeometry(QRect(0, 0, 391, 211));
    gridLayout5 = new QGridLayout(localstate);
    gridLayout5->setSpacing(6);
    gridLayout5->setMargin(9);
    gridLayout5->setObjectName(QString::fromUtf8("gridLayout5"));
    managePatches = new QPushButton(localstate);
    managePatches->setObjectName(QString::fromUtf8("managePatches"));

    gridLayout5->addWidget(managePatches, 0, 0, 1, 1);

    toolBox->addItem(localstate, QApplication::translate("Teamwork", "Local state", 0, QApplication::UnicodeUTF8));
    enableCollaboration = new QCheckBox(Teamwork);
    enableCollaboration->setObjectName(QString::fromUtf8("enableCollaboration"));
    enableCollaboration->setGeometry(QRect(9, 9, 391, 19));
    historyTab = new QTabWidget(Teamwork);
    historyTab->setObjectName(QString::fromUtf8("historyTab"));
    historyTab->setGeometry(QRect(9, 363, 391, 323));
    messagesTab = new QWidget();
    messagesTab->setObjectName(QString::fromUtf8("messagesTab"));
    layoutWidget = new QWidget(messagesTab);
    layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
    layoutWidget->setGeometry(QRect(10, 270, 371, 28));
    hboxLayout2 = new QHBoxLayout(layoutWidget);
    hboxLayout2->setSpacing(6);
    hboxLayout2->setMargin(0);
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    clearMessages = new QPushButton(layoutWidget);
    clearMessages->setObjectName(QString::fromUtf8("clearMessages"));

    hboxLayout2->addWidget(clearMessages);

    messageHistory = new QPushButton(layoutWidget);
    messageHistory->setObjectName(QString::fromUtf8("messageHistory"));

    hboxLayout2->addWidget(messageHistory);

    messageFilter = new QToolButton(layoutWidget);
    messageFilter->setObjectName(QString::fromUtf8("messageFilter"));

    hboxLayout2->addWidget(messageFilter);

    messageList = new QListView(messagesTab);
    messageList->setObjectName(QString::fromUtf8("messageList"));
    messageList->setGeometry(QRect(9, 9, 369, 251));
    messageList->setProperty("isWrapping", QVariant(false));
    messageList->setViewMode(QListView::ListMode);
    historyTab->addTab(messagesTab, QApplication::translate("Teamwork", "Messages", 0, QApplication::UnicodeUTF8));
    logTab = new QWidget();
    logTab->setObjectName(QString::fromUtf8("logTab"));
    gridLayout6 = new QGridLayout(logTab);
    gridLayout6->setSpacing(6);
    gridLayout6->setMargin(9);
    gridLayout6->setObjectName(QString::fromUtf8("gridLayout6"));
    vboxLayout = new QVBoxLayout();
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(0);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    logList = new QListView(logTab);
    logList->setObjectName(QString::fromUtf8("logList"));

    vboxLayout->addWidget(logList);

    hboxLayout3 = new QHBoxLayout();
    hboxLayout3->setSpacing(6);
    hboxLayout3->setMargin(0);
    hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
    clearLog = new QPushButton(logTab);
    clearLog->setObjectName(QString::fromUtf8("clearLog"));

    hboxLayout3->addWidget(clearLog);

    saveLog = new QPushButton(logTab);
    saveLog->setObjectName(QString::fromUtf8("saveLog"));

    hboxLayout3->addWidget(saveLog);

    logFilter = new QToolButton(logTab);
    logFilter->setObjectName(QString::fromUtf8("logFilter"));

    hboxLayout3->addWidget(logFilter);


    vboxLayout->addLayout(hboxLayout3);


    gridLayout6->addLayout(vboxLayout, 0, 0, 1, 1);

    historyTab->addTab(logTab, QApplication::translate("Teamwork", "Log", 0, QApplication::UnicodeUTF8));
    retranslateUi(Teamwork);

    toolBox->setCurrentIndex(3);
    connectionTab->setCurrentIndex(0);
    historyTab->setCurrentIndex(0);


    QMetaObject::connectSlotsByName(Teamwork);
    } // setupUi

    void retranslateUi(QWidget *Teamwork)
    {
    Teamwork->setWindowTitle(QApplication::translate("Teamwork", "Form", 0, QApplication::UnicodeUTF8));
    connectedClientsLabel->setText(QApplication::translate("Teamwork", "Connected clients:", 0, QApplication::UnicodeUTF8));
    allowIncoming->setText(QApplication::translate("Teamwork", "Allow incoming", 0, QApplication::UnicodeUTF8));
    connectionTab->setTabText(connectionTab->indexOf(localTab), QApplication::translate("Teamwork", "Local server", 0, QApplication::UnicodeUTF8));
    loginPassword->setToolTip(QApplication::translate("Teamwork", "the password for logging into the server", 0, QApplication::UnicodeUTF8));
    connectServer->setToolTip(QApplication::translate("Teamwork", "connect to the given server using the given login-information(if the name is blank the local name is used)", 0, QApplication::UnicodeUTF8));
    connectServer->setText(QApplication::translate("Teamwork", "Connect", 0, QApplication::UnicodeUTF8));
    serverBox->setToolTip(QApplication::translate("Teamwork", "the server to connect", 0, QApplication::UnicodeUTF8));
    connectedServersLabel->setText(QApplication::translate("Teamwork", "Connected servers:", 0, QApplication::UnicodeUTF8));
    loginName->setToolTip(QApplication::translate("Teamwork", "the user-name for logging into the server", 0, QApplication::UnicodeUTF8));
    connectionTab->setTabText(connectionTab->indexOf(remoteTab), QApplication::translate("Teamwork", "Servers", 0, QApplication::UnicodeUTF8));
    toolBox->setItemText(toolBox->indexOf(connection), QApplication::translate("Teamwork", "Connection", 0, QApplication::UnicodeUTF8));
    availableLabel->setText(QApplication::translate("Teamwork", "Available:", 0, QApplication::UnicodeUTF8));
    connectedLabel->setText(QApplication::translate("Teamwork", "Collaborating:", 0, QApplication::UnicodeUTF8));
    toolBox->setItemText(toolBox->indexOf(developers), QApplication::translate("Teamwork", "Developers", 0, QApplication::UnicodeUTF8));
    messageText->setToolTip(QApplication::translate("Teamwork", "the message you wish to send", 0, QApplication::UnicodeUTF8));
    sendMessage->setText(QApplication::translate("Teamwork", "&Send message", 0, QApplication::UnicodeUTF8));
    sendMessage->setShortcut(QApplication::translate("Teamwork", "S", 0, QApplication::UnicodeUTF8));
    messageTargetUser->setToolTip(QApplication::translate("Teamwork", "target-user", 0, QApplication::UnicodeUTF8));
    typeLabel->setText(QApplication::translate("Teamwork", "type:", 0, QApplication::UnicodeUTF8));
    messageType->clear();
    messageType->addItem(QApplication::translate("Teamwork", "Message", 0, QApplication::UnicodeUTF8));
    messageType->addItem(QApplication::translate("Teamwork", "Comment", 0, QApplication::UnicodeUTF8));
    messageType->addItem(QApplication::translate("Teamwork", "Source-discussion", 0, QApplication::UnicodeUTF8));
    referenceLabel->setText(QApplication::translate("Teamwork", "Reference:", 0, QApplication::UnicodeUTF8));
    contextLabel->setText(QApplication::translate("Teamwork", "Context:", 0, QApplication::UnicodeUTF8));
    toolBox->setItemText(toolBox->indexOf(messaging), QApplication::translate("Teamwork", "Messaging", 0, QApplication::UnicodeUTF8));
    managePatches->setText(QApplication::translate("Teamwork", "Manage local patches", 0, QApplication::UnicodeUTF8));
    toolBox->setItemText(toolBox->indexOf(localstate), QApplication::translate("Teamwork", "Local state", 0, QApplication::UnicodeUTF8));
    enableCollaboration->setText(QApplication::translate("Teamwork", "Enable Collaboration", 0, QApplication::UnicodeUTF8));
    clearMessages->setText(QApplication::translate("Teamwork", "Clear", 0, QApplication::UnicodeUTF8));
    messageHistory->setText(QApplication::translate("Teamwork", "History", 0, QApplication::UnicodeUTF8));
    messageFilter->setText(QApplication::translate("Teamwork", "Filter", 0, QApplication::UnicodeUTF8));
    historyTab->setTabText(historyTab->indexOf(messagesTab), QApplication::translate("Teamwork", "Messages", 0, QApplication::UnicodeUTF8));
    clearLog->setText(QApplication::translate("Teamwork", "Clear", 0, QApplication::UnicodeUTF8));
    saveLog->setText(QApplication::translate("Teamwork", "Save", 0, QApplication::UnicodeUTF8));
    logFilter->setText(QApplication::translate("Teamwork", "Filter", 0, QApplication::UnicodeUTF8));
    historyTab->setTabText(historyTab->indexOf(logTab), QApplication::translate("Teamwork", "Log", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(Teamwork);
    } // retranslateUi

};


namespace Ui {
    class Teamwork: public Ui_Teamwork {};
} // namespace Ui

#endif // KDEVTEAMWORK_INTERFACE_H
