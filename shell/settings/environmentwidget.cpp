/* This file is part of KDevelop
Copyright 2006 Adam Treat <treat@kde.org>
Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
Copyright 2008 Andreas Pakuat <apaku@gmx.de>
Copyright 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#include "environmentwidget.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QValidator>

#include <KLocalizedString>

#include "environmentprofilelistmodel.h"
#include "environmentprofilemodel.h"
#include "placeholderitemproxymodel.h"
#include "../debug.h"

using namespace KDevelop;


class ProfileNameValidator : public QValidator
{
    Q_OBJECT

public:
    explicit ProfileNameValidator(EnvironmentProfileListModel* environmentProfileListModel, QObject* parent = nullptr);
    QValidator::State validate(QString& input, int& pos) const override;

private:
    const EnvironmentProfileListModel* const m_environmentProfileListModel;
};

ProfileNameValidator::ProfileNameValidator(EnvironmentProfileListModel* environmentProfileListModel,
                                           QObject* parent)
    : QValidator(parent)
    , m_environmentProfileListModel(environmentProfileListModel)
{
}

QValidator::State ProfileNameValidator::validate(QString& input, int& pos) const
{
    Q_UNUSED(pos);

    if (input.isEmpty()) {
        return QValidator::Intermediate;
    }
    if (m_environmentProfileListModel->hasProfile(input)) {
        return QValidator::Intermediate;
    }
    return QValidator::Acceptable;
}



EnvironmentWidget::EnvironmentWidget( QWidget *parent )
    : QWidget(parent)
    , m_environmentProfileListModel(new EnvironmentProfileListModel(this))
    , m_environmentProfileModel(new EnvironmentProfileModel(m_environmentProfileListModel, this))
    , m_proxyModel(new QSortFilterProxyModel(this))
{
    // setup ui
    ui.setupUi( this );

    ui.profileSelect->setModel(m_environmentProfileListModel);
    m_proxyModel->setSourceModel(m_environmentProfileModel);

    PlaceholderItemProxyModel* topProxyModel  = new PlaceholderItemProxyModel(this);
    topProxyModel->setSourceModel(m_proxyModel);
    topProxyModel->setColumnHint(0, i18n("Enter variable..."));
    connect(topProxyModel, &PlaceholderItemProxyModel::dataInserted, this, &EnvironmentWidget::onVariableInserted);

    ui.variableTable->setModel( topProxyModel );
    ui.variableTable->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
    ui.variableTable->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::Stretch );
    ui.removeVariableButton->setShortcut(Qt::Key_Delete);

    connect(ui.removeVariableButton, &QPushButton::clicked,
            this, &EnvironmentWidget::removeSelectedVariables);
    connect(ui.batchModeEditButton, &QPushButton::clicked,
            this, &EnvironmentWidget::batchModeEditButtonClicked);

    connect(ui.cloneProfileButton, &QPushButton::clicked, this, &EnvironmentWidget::cloneSelectedProfile);
    connect(ui.addProfileButton, &QPushButton::clicked, this, &EnvironmentWidget::addProfile);
    connect(ui.removeProfileButton, &QPushButton::clicked, this, &EnvironmentWidget::removeSelectedProfile);
    connect(ui.setAsDefaultProfileButton, &QPushButton::clicked, this, &EnvironmentWidget::setSelectedProfileAsDefault);
    connect(ui.profileSelect, static_cast<void(KComboBox::*)(int)>(&KComboBox::currentIndexChanged),
            this, &EnvironmentWidget::onSelectedProfileChanged);

    connect(m_environmentProfileListModel, &EnvironmentProfileListModel::defaultProfileChanged,
            this, &EnvironmentWidget::onDefaultProfileChanged);
    connect(m_environmentProfileListModel, &EnvironmentProfileListModel::rowsInserted,
            this, &EnvironmentWidget::changed);
    connect(m_environmentProfileListModel, &EnvironmentProfileListModel::rowsRemoved,
            this, &EnvironmentWidget::changed);
    connect(m_environmentProfileListModel, &EnvironmentProfileListModel::defaultProfileChanged,
            this, &EnvironmentWidget::changed);

    connect(ui.variableTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &EnvironmentWidget::updateDeleteVariableButton);
    connect(m_environmentProfileModel, &EnvironmentProfileModel::rowsInserted,
            this, &EnvironmentWidget::updateDeleteVariableButton);
    connect(m_environmentProfileModel, &EnvironmentProfileModel::rowsRemoved,
            this, &EnvironmentWidget::updateDeleteVariableButton);
    connect(m_environmentProfileModel, &EnvironmentProfileModel::modelReset,
            this, &EnvironmentWidget::updateDeleteVariableButton);
    connect(m_environmentProfileModel, &EnvironmentProfileModel::dataChanged,
            this, &EnvironmentWidget::changed);
    connect(m_environmentProfileModel, &EnvironmentProfileModel::rowsInserted,
            this, &EnvironmentWidget::changed);
    connect(m_environmentProfileModel, &EnvironmentProfileModel::rowsRemoved,
            this, &EnvironmentWidget::changed);
}

void EnvironmentWidget::selectProfile(const QString& profileName)
{
    const int profileIndex = m_environmentProfileListModel->profileIndex(profileName);
    if (profileIndex < 0) {
        return;
    }
    ui.profileSelect->setCurrentIndex(profileIndex);
}

void EnvironmentWidget::updateDeleteVariableButton()
{
    const auto selectedRows = ui.variableTable->selectionModel()->selectedRows();
    ui.removeVariableButton->setEnabled(!selectedRows.isEmpty());
}

void EnvironmentWidget::setSelectedProfileAsDefault()
{
    const int selectedIndex = ui.profileSelect->currentIndex();
    m_environmentProfileListModel->setDefaultProfile(selectedIndex);
}

void EnvironmentWidget::loadSettings( KConfig* config )
{
    qCDebug(SHELL) << "Loading profiles from config";
    m_environmentProfileListModel->loadFromConfig(config);

    const int defaultProfileIndex = m_environmentProfileListModel->defaultProfileIndex();
    ui.profileSelect->setCurrentIndex(defaultProfileIndex);
}

void EnvironmentWidget::saveSettings( KConfig* config )
{
    m_environmentProfileListModel->saveToConfig(config);
}

void EnvironmentWidget::defaults( KConfig* config )
{
    loadSettings( config );
}

QString EnvironmentWidget::askNewProfileName(const QString& defaultName)
{
    QDialog dialog(this);
    dialog.setWindowTitle(i18n("Enter Name of New Environment Profile"));

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    auto editLayout = new QHBoxLayout;

    auto label = new QLabel(i18n("Name:"));
    editLayout->addWidget(label);
    auto edit = new QLineEdit;
    editLayout->addWidget(edit);
    layout->addLayout(editLayout);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    auto okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setEnabled(false);
    okButton->setDefault(true);
    dialog.connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    dialog.connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttonBox);

    auto validator = new ProfileNameValidator(m_environmentProfileListModel, &dialog);
    connect(edit, &QLineEdit::textChanged, validator, [validator, okButton](const QString& text) {
        int pos;
        QString t(text);
        const bool isValidProfileName = (validator->validate(t, pos) == QValidator::Acceptable);
        okButton->setEnabled(isValidProfileName);
    });

    edit->setText(defaultName);
    edit->selectAll();

    if (dialog.exec() != QDialog::Accepted) {
        return {};
    }

    return edit->text();
}

void EnvironmentWidget::removeSelectedVariables()
{
    const auto selectedRows = ui.variableTable->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        return;
    }

    QStringList variables;
    for (const auto& idx : selectedRows) {
        const QString variable = idx.data(EnvironmentProfileModel::VariableRole).toString();
        variables << variable;
    }

    m_environmentProfileModel->removeVariables(variables);
}

void EnvironmentWidget::onVariableInserted(int column, const QVariant& value)
{
    Q_UNUSED(column);
    m_environmentProfileModel->addVariable(value.toString(), QString());
}

void EnvironmentWidget::batchModeEditButtonClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle( i18n( "Batch Edit Mode" ) );

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QTextEdit *edit = new QTextEdit;
    edit->setPlaceholderText(QStringLiteral("VARIABLE1=VALUE1\nVARIABLE2=VALUE2"));
    QString text;
    for (int i = 0; i < m_proxyModel->rowCount(); ++i) {
        const auto variable = m_proxyModel->index(i, EnvironmentProfileModel::VariableColumn).data().toString();
        const auto value = m_proxyModel->index(i, EnvironmentProfileModel::ValueColumn).data().toString();
        text.append(QStringLiteral("%1=%2\n").arg(variable, value));
    }
    edit->setText(text);
    layout->addWidget( edit );

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    auto okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    dialog.connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    dialog.connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttonBox);

    dialog.resize(600, 400);

    if ( dialog.exec() != QDialog::Accepted ) {
        return;
    }

    m_environmentProfileModel->setVariablesFromString(edit->toPlainText());
}

void EnvironmentWidget::addProfile()
{
    const auto profileName = askNewProfileName(QString());
    if (profileName.isEmpty()) {
        return;
    }

    const int profileIndex = m_environmentProfileListModel->addProfile(profileName);

    ui.profileSelect->setCurrentIndex(profileIndex);
    ui.variableTable->setFocus(Qt::OtherFocusReason);
}

void EnvironmentWidget::cloneSelectedProfile()
{
    const int currentIndex = ui.profileSelect->currentIndex();
    const auto currentProfileName = m_environmentProfileListModel->profileName(currentIndex);
    // pass original name as starting name, as the user might want to enter a variant of it
    const auto profileName = askNewProfileName(currentProfileName);
    if (profileName.isEmpty()) {
        return;
    }

    const int profileIndex = m_environmentProfileListModel->cloneProfile(profileName, currentProfileName);

    ui.profileSelect->setCurrentIndex(profileIndex);
    ui.variableTable->setFocus(Qt::OtherFocusReason);
}

void EnvironmentWidget::removeSelectedProfile()
{
    if (ui.profileSelect->count() <= 1) {
        return;
    }

    const int selectedProfileIndex = ui.profileSelect->currentIndex();

    m_environmentProfileListModel->removeProfile(selectedProfileIndex);

    const int defaultProfileIndex = m_environmentProfileListModel->defaultProfileIndex();
    ui.profileSelect->setCurrentIndex(defaultProfileIndex);
}

void EnvironmentWidget::onDefaultProfileChanged(int defaultProfileIndex)
{
    const int selectedProfileIndex = ui.profileSelect->currentIndex();
    const bool isDefaultProfile = (defaultProfileIndex == selectedProfileIndex);

    ui.removeProfileButton->setEnabled(ui.profileSelect->count() > 1 && !isDefaultProfile);
    ui.setAsDefaultProfileButton->setEnabled(!isDefaultProfile);
}

void EnvironmentWidget::onSelectedProfileChanged(int selectedProfileIndex)
{
    const auto selectedProfileName = m_environmentProfileListModel->profileName(selectedProfileIndex);
    m_environmentProfileModel->setCurrentProfile(selectedProfileName);

    const bool isDefaultProfile = (m_environmentProfileListModel->defaultProfileIndex() == selectedProfileIndex);

    ui.removeProfileButton->setEnabled(ui.profileSelect->count() > 1 && !isDefaultProfile);
    ui.setAsDefaultProfileButton->setEnabled(!isDefaultProfile);
}

#include "environmentwidget.moc"
