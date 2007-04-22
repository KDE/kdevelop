#ifndef KDEVTEAMWORK_FILECOLLABORATIONSESSION_H
#define KDEVTEAMWORK_FILECOLLABORATIONSESSION_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include "klineedit.h"

class Ui_NewFileCollaborationSession
{
public:
    QGridLayout *gridLayout;
    QCheckBox *allowSentDocuments;
    QLabel *files;
    QLabel *users;
    QDialogButtonBox *buttonBox;
    KLineEdit *sessionName;
    QLabel *label;

    void setupUi(QDialog *NewFileCollaborationSession)
    {
    NewFileCollaborationSession->setObjectName(QString::fromUtf8("NewFileCollaborationSession"));
    NewFileCollaborationSession->resize(QSize(400, 177).expandedTo(NewFileCollaborationSession->minimumSizeHint()));
    gridLayout = new QGridLayout(NewFileCollaborationSession);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(9);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    allowSentDocuments = new QCheckBox(NewFileCollaborationSession);
    allowSentDocuments->setObjectName(QString::fromUtf8("allowSentDocuments"));
    allowSentDocuments->setChecked(true);

    gridLayout->addWidget(allowSentDocuments, 1, 0, 1, 3);

    files = new QLabel(NewFileCollaborationSession);
    files->setObjectName(QString::fromUtf8("files"));

    gridLayout->addWidget(files, 2, 0, 1, 3);

    users = new QLabel(NewFileCollaborationSession);
    users->setObjectName(QString::fromUtf8("users"));

    gridLayout->addWidget(users, 3, 0, 1, 2);

    buttonBox = new QDialogButtonBox(NewFileCollaborationSession);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(4));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(buttonBox->sizePolicy().hasHeightForWidth());
    buttonBox->setSizePolicy(sizePolicy);
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);

    gridLayout->addWidget(buttonBox, 3, 2, 1, 1);

    sessionName = new KLineEdit(NewFileCollaborationSession);
    sessionName->setObjectName(QString::fromUtf8("sessionName"));

    gridLayout->addWidget(sessionName, 0, 1, 1, 2);

    label = new QLabel(NewFileCollaborationSession);
    label->setObjectName(QString::fromUtf8("label"));

    gridLayout->addWidget(label, 0, 0, 1, 1);

    retranslateUi(NewFileCollaborationSession);
    QObject::connect(buttonBox, SIGNAL(accepted()), NewFileCollaborationSession, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), NewFileCollaborationSession, SLOT(reject()));

    QMetaObject::connectSlotsByName(NewFileCollaborationSession);
    } // setupUi

    void retranslateUi(QDialog *NewFileCollaborationSession)
    {
    NewFileCollaborationSession->setWindowTitle(QApplication::translate("NewFileCollaborationSession", "New File-Collaboration Session", 0, QApplication::UnicodeUTF8));
    allowSentDocuments->setText(QApplication::translate("NewFileCollaborationSession", "Allow collaborators to add own documents", 0, QApplication::UnicodeUTF8));
    files->setText(QApplication::translate("NewFileCollaborationSession", "Files:\n"
"FIle 1\n"
"File 2", 0, QApplication::UnicodeUTF8));
    users->setText(QApplication::translate("NewFileCollaborationSession", "Invite users:", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("NewFileCollaborationSession", "Session-Name:", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(NewFileCollaborationSession);
    } // retranslateUi

};

namespace Ui {
    class NewFileCollaborationSession: public Ui_NewFileCollaborationSession {};
} // namespace Ui

#endif // KDEVTEAMWORK_FILECOLLABORATIONSESSION_H
