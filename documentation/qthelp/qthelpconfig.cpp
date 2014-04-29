/*  This file is part of KDevelop

    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>
    Copyright 2014 Kevin Funk <kfunk@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "qthelpconfig.h"

#include <KMessageBox>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <KDebug>
#include <KSettings/Dispatcher>
#include <knewstuff3/knewstuffbutton.h>
#include <qhelpenginecore.h>
#include <QFileDialog>

#include "ui_qthelpconfig.h"
#include "ui_qthelpconfigeditdialog.h"
#include "qthelp_config_shared.h"

K_PLUGIN_FACTORY(QtHelpConfigFactory, registerPlugin<QtHelpConfig>();)
K_EXPORT_PLUGIN(QtHelpConfigFactory("kdevqthelp_config"))

enum Column
{
    NameColumn,
    PathColumn,
    IconColumn,
    GhnsColumn
};

class QtHelpConfigEditDialog : public QDialog, public Ui_QtHelpConfigEditDialog
{
public:
    explicit QtHelpConfigEditDialog(QTreeWidgetItem* modifiedItem, QtHelpConfig* parent = 0,
                                    Qt::WindowFlags f = 0)
        : QDialog(parent, f)
        , m_modifiedItem(modifiedItem)
        , m_config(parent)
    {
        setupUi(this);

        if (modifiedItem) {
            setWindowTitle(i18n("Modify Entry"));
        } else {
            setWindowTitle(i18n("Add New Entry"));
        }
        qchIcon->setIcon("qtlogo");
    }

    bool checkQtHelpFile();

    virtual void accept() override;

private:
    QTreeWidgetItem* m_modifiedItem;
    QtHelpConfig* m_config;
};

bool QtHelpConfigEditDialog::checkQtHelpFile()
{
    //verify if the file is valid and if there is a name
    if(qchName->text().isEmpty()){
        KMessageBox::error(this, i18n("Name cannot be empty."));
        return false;
    }

    return m_config->checkNamespace(qchRequester->text(), m_modifiedItem);
}

void QtHelpConfigEditDialog::accept()
{
    if (!checkQtHelpFile())
        return;

    QDialog::accept();
}

QtHelpConfig::QtHelpConfig(QWidget *parent, const QVariantList &args)
    : KCModule(QtHelpConfigFactory::componentData(), parent, args)
{
    QVBoxLayout * l = new QVBoxLayout( this );

    QWidget* w = new QWidget;
    m_configWidget = new Ui::QtHelpConfigUI;
    m_configWidget->setupUi( w );
    m_configWidget->addButton->setIcon(QIcon::fromTheme("list-add"));
    connect(m_configWidget->addButton, SIGNAL(clicked(bool)), this, SLOT(add()));
    m_configWidget->editButton->setIcon(QIcon::fromTheme("document-edit"));
    connect(m_configWidget->editButton, SIGNAL(clicked(bool)), this, SLOT(modify()));
    m_configWidget->removeButton->setIcon(QIcon::fromTheme("list-remove"));
    connect(m_configWidget->removeButton, SIGNAL(clicked(bool)), this, SLOT(remove()));
    m_configWidget->upButton->setIcon(QIcon::fromTheme("arrow-up"));
    connect(m_configWidget->upButton, SIGNAL(clicked(bool)), this, SLOT(up()));
    m_configWidget->downButton->setIcon(QIcon::fromTheme("arrow-down"));
    connect(m_configWidget->downButton, SIGNAL(clicked(bool)), this, SLOT(down()));
    // Table
    connect(m_configWidget->qchTable, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
    m_configWidget->qchTable->setColumnHidden(IconColumn, true);
    m_configWidget->qchTable->setColumnHidden(GhnsColumn, true);
    m_configWidget->qchTable->header()->setStretchLastSection(true);

    // Add GHNS button
    KNS3::Button *knsButton = new KNS3::Button(i18nc("Allow user to get some API documentation with GHNS", "Get New Documentation"), "kdevelop-qthelp.knsrc", m_configWidget->qchManage);
    m_configWidget->verticalLayout->insertWidget(1, knsButton);
    connect(knsButton, SIGNAL(dialogFinished(KNS3::Entry::List)), SLOT(knsUpdate(KNS3::Entry::List)));
    connect(m_configWidget->loadQtDocsCheckBox, SIGNAL(toggled(bool)), this, SLOT(changed()));
    connect(m_configWidget->qchSearchDirButton, SIGNAL(clicked(bool)), this, SLOT(chooseSearchDir()));
    connect(m_configWidget->qchSearchDir,SIGNAL(textChanged(QString)), this, SLOT(searchDirChanged()));
    l->addWidget( w );
    load();
    selectionChanged();
}

QtHelpConfig::~QtHelpConfig()
{
    delete m_configWidget;
}

void QtHelpConfig::save()
{
    QStringList iconList, nameList, pathList, ghnsList;
    for (int i = 0; i < m_configWidget->qchTable->topLevelItemCount(); i++) {
        const QTreeWidgetItem* item = m_configWidget->qchTable->topLevelItem(i);
        nameList << item->text(0);
        pathList << item->text(1);
        iconList << item->text(2);
        ghnsList << item->text(3);
    }
    QString searchDir = m_configWidget->qchSearchDir->text();
    bool loadQtDoc = m_configWidget->loadQtDocsCheckBox->isChecked();

    qtHelpWriteConfig(iconList, nameList, pathList, ghnsList, searchDir, loadQtDoc);

    KSettings::Dispatcher::reparseConfiguration( componentData().componentName() );

    emit changed(false);
}

void QtHelpConfig::load()
{
    m_configWidget->qchTable->clear();;

    QStringList iconList, nameList, pathList, ghnsList;
    QString searchDir;
    bool loadQtDoc;
    qtHelpReadConfig(iconList, nameList, pathList, ghnsList, searchDir, loadQtDoc);

    const int size = qMin(qMin(iconList.size(), nameList.size()), pathList.size());
    for(int i = 0; i < size; ++i) {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_configWidget->qchTable);
        item->setIcon(NameColumn, QIcon(iconList.at(i)));
        item->setText(NameColumn, nameList.at(i));
        item->setText(PathColumn, pathList.at(i));
        item->setText(IconColumn, iconList.at(i));
        item->setText(GhnsColumn, ghnsList.size()>i ? ghnsList.at(i) : "0");
    }
    m_configWidget->qchSearchDir->setText(searchDir);
    m_configWidget->loadQtDocsCheckBox->setChecked(loadQtDoc);

    emit changed(false);
}

void QtHelpConfig::defaults()
{
    bool change=false;
    if(m_configWidget->qchTable->topLevelItemCount() > 0) {
        m_configWidget->qchTable->clear();
        change = true;
    }
    if(!m_configWidget->loadQtDocsCheckBox->isChecked()){
        m_configWidget->loadQtDocsCheckBox->setChecked(true);
        change = true;
    }
    emit changed(change);
}

void QtHelpConfig::selectionChanged()
{
    if (m_configWidget->qchTable->selectedItems().isEmpty()) {
        m_configWidget->removeButton->setEnabled(false);
        m_configWidget->editButton->setEnabled(false);
        m_configWidget->upButton->setEnabled(false);
        m_configWidget->downButton->setEnabled(false);
    } else {
        QTreeWidgetItem* selectedItem = m_configWidget->qchTable->selectedItems().at(0);
        const int selectedRow = m_configWidget->qchTable->indexOfTopLevelItem(selectedItem);
        int rowCount = m_configWidget->qchTable->topLevelItemCount();
        if (selectedItem->text(GhnsColumn) != "0") {
            // TODO: Can't we just remove the file even if it has been installed via GHNS?
            m_configWidget->removeButton->setEnabled(false);
            m_configWidget->removeButton->setToolTip(tr("Please uninstall this via GHNS"));
        } else {
            m_configWidget->removeButton->setEnabled(true);
            m_configWidget->removeButton->setToolTip(QString());
        }
        m_configWidget->editButton->setEnabled(true);
        if (selectedRow == 0) {
            m_configWidget->upButton->setEnabled(false);
        } else {
            m_configWidget->upButton->setEnabled(true);
        }
        if (rowCount > selectedRow + 1) {
            m_configWidget->downButton->setEnabled(true);
        } else {
            m_configWidget->downButton->setEnabled(false);
        }
    }
}

void QtHelpConfig::add()
{
    QtHelpConfigEditDialog dialog(0, this);
    if (!dialog.exec())
        return;

    QTreeWidgetItem* item = new QTreeWidgetItem(m_configWidget->qchTable);
    item->setIcon(NameColumn, QIcon(dialog.qchIcon->icon()));
    item->setText(NameColumn, dialog.qchName->text());
    item->setText(PathColumn, dialog.qchRequester->text());
    item->setText(IconColumn, dialog.qchIcon->icon());
    item->setText(GhnsColumn, "0");
    m_configWidget->qchTable->setCurrentItem(item);
    emit changed(true);
}

void QtHelpConfig::modify()
{
    QTreeWidgetItem* item = m_configWidget->qchTable->currentItem();
    if (!item)
        return;

    QtHelpConfigEditDialog dialog(item, this);
    if (item->text(GhnsColumn) != "0") {
        dialog.qchRequester->setText(i18n("Documentation provided by GHNS"));
        dialog.qchRequester->setEnabled(false);
    } else {
        dialog.qchRequester->setText(item->text(PathColumn));
        dialog.qchRequester->setEnabled(true);
    }
    dialog.qchName->setText(item->text(NameColumn));
    dialog.qchIcon->setIcon(item->text(IconColumn));
    if (!dialog.exec()) {
        return;
    }

    item->setIcon(NameColumn, QIcon(dialog.qchIcon->icon()));
    item->setText(NameColumn, dialog.qchName->text());
    item->setText(IconColumn, dialog.qchIcon->icon());
    if(item->text(GhnsColumn) == "0") {
        item->setText(PathColumn, dialog.qchRequester->text());
    }
    emit changed(true);
}

bool QtHelpConfig::checkNamespace(const QString& filename, QTreeWidgetItem* modifiedItem)
{
    QString qtHelpNamespace = QHelpEngineCore::namespaceName(filename);
    if (qtHelpNamespace.isEmpty()) {
        // Open error message (not valid Qt Compressed Help file)
        KMessageBox::error(this, i18n("Qt Compressed Help file is not valid."));
        return false;
    }
    // verify if it's the namespace it's not already in the list
    for(int i=0; i < m_configWidget->qchTable->topLevelItemCount(); i++) {
        const QTreeWidgetItem* item = m_configWidget->qchTable->topLevelItem(i);
        if (item != modifiedItem){
            if (qtHelpNamespace == QHelpEngineCore::namespaceName(item->text(PathColumn))) {
                // Open error message, documentation already imported
                KMessageBox::error(this, i18n("Documentation already imported"));
                return false;
            }
        }
    }
    return true;
}

void QtHelpConfig::remove()
{
    QTreeWidgetItem* selectedItem = m_configWidget->qchTable->currentItem();
    if (!selectedItem)
        return;

    delete selectedItem;
    emit changed(true);
}

void QtHelpConfig::up()
{
    QTreeWidgetItem* item = m_configWidget->qchTable->currentItem();
    if (!item)
        return;
    const int row = m_configWidget->qchTable->indexOfTopLevelItem(item);
    if (row == 0)
        return;

    m_configWidget->qchTable->takeTopLevelItem(row);
    m_configWidget->qchTable->insertTopLevelItem(row - 1, item);
    m_configWidget->qchTable->setCurrentItem(item);
    emit changed();
}

void QtHelpConfig::down()
{
    QTreeWidgetItem* item = m_configWidget->qchTable->currentItem();
    if (!item)
        return;
    const int row = m_configWidget->qchTable->indexOfTopLevelItem(item);
    if (row + 1 >= m_configWidget->qchTable->topLevelItemCount())
        return;

    m_configWidget->qchTable->takeTopLevelItem(row);
    m_configWidget->qchTable->insertTopLevelItem(row + 1, item);
    m_configWidget->qchTable->setCurrentItem(item);
    emit changed();
}

void QtHelpConfig::knsUpdate(KNS3::Entry::List list)
{
    if (list.isEmpty())
        return;

    foreach (const KNS3::Entry& e, list) {
        if(e.status() == KNS3::Entry::Installed) {
            if(e.installedFiles().size() == 1) {
                QString filename = e.installedFiles().at(0);
                if(checkNamespace(filename, nullptr)){
                    QTreeWidgetItem* item = new QTreeWidgetItem(m_configWidget->qchTable);
                    item->setIcon(NameColumn, QIcon("documentation"));
                    item->setText(NameColumn, e.name());
                    item->setText(PathColumn, filename);
                    item->setText(IconColumn, "documentation");
                    item->setText(GhnsColumn, "1");
                    m_configWidget->qchTable->setCurrentItem(item);
                } else {
                    kDebug() << "namespace error";
                }
            }
        } else if(e.status() ==  KNS3::Entry::Deleted) {
            if(e.uninstalledFiles().size() == 1) {
                for(int i=0; i < m_configWidget->qchTable->topLevelItemCount(); i++) {
                    QTreeWidgetItem* item = m_configWidget->qchTable->topLevelItem(i);
                    if (e.uninstalledFiles().at(0) == item->text(PathColumn)) {
                        delete item;
                        break;
                    }
                }
            }
        }
    }
    emit changed(true);
}

void QtHelpConfig::chooseSearchDir()
{
    m_configWidget->qchSearchDir->setText(QFileDialog::getExistingDirectory(this));
}


void QtHelpConfig::searchDirChanged()
{
    emit changed(true);
}


#include "qthelpconfig.moc"
