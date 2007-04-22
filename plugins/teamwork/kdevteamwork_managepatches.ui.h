#ifndef KDEVTEAMWORK_MANAGEPATCHES_H
#define KDEVTEAMWORK_MANAGEPATCHES_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTreeView>
#include <QtGui/QVBoxLayout>

class Ui_ManagePatches
{
public:
    QGridLayout *gridLayout;
    QVBoxLayout *vboxLayout;
    QLabel *label;
    QHBoxLayout *hboxLayout;
    QTreeView *patchesList;
    QVBoxLayout *vboxLayout1;
    QPushButton *add;
    QPushButton *edit;
    QPushButton *remove;
    QSpacerItem *spacerItem;
    QHBoxLayout *hboxLayout1;
    QSpacerItem *spacerItem1;
    QPushButton *closeButton;

    void setupUi(QDialog *ManagePatches)
    {
    ManagePatches->setObjectName(QString::fromUtf8("ManagePatches"));
    ManagePatches->resize(QSize(537, 432).expandedTo(ManagePatches->minimumSizeHint()));
    gridLayout = new QGridLayout(ManagePatches);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(9);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    vboxLayout = new QVBoxLayout();
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(0);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    label = new QLabel(ManagePatches);
    label->setObjectName(QString::fromUtf8("label"));

    vboxLayout->addWidget(label);

    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    patchesList = new QTreeView(ManagePatches);
    patchesList->setObjectName(QString::fromUtf8("patchesList"));

    hboxLayout->addWidget(patchesList);

    vboxLayout1 = new QVBoxLayout();
    vboxLayout1->setSpacing(6);
    vboxLayout1->setMargin(0);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    add = new QPushButton(ManagePatches);
    add->setObjectName(QString::fromUtf8("add"));

    vboxLayout1->addWidget(add);

    edit = new QPushButton(ManagePatches);
    edit->setObjectName(QString::fromUtf8("edit"));

    vboxLayout1->addWidget(edit);

    remove = new QPushButton(ManagePatches);
    remove->setObjectName(QString::fromUtf8("remove"));

    vboxLayout1->addWidget(remove);

    spacerItem = new QSpacerItem(20, 231, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout1->addItem(spacerItem);


    hboxLayout->addLayout(vboxLayout1);


    vboxLayout->addLayout(hboxLayout);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    spacerItem1 = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout1->addItem(spacerItem1);

    closeButton = new QPushButton(ManagePatches);
    closeButton->setObjectName(QString::fromUtf8("closeButton"));

    hboxLayout1->addWidget(closeButton);


    vboxLayout->addLayout(hboxLayout1);


    gridLayout->addLayout(vboxLayout, 0, 0, 1, 1);

    retranslateUi(ManagePatches);
    QObject::connect(closeButton, SIGNAL(pressed()), ManagePatches, SLOT(accept()));

    QMetaObject::connectSlotsByName(ManagePatches);
    } // setupUi

    void retranslateUi(QDialog *ManagePatches)
    {
    ManagePatches->setWindowTitle(QApplication::translate("ManagePatches", "Manage Patches", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("ManagePatches", "Available patches:", 0, QApplication::UnicodeUTF8));
    add->setText(QApplication::translate("ManagePatches", "Add", 0, QApplication::UnicodeUTF8));
    edit->setText(QApplication::translate("ManagePatches", "Edit", 0, QApplication::UnicodeUTF8));
    remove->setText(QApplication::translate("ManagePatches", "Remove", 0, QApplication::UnicodeUTF8));
    closeButton->setText(QApplication::translate("ManagePatches", "Close", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(ManagePatches);
    } // retranslateUi

};

namespace Ui {
    class ManagePatches: public Ui_ManagePatches {};
} // namespace Ui

#endif // KDEVTEAMWORK_MANAGEPATCHES_H
