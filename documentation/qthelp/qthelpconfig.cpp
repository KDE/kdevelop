/*  This file is part of KDevelop

    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>

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

#include "ui_qthelpconfig.h"
#include "qthelpplugin.h"
#include "qthelpprovider.h"

QtHelpConfig::QtHelpConfig(QWidget *parent, const QVariantList &args)
    : KCModule(QtHelpFactory::componentData(), parent, args)
{
    QVBoxLayout * l = new QVBoxLayout( this );

    QWidget* w = new QWidget;
    m_configWidget = new Ui::QtHelpConfigUI;
    m_configWidget->setupUi( w );
    m_configWidget->qchIcon->setIcon("qtlogo");
    m_configWidget->addButton->setIcon(KIcon("list-add"));
    connect(m_configWidget->addButton, SIGNAL(clicked(bool)), this, SLOT(add()));
    m_configWidget->editButton->setIcon(KIcon("document-edit"));
    connect(m_configWidget->editButton, SIGNAL(clicked(bool)), this, SLOT(modify()));
    m_configWidget->removeButton->setIcon(KIcon("list-remove"));
    connect(m_configWidget->removeButton, SIGNAL(clicked(bool)), this, SLOT(remove()));
    m_configWidget->upButton->setIcon(KIcon("arrow-up"));
    connect(m_configWidget->upButton, SIGNAL(clicked(bool)), this, SLOT(up()));
    m_configWidget->downButton->setIcon(KIcon("arrow-down"));
    connect(m_configWidget->downButton, SIGNAL(clicked(bool)), this, SLOT(down()));
    // Table
    connect(m_configWidget->qchTable, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
    m_configWidget->qchTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_configWidget->qchTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_configWidget->qchTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_configWidget->qchTable->setColumnHidden(2, true);
    m_configWidget->qchTable->horizontalHeader()->setStretchLastSection(true);

    connect(m_configWidget->loadQtDocsCheckBox, SIGNAL(toggled(bool)), this, SLOT(changed()));
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
    QtHelpPlugin *plugin = QtHelpPlugin::self();
    if(plugin){
        QStringList iconList;
        QStringList nameList;
        QStringList pathList;
        for(int i=0; i < m_configWidget->qchTable->rowCount(); i++) {
            nameList << m_configWidget->qchTable->item(i,0)->text();
            pathList << m_configWidget->qchTable->item(i,1)->text();
            iconList << m_configWidget->qchTable->item(i,2)->text();
        }
        bool loadQtDoc = m_configWidget->loadQtDocsCheckBox->isChecked();
        plugin->writeConfig(iconList, nameList, pathList, loadQtDoc);
    }
    emit changed(false);
}

void QtHelpConfig::load()
{
    while(m_configWidget->qchTable->rowCount()) {
        m_configWidget->qchTable->removeRow(0);
    }
    QtHelpPlugin *plugin = QtHelpPlugin::self();
    if(plugin){
        plugin->readConfig();
        foreach(QtHelpProvider* qtProvider, plugin->qtHelpProviderLoaded()) {
            int row = m_configWidget->qchTable->rowCount();
            m_configWidget->qchTable->insertRow(row);
            QTableWidgetItem *itemName = new QTableWidgetItem(KIcon(qtProvider->iconName()), qtProvider->name());
            m_configWidget->qchTable->setItem(row, 0, itemName);
            QTableWidgetItem *itemPath = new QTableWidgetItem(qtProvider->fileName());
            m_configWidget->qchTable->setItem(row, 1, itemPath);
            QTableWidgetItem *itemIconName = new QTableWidgetItem(qtProvider->iconName());
            m_configWidget->qchTable->setItem(row, 2, itemIconName);
        }
        m_configWidget->loadQtDocsCheckBox->setChecked(plugin->qtHelpQtDocLoaded());
    }
    emit changed(false);
}

void QtHelpConfig::defaults()
{
    bool change=false;
    if(m_configWidget->qchTable->rowCount() > 0) {
        while(m_configWidget->qchTable->rowCount()) {
            m_configWidget->qchTable->removeRow(0);
        }
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
        int row = m_configWidget->qchTable->selectedItems().at(0)->row();
        int nbRow = m_configWidget->qchTable->rowCount();
        m_configWidget->removeButton->setEnabled(true);
        m_configWidget->editButton->setEnabled(true);
        m_configWidget->qchName->setText(m_configWidget->qchTable->item(row, 0)->text());
        m_configWidget->qchRequester->setText(m_configWidget->qchTable->item(row, 1)->text());
        m_configWidget->qchIcon->setIcon(m_configWidget->qchTable->item(row, 2)->text());
        if (row==0) {
            m_configWidget->upButton->setEnabled(false);
        } else {
            m_configWidget->upButton->setEnabled(true);
        }
        if (nbRow > row + 1) {
            m_configWidget->downButton->setEnabled(true);
        } else {
            m_configWidget->downButton->setEnabled(false);
        }
    }
}

void QtHelpConfig::add()
{
    if(!checkQtHelpFile(false)){
        return;
    }
    int row = m_configWidget->qchTable->rowCount();
    m_configWidget->qchTable->insertRow(row);
    QTableWidgetItem *itemName = new QTableWidgetItem(KIcon(m_configWidget->qchIcon->icon()), m_configWidget->qchName->text());
    m_configWidget->qchTable->setItem(row, 0, itemName);
    QTableWidgetItem *itemPath = new QTableWidgetItem(m_configWidget->qchRequester->text());
    m_configWidget->qchTable->setItem(row, 1, itemPath);
    QTableWidgetItem *itemIconName = new QTableWidgetItem(m_configWidget->qchIcon->icon());
    m_configWidget->qchTable->setItem(row, 2, itemIconName);
    m_configWidget->qchTable->setCurrentCell(row, 0);
    emit changed(true);
}

void QtHelpConfig::modify()
{
    if(!checkQtHelpFile(true)){
        return;
    }
    if (!m_configWidget->qchTable->selectedItems().isEmpty()) {
        int row = m_configWidget->qchTable->selectedItems().at(0)->row();
        m_configWidget->qchTable->item(row, 0)->setIcon(KIcon(m_configWidget->qchIcon->icon()));
        m_configWidget->qchTable->item(row, 0)->setText(m_configWidget->qchName->text());
        m_configWidget->qchTable->item(row, 1)->setText(m_configWidget->qchRequester->text());
        m_configWidget->qchTable->item(row, 2)->setText(m_configWidget->qchIcon->icon());
        emit changed(true);
    }
}

bool QtHelpConfig::checkQtHelpFile(bool modify)
{
    QString qtHelpNamespace = QHelpEngineCore::namespaceName(m_configWidget->qchRequester->text());
    //verify if the file is valid and if there is a name
    if(m_configWidget->qchName->text().isEmpty()){
        KMessageBox::error(this, i18n("Name can't be empty."));
        return false;
    }
    if (qtHelpNamespace.isEmpty()) {
        // Open error message (not valid Qt Compressed Help file)
        KMessageBox::error(this, i18n("Qt Compressed Help file is not valid."));
        return false;
    }
    int modifyIndex = -1;
    if(modify){
        modifyIndex = m_configWidget->qchTable->currentRow();
    }
    // verify if it's the namespace it's not already in the list
    for(int i=0; i < m_configWidget->qchTable->rowCount(); i++) {
        if(i != modifyIndex){
            if(qtHelpNamespace == QHelpEngineCore::namespaceName(m_configWidget->qchTable->item(i,1)->text())){
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
    if (!m_configWidget->qchTable->selectedItems().isEmpty()) {
        int row = m_configWidget->qchTable->selectedItems().at(0)->row();
        m_configWidget->qchTable->removeRow(row);
        emit changed(true);
    }
}

void QtHelpConfig::up()
{
    if (!m_configWidget->qchTable->selectedItems().isEmpty()) {
        int row = m_configWidget->qchTable->selectedItems().at(0)->row();
        if (row > 0) {
            QTableWidgetItem *currentItemName = m_configWidget->qchTable->takeItem(row, 0);
            QTableWidgetItem *currentItemPath = m_configWidget->qchTable->takeItem(row, 1);
            QTableWidgetItem *currentItemIconName = m_configWidget->qchTable->takeItem(row, 2);
            QTableWidgetItem *previousItemName = m_configWidget->qchTable->takeItem(row-1, 0);
            QTableWidgetItem *previousItemPath = m_configWidget->qchTable->takeItem(row-1, 1);
            QTableWidgetItem *previousItemIconName = m_configWidget->qchTable->takeItem(row-1, 2);
            m_configWidget->qchTable->setItem(row, 0, previousItemName);
            m_configWidget->qchTable->setItem(row, 1, previousItemPath);
            m_configWidget->qchTable->setItem(row, 2, previousItemIconName);
            m_configWidget->qchTable->setItem(row-1, 0, currentItemName);
            m_configWidget->qchTable->setItem(row-1, 1, currentItemPath);
            m_configWidget->qchTable->setItem(row-1, 2, currentItemIconName);
            m_configWidget->qchTable->setCurrentCell(row-1, 0);
            emit changed(true);
        }
    }
}

void QtHelpConfig::down()
{
    if (!m_configWidget->qchTable->selectedItems().isEmpty()) {
        int row = m_configWidget->qchTable->selectedItems().at(0)->row();
        if (row + 1 < m_configWidget->qchTable->rowCount()) {
            QTableWidgetItem *currentItemName = m_configWidget->qchTable->takeItem(row, 0);
            QTableWidgetItem *currentItemPath = m_configWidget->qchTable->takeItem(row, 1);
            QTableWidgetItem *currentItemIconName = m_configWidget->qchTable->takeItem(row, 2);
            QTableWidgetItem *nextItemName = m_configWidget->qchTable->takeItem(row+1, 0);
            QTableWidgetItem *nextItemPath = m_configWidget->qchTable->takeItem(row+1, 1);
            QTableWidgetItem *nextItemIconName = m_configWidget->qchTable->takeItem(row+1, 2);
            m_configWidget->qchTable->setItem(row, 0, nextItemName);
            m_configWidget->qchTable->setItem(row, 1, nextItemPath);
            m_configWidget->qchTable->setItem(row, 2, nextItemIconName);
            m_configWidget->qchTable->setItem(row+1, 0, currentItemName);
            m_configWidget->qchTable->setItem(row+1, 1, currentItemPath);
            m_configWidget->qchTable->setItem(row+1, 2, currentItemIconName);
            m_configWidget->qchTable->setCurrentCell(row+1, 0);
            emit changed(true);
        }
    }
}


#include "qthelpconfig.moc"
