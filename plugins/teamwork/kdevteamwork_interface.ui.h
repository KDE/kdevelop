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
#include <QtGui/QSpacerItem>
#include <QtGui/QTabWidget>
#include <QtGui/QToolBox>
#include <QtGui/QToolButton>
#include <QtGui/QTreeView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "kcombobox.h"
#include "klineedit.h"
#include "ktextedit.h"

class Ui_Teamwork
{
public:
    QGridLayout *gridLayout;
    QToolBox *toolBox;
    QWidget *connection;
    QGridLayout *gridLayout1;
    QTabWidget *connectionTab;
    QWidget *localTab;
    QGridLayout *gridLayout2;
    QListView *connectedClients;
    QLabel *connectedClientsLabel;
    QCheckBox *allowIncoming;
    QWidget *remoteTab;
    QGridLayout *gridLayout3;
    QListView *connectedServers;
    KLineEdit *loginPassword;
    QPushButton *connectServer;
    QComboBox *serverBox;
    QLabel *connectedServersLabel;
    KLineEdit *loginName;
    QWidget *developers;
    QGridLayout *gridLayout4;
    QLabel *availableLabel;
    QTreeView *availableDevelopers;
    QTreeView *connectedDevelopers;
    QLabel *connectedLabel;
    QWidget *messaging;
    QGridLayout *gridLayout5;
    QHBoxLayout *hboxLayout;
    KComboBox *messageTargetUser;
    QToolButton *answeringToButton;
    QHBoxLayout *hboxLayout1;
    QLabel *typeLabel;
    QComboBox *messageType;
    QHBoxLayout *hboxLayout2;
    QLabel *contextLabel;
    QLineEdit *context;
    QHBoxLayout *hboxLayout3;
    QLabel *referenceLabel;
    QLineEdit *reference;
    QHBoxLayout *hboxLayout4;
    QPushButton *clearMessageButton;
    QSpacerItem *spacerItem;
    QPushButton *sendMessage;
    KTextEdit *messageText;
    QWidget *localstate;
    QGridLayout *gridLayout6;
    QPushButton *managePatches;
    QTabWidget *historyTab;
    QWidget *messagesTab;
    QGridLayout *gridLayout7;
    QTabWidget *messageUsers;
    QWidget *allUsersTab;
    QGridLayout *gridLayout8;
    QListView *messageList;
    QWidget *layoutWidget;
    QHBoxLayout *hboxLayout5;
    QPushButton *clearMessages;
    QPushButton *messageHistory;
    QWidget *logTab;
    QGridLayout *gridLayout9;
    QVBoxLayout *vboxLayout;
    QListView *logList;
    QHBoxLayout *hboxLayout6;
    QPushButton *clearLog;
    QPushButton *saveLog;
    QToolButton *logFilter;
    QCheckBox *enableCollaboration;

    void setupUi(QWidget *Teamwork)
    {
    Teamwork->setObjectName(QString::fromUtf8("Teamwork"));
    Teamwork->resize(QSize(321, 748).expandedTo(Teamwork->minimumSizeHint()));
    gridLayout = new QGridLayout(Teamwork);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(9);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    toolBox = new QToolBox(Teamwork);
    toolBox->setObjectName(QString::fromUtf8("toolBox"));
    connection = new QWidget();
    connection->setObjectName(QString::fromUtf8("connection"));
    connection->setGeometry(QRect(0, 0, 177, 243));
    gridLayout1 = new QGridLayout(connection);
    gridLayout1->setSpacing(6);
    gridLayout1->setMargin(9);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
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
    gridLayout2 = new QGridLayout(localTab);
    gridLayout2->setSpacing(6);
    gridLayout2->setMargin(9);
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
    connectedClients = new QListView(localTab);
    connectedClients->setObjectName(QString::fromUtf8("connectedClients"));

    gridLayout2->addWidget(connectedClients, 2, 0, 1, 1);

    connectedClientsLabel = new QLabel(localTab);
    connectedClientsLabel->setObjectName(QString::fromUtf8("connectedClientsLabel"));

    gridLayout2->addWidget(connectedClientsLabel, 1, 0, 1, 1);

    allowIncoming = new QCheckBox(localTab);
    allowIncoming->setObjectName(QString::fromUtf8("allowIncoming"));

    gridLayout2->addWidget(allowIncoming, 0, 0, 1, 1);

    connectionTab->addTab(localTab, QApplication::translate("Teamwork", "Local server", 0, QApplication::UnicodeUTF8));
    remoteTab = new QWidget();
    remoteTab->setObjectName(QString::fromUtf8("remoteTab"));
    gridLayout3 = new QGridLayout(remoteTab);
    gridLayout3->setSpacing(6);
    gridLayout3->setMargin(9);
    gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));
    connectedServers = new QListView(remoteTab);
    connectedServers->setObjectName(QString::fromUtf8("connectedServers"));

    gridLayout3->addWidget(connectedServers, 3, 0, 1, 3);

    loginPassword = new KLineEdit(remoteTab);
    loginPassword->setObjectName(QString::fromUtf8("loginPassword"));

    gridLayout3->addWidget(loginPassword, 1, 1, 1, 1);

    connectServer = new QPushButton(remoteTab);
    connectServer->setObjectName(QString::fromUtf8("connectServer"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(connectServer->sizePolicy().hasHeightForWidth());
    connectServer->setSizePolicy(sizePolicy1);

    gridLayout3->addWidget(connectServer, 1, 2, 1, 1);

    serverBox = new QComboBox(remoteTab);
    serverBox->setObjectName(QString::fromUtf8("serverBox"));
    serverBox->setEditable(true);

    gridLayout3->addWidget(serverBox, 0, 0, 1, 3);

    connectedServersLabel = new QLabel(remoteTab);
    connectedServersLabel->setObjectName(QString::fromUtf8("connectedServersLabel"));

    gridLayout3->addWidget(connectedServersLabel, 2, 0, 1, 3);

    loginName = new KLineEdit(remoteTab);
    loginName->setObjectName(QString::fromUtf8("loginName"));

    gridLayout3->addWidget(loginName, 1, 0, 1, 1);

    connectionTab->addTab(remoteTab, QApplication::translate("Teamwork", "Servers", 0, QApplication::UnicodeUTF8));

    gridLayout1->addWidget(connectionTab, 0, 0, 1, 1);

    toolBox->addItem(connection, QApplication::translate("Teamwork", "Connection", 0, QApplication::UnicodeUTF8));
    developers = new QWidget();
    developers->setObjectName(QString::fromUtf8("developers"));
    developers->setGeometry(QRect(0, 0, 112, 258));
    gridLayout4 = new QGridLayout(developers);
    gridLayout4->setSpacing(6);
    gridLayout4->setMargin(9);
    gridLayout4->setObjectName(QString::fromUtf8("gridLayout4"));
    availableLabel = new QLabel(developers);
    availableLabel->setObjectName(QString::fromUtf8("availableLabel"));

    gridLayout4->addWidget(availableLabel, 0, 0, 1, 1);

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
    availableDevelopers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    availableDevelopers->setAlternatingRowColors(true);
    availableDevelopers->setIndentation(15);
    availableDevelopers->setRootIsDecorated(true);

    gridLayout4->addWidget(availableDevelopers, 1, 0, 1, 1);

    connectedDevelopers = new QTreeView(developers);
    connectedDevelopers->setObjectName(QString::fromUtf8("connectedDevelopers"));
    QSizePolicy sizePolicy3(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(7));
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(connectedDevelopers->sizePolicy().hasHeightForWidth());
    connectedDevelopers->setSizePolicy(sizePolicy3);
    connectedDevelopers->setMaximumSize(QSize(16777215, 16777215));
    connectedDevelopers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connectedDevelopers->setAlternatingRowColors(true);
    connectedDevelopers->setIndentation(15);
    connectedDevelopers->setRootIsDecorated(true);

    gridLayout4->addWidget(connectedDevelopers, 3, 0, 1, 1);

    connectedLabel = new QLabel(developers);
    connectedLabel->setObjectName(QString::fromUtf8("connectedLabel"));

    gridLayout4->addWidget(connectedLabel, 2, 0, 1, 1);

    toolBox->addItem(developers, QApplication::translate("Teamwork", "Developers", 0, QApplication::UnicodeUTF8));
    messaging = new QWidget();
    messaging->setObjectName(QString::fromUtf8("messaging"));
    messaging->setGeometry(QRect(0, 0, 303, 314));
    gridLayout5 = new QGridLayout(messaging);
    gridLayout5->setSpacing(6);
    gridLayout5->setMargin(9);
    gridLayout5->setObjectName(QString::fromUtf8("gridLayout5"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    messageTargetUser = new KComboBox(messaging);
    messageTargetUser->setObjectName(QString::fromUtf8("messageTargetUser"));
    QSizePolicy sizePolicy4(static_cast<QSizePolicy::Policy>(5), static_cast<QSizePolicy::Policy>(0));
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(messageTargetUser->sizePolicy().hasHeightForWidth());
    messageTargetUser->setSizePolicy(sizePolicy4);
    messageTargetUser->setEditable(true);

    hboxLayout->addWidget(messageTargetUser);

    answeringToButton = new QToolButton(messaging);
    answeringToButton->setObjectName(QString::fromUtf8("answeringToButton"));
    QSizePolicy sizePolicy5(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(4));
    sizePolicy5.setHorizontalStretch(0);
    sizePolicy5.setVerticalStretch(0);
    sizePolicy5.setHeightForWidth(answeringToButton->sizePolicy().hasHeightForWidth());
    answeringToButton->setSizePolicy(sizePolicy5);
    answeringToButton->setContextMenuPolicy(Qt::NoContextMenu);
    answeringToButton->setAutoFillBackground(true);
    answeringToButton->setCheckable(true);

    hboxLayout->addWidget(answeringToButton);


    gridLayout5->addLayout(hboxLayout, 0, 0, 1, 1);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    typeLabel = new QLabel(messaging);
    typeLabel->setObjectName(QString::fromUtf8("typeLabel"));
    QSizePolicy sizePolicy6(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy6.setHorizontalStretch(0);
    sizePolicy6.setVerticalStretch(0);
    sizePolicy6.setHeightForWidth(typeLabel->sizePolicy().hasHeightForWidth());
    typeLabel->setSizePolicy(sizePolicy6);
    typeLabel->setMaximumSize(QSize(50, 16777215));
    typeLabel->setLayoutDirection(Qt::LeftToRight);
    typeLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    hboxLayout1->addWidget(typeLabel);

    messageType = new QComboBox(messaging);
    messageType->setObjectName(QString::fromUtf8("messageType"));

    hboxLayout1->addWidget(messageType);


    gridLayout5->addLayout(hboxLayout1, 1, 0, 1, 1);

    hboxLayout2 = new QHBoxLayout();
    hboxLayout2->setSpacing(6);
    hboxLayout2->setMargin(0);
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    contextLabel = new QLabel(messaging);
    contextLabel->setObjectName(QString::fromUtf8("contextLabel"));
    QSizePolicy sizePolicy7(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy7.setHorizontalStretch(0);
    sizePolicy7.setVerticalStretch(0);
    sizePolicy7.setHeightForWidth(contextLabel->sizePolicy().hasHeightForWidth());
    contextLabel->setSizePolicy(sizePolicy7);

    hboxLayout2->addWidget(contextLabel);

    context = new QLineEdit(messaging);
    context->setObjectName(QString::fromUtf8("context"));

    hboxLayout2->addWidget(context);


    gridLayout5->addLayout(hboxLayout2, 3, 0, 1, 1);

    hboxLayout3 = new QHBoxLayout();
    hboxLayout3->setSpacing(2);
    hboxLayout3->setMargin(0);
    hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
    referenceLabel = new QLabel(messaging);
    referenceLabel->setObjectName(QString::fromUtf8("referenceLabel"));
    QSizePolicy sizePolicy8(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
    sizePolicy8.setHorizontalStretch(0);
    sizePolicy8.setVerticalStretch(0);
    sizePolicy8.setHeightForWidth(referenceLabel->sizePolicy().hasHeightForWidth());
    referenceLabel->setSizePolicy(sizePolicy8);

    hboxLayout3->addWidget(referenceLabel);

    reference = new QLineEdit(messaging);
    reference->setObjectName(QString::fromUtf8("reference"));

    hboxLayout3->addWidget(reference);


    gridLayout5->addLayout(hboxLayout3, 2, 0, 1, 1);

    hboxLayout4 = new QHBoxLayout();
    hboxLayout4->setSpacing(6);
    hboxLayout4->setMargin(0);
    hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));
    clearMessageButton = new QPushButton(messaging);
    clearMessageButton->setObjectName(QString::fromUtf8("clearMessageButton"));

    hboxLayout4->addWidget(clearMessageButton);

    spacerItem = new QSpacerItem(71, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout4->addItem(spacerItem);

    sendMessage = new QPushButton(messaging);
    sendMessage->setObjectName(QString::fromUtf8("sendMessage"));

    hboxLayout4->addWidget(sendMessage);


    gridLayout5->addLayout(hboxLayout4, 5, 0, 1, 1);

    messageText = new KTextEdit(messaging);
    messageText->setObjectName(QString::fromUtf8("messageText"));

    gridLayout5->addWidget(messageText, 4, 0, 1, 1);

    toolBox->addItem(messaging, QApplication::translate("Teamwork", "Messaging", 0, QApplication::UnicodeUTF8));
    localstate = new QWidget();
    localstate->setObjectName(QString::fromUtf8("localstate"));
    localstate->setGeometry(QRect(0, 0, 166, 45));
    gridLayout6 = new QGridLayout(localstate);
    gridLayout6->setSpacing(6);
    gridLayout6->setMargin(9);
    gridLayout6->setObjectName(QString::fromUtf8("gridLayout6"));
    managePatches = new QPushButton(localstate);
    managePatches->setObjectName(QString::fromUtf8("managePatches"));

    gridLayout6->addWidget(managePatches, 0, 0, 1, 1);

    toolBox->addItem(localstate, QApplication::translate("Teamwork", "Local state", 0, QApplication::UnicodeUTF8));

    gridLayout->addWidget(toolBox, 1, 0, 1, 1);

    historyTab = new QTabWidget(Teamwork);
    historyTab->setObjectName(QString::fromUtf8("historyTab"));
    messagesTab = new QWidget();
    messagesTab->setObjectName(QString::fromUtf8("messagesTab"));
    gridLayout7 = new QGridLayout(messagesTab);
    gridLayout7->setSpacing(6);
    gridLayout7->setMargin(9);
    gridLayout7->setObjectName(QString::fromUtf8("gridLayout7"));
    messageUsers = new QTabWidget(messagesTab);
    messageUsers->setObjectName(QString::fromUtf8("messageUsers"));
    allUsersTab = new QWidget();
    allUsersTab->setObjectName(QString::fromUtf8("allUsersTab"));
    gridLayout8 = new QGridLayout(allUsersTab);
    gridLayout8->setSpacing(6);
    gridLayout8->setMargin(9);
    gridLayout8->setObjectName(QString::fromUtf8("gridLayout8"));
    messageList = new QListView(allUsersTab);
    messageList->setObjectName(QString::fromUtf8("messageList"));
    messageList->setProperty("isWrapping", QVariant(false));
    messageList->setViewMode(QListView::ListMode);

    gridLayout8->addWidget(messageList, 0, 0, 1, 1);

    messageUsers->addTab(allUsersTab, QApplication::translate("Teamwork", "All Users", 0, QApplication::UnicodeUTF8));

    gridLayout7->addWidget(messageUsers, 0, 0, 1, 1);

    layoutWidget = new QWidget(messagesTab);
    layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
    hboxLayout5 = new QHBoxLayout(layoutWidget);
    hboxLayout5->setSpacing(6);
    hboxLayout5->setMargin(0);
    hboxLayout5->setObjectName(QString::fromUtf8("hboxLayout5"));
    clearMessages = new QPushButton(layoutWidget);
    clearMessages->setObjectName(QString::fromUtf8("clearMessages"));

    hboxLayout5->addWidget(clearMessages);

    messageHistory = new QPushButton(layoutWidget);
    messageHistory->setObjectName(QString::fromUtf8("messageHistory"));

    hboxLayout5->addWidget(messageHistory);


    gridLayout7->addWidget(layoutWidget, 1, 0, 1, 1);

    historyTab->addTab(messagesTab, QApplication::translate("Teamwork", "Messages", 0, QApplication::UnicodeUTF8));
    logTab = new QWidget();
    logTab->setObjectName(QString::fromUtf8("logTab"));
    gridLayout9 = new QGridLayout(logTab);
    gridLayout9->setSpacing(6);
    gridLayout9->setMargin(9);
    gridLayout9->setObjectName(QString::fromUtf8("gridLayout9"));
    vboxLayout = new QVBoxLayout();
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(0);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    logList = new QListView(logTab);
    logList->setObjectName(QString::fromUtf8("logList"));

    vboxLayout->addWidget(logList);

    hboxLayout6 = new QHBoxLayout();
    hboxLayout6->setSpacing(6);
    hboxLayout6->setMargin(0);
    hboxLayout6->setObjectName(QString::fromUtf8("hboxLayout6"));
    clearLog = new QPushButton(logTab);
    clearLog->setObjectName(QString::fromUtf8("clearLog"));

    hboxLayout6->addWidget(clearLog);

    saveLog = new QPushButton(logTab);
    saveLog->setObjectName(QString::fromUtf8("saveLog"));

    hboxLayout6->addWidget(saveLog);

    logFilter = new QToolButton(logTab);
    logFilter->setObjectName(QString::fromUtf8("logFilter"));

    hboxLayout6->addWidget(logFilter);


    vboxLayout->addLayout(hboxLayout6);


    gridLayout9->addLayout(vboxLayout, 0, 0, 1, 1);

    historyTab->addTab(logTab, QApplication::translate("Teamwork", "Log", 0, QApplication::UnicodeUTF8));

    gridLayout->addWidget(historyTab, 2, 0, 1, 1);

    enableCollaboration = new QCheckBox(Teamwork);
    enableCollaboration->setObjectName(QString::fromUtf8("enableCollaboration"));

    gridLayout->addWidget(enableCollaboration, 0, 0, 1, 1);

    retranslateUi(Teamwork);

    toolBox->setCurrentIndex(2);
    connectionTab->setCurrentIndex(1);
    historyTab->setCurrentIndex(0);
    messageUsers->setCurrentIndex(0);


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
    messageTargetUser->setToolTip(QApplication::translate("Teamwork", "target-user", 0, QApplication::UnicodeUTF8));
    answeringToButton->setText(QApplication::translate("Teamwork", "Answer To", 0, QApplication::UnicodeUTF8));
    typeLabel->setText(QApplication::translate("Teamwork", "Type:", 0, QApplication::UnicodeUTF8));
    messageType->clear();
    messageType->addItem(QApplication::translate("Teamwork", "Message", 0, QApplication::UnicodeUTF8));
    messageType->addItem(QApplication::translate("Teamwork", "Comment", 0, QApplication::UnicodeUTF8));
    messageType->addItem(QApplication::translate("Teamwork", "Source-discussion", 0, QApplication::UnicodeUTF8));
    contextLabel->setText(QApplication::translate("Teamwork", "Theme:", 0, QApplication::UnicodeUTF8));
    referenceLabel->setText(QApplication::translate("Teamwork", "Reference:", 0, QApplication::UnicodeUTF8));
    clearMessageButton->setText(QApplication::translate("Teamwork", "Clear", 0, QApplication::UnicodeUTF8));
    sendMessage->setText(QApplication::translate("Teamwork", "&Send message", 0, QApplication::UnicodeUTF8));
    sendMessage->setShortcut(QApplication::translate("Teamwork", "S", 0, QApplication::UnicodeUTF8));
    messageText->setToolTip(QApplication::translate("Teamwork", "the message you wish to send", 0, QApplication::UnicodeUTF8));
    toolBox->setItemText(toolBox->indexOf(messaging), QApplication::translate("Teamwork", "Messaging", 0, QApplication::UnicodeUTF8));
    managePatches->setText(QApplication::translate("Teamwork", "Manage local patches", 0, QApplication::UnicodeUTF8));
    toolBox->setItemText(toolBox->indexOf(localstate), QApplication::translate("Teamwork", "Local state", 0, QApplication::UnicodeUTF8));
    messageUsers->setTabText(messageUsers->indexOf(allUsersTab), QApplication::translate("Teamwork", "All Users", 0, QApplication::UnicodeUTF8));
    clearMessages->setText(QApplication::translate("Teamwork", "Clear", 0, QApplication::UnicodeUTF8));
    messageHistory->setText(QApplication::translate("Teamwork", "History", 0, QApplication::UnicodeUTF8));
    historyTab->setTabText(historyTab->indexOf(messagesTab), QApplication::translate("Teamwork", "Messages", 0, QApplication::UnicodeUTF8));
    clearLog->setText(QApplication::translate("Teamwork", "Clear", 0, QApplication::UnicodeUTF8));
    saveLog->setText(QApplication::translate("Teamwork", "Save", 0, QApplication::UnicodeUTF8));
    logFilter->setText(QApplication::translate("Teamwork", "Filter", 0, QApplication::UnicodeUTF8));
    historyTab->setTabText(historyTab->indexOf(logTab), QApplication::translate("Teamwork", "Log", 0, QApplication::UnicodeUTF8));
    enableCollaboration->setText(QApplication::translate("Teamwork", "Enable Collaboration", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(Teamwork);
    } // retranslateUi

};

namespace Ui {
    class Teamwork: public Ui_Teamwork {};
} // namespace Ui

#endif // KDEVTEAMWORK_INTERFACE_H
