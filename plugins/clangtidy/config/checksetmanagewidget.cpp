/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "checksetmanagewidget.h"

// plugin
#include "checksetselectionlistmodel.h"
#include "checksetselectionmanager.h"
#include "debug.h"
// KDevPlatform
#include <util/scopeddialog.h>
// KF
#include <KLocalizedString>
// Qt
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QValidator>

using namespace KDevelop;

namespace ClangTidy {

class CheckSetNameValidator : public QValidator
{
    Q_OBJECT

public:
    explicit CheckSetNameValidator(CheckSetSelectionListModel* checkSetSelectionListModel,
                                   QObject* parent = nullptr);
    QValidator::State validate(QString& input, int& pos) const override;

private:
    const CheckSetSelectionListModel* const m_checkSetSelectionListModel;
};

CheckSetNameValidator::CheckSetNameValidator(CheckSetSelectionListModel* checkSetSelectionListModel,
                                           QObject* parent)
    : QValidator(parent)
    , m_checkSetSelectionListModel(checkSetSelectionListModel)
{
}

QValidator::State CheckSetNameValidator::validate(QString& input, int& pos) const
{
    Q_UNUSED(pos);

    if (input.isEmpty()) {
        return QValidator::Intermediate;
    }
    if (m_checkSetSelectionListModel->hasCheckSetSelection(input)) {
        return QValidator::Intermediate;
    }
    return QValidator::Acceptable;
}


class CheckSetNameEditor : public QDialog
{
    Q_OBJECT

public:
    explicit CheckSetNameEditor(CheckSetSelectionListModel* checkSetSelectionListModel,
                                const QString& defaultName, QWidget* parent = nullptr);

public:
    QString name() const;

private:
    void handleNameEdit(const QString& text);

private:
    CheckSetNameValidator* m_validator;
    QLineEdit* m_nameEdit;
    QPushButton* m_okButton;
};

CheckSetNameEditor::CheckSetNameEditor(CheckSetSelectionListModel* checkSetSelectionListModel,
                                       const QString& defaultName, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Enter Name of New Check Set"));

    auto* layout = new QVBoxLayout(this);

    auto* editLayout = new QHBoxLayout;

    auto* label = new QLabel(i18nc("@label:textbox", "Name:"));
    editLayout->addWidget(label);
    m_nameEdit = new QLineEdit;
    m_nameEdit->setClearButtonEnabled(true);
    editLayout->addWidget(m_nameEdit);
    layout->addLayout(editLayout);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_okButton = buttonBox->button(QDialogButtonBox::Ok);
    m_okButton->setEnabled(false);
    m_okButton->setDefault(true);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);

    m_validator = new CheckSetNameValidator(checkSetSelectionListModel, this);
    connect(m_nameEdit, &QLineEdit::textChanged, this, &CheckSetNameEditor::handleNameEdit);

    m_nameEdit->setText(defaultName);
    m_nameEdit->selectAll();
}

QString CheckSetNameEditor::name() const
{
    return m_nameEdit->text();
}

void CheckSetNameEditor::handleNameEdit(const QString& _text)
{
    int pos;
    QString text(_text);
    const bool isValidCheckSetSelectionName = (m_validator->validate(text, pos) == QValidator::Acceptable);

    m_okButton->setEnabled(isValidCheckSetSelectionName);
}


CheckSetManageWidget::CheckSetManageWidget(QWidget* parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);
}

void CheckSetManageWidget::setCheckSetSelectionManager(CheckSetSelectionManager* checkSetSelectionManager,
                                                       const CheckSet* checkSet)
{
    m_ui.enabledChecks->setCheckSet(checkSet);
    m_checkSetSelectionListModel = new CheckSetSelectionListModel(checkSetSelectionManager, this);
    m_ui.checkSetSelect->setModel(m_checkSetSelectionListModel);

    const auto defaultIndex = m_checkSetSelectionListModel->defaultCheckSetSelectionRow();
    m_ui.checkSetSelect->setCurrentIndex(defaultIndex);
    onSelectedCheckSetSelectionChanged(defaultIndex);

    connect(m_ui.cloneCheckSetSelectionButton, &QPushButton::clicked,
            this, &CheckSetManageWidget::cloneSelectedCheckSetSelection);
    connect(m_ui.addCheckSetSelectionButton, &QPushButton::clicked,
            this, &CheckSetManageWidget::addCheckSetSelection);
    connect(m_ui.removeCheckSetSelectionButton, &QPushButton::clicked,
            this, &CheckSetManageWidget::removeSelectedCheckSetSelection);
    connect(m_ui.setAsDefaultCheckSetSelectionButton, &QPushButton::clicked,
            this, &CheckSetManageWidget::setSelectedCheckSetSelectionAsDefault);
    connect(m_ui.editCheckSetSelectionNameButton, &QPushButton::clicked,
            this, &CheckSetManageWidget::editSelectedCheckSetSelectionName);
    connect(m_ui.checkSetSelect, QOverload<int>::of(&KComboBox::currentIndexChanged),
            this, &CheckSetManageWidget::onSelectedCheckSetSelectionChanged);

    connect(m_checkSetSelectionListModel, &CheckSetSelectionListModel::defaultCheckSetSelectionChanged,
            this, &CheckSetManageWidget::onDefaultCheckSetSelectionChanged);

    connect(m_checkSetSelectionListModel, &CheckSetSelectionListModel::rowsInserted,
            this, &CheckSetManageWidget::changed);
    connect(m_checkSetSelectionListModel, &CheckSetSelectionListModel::rowsRemoved,
            this, &CheckSetManageWidget::changed);
    connect(m_checkSetSelectionListModel, &CheckSetSelectionListModel::checkSetSelectionChanged,
            this, &CheckSetManageWidget::changed);
    connect(m_checkSetSelectionListModel, &CheckSetSelectionListModel::defaultCheckSetSelectionChanged,
            this, &CheckSetManageWidget::changed);

    connect(m_ui.enabledChecks, &CheckSelection::checksChanged,
            this, &CheckSetManageWidget::onEnabledChecksChanged);
}

void CheckSetManageWidget::setSelectedCheckSetSelectionAsDefault()
{
    const int selectedIndex = m_ui.checkSetSelect->currentIndex();
    m_checkSetSelectionListModel->setDefaultCheckSetSelection(selectedIndex);
}

void CheckSetManageWidget::reload()
{
    if (!m_checkSetSelectionListModel) {
        return;
    }

    const int currentIndexBefore = m_ui.checkSetSelect->currentIndex();
    const QString currentId = m_checkSetSelectionListModel->checkSetSelectionId(currentIndexBefore);
    m_checkSetSelectionListModel->reload();

    const int currentIndexAfter = m_checkSetSelectionListModel->row(currentId);
    m_ui.checkSetSelect->setCurrentIndex(currentIndexAfter);
}

void CheckSetManageWidget::store() const
{
    if (!m_checkSetSelectionListModel) {
        return;
    }

    m_checkSetSelectionListModel->store();
}

QString CheckSetManageWidget::askNewCheckSetSelectionName(const QString& defaultName)
{
    ScopedDialog<CheckSetNameEditor> dialog(m_checkSetSelectionListModel, defaultName, this);

    if (dialog->exec() != QDialog::Accepted) {
        return {};
    }

    return dialog->name();
}

void CheckSetManageWidget::editSelectedCheckSetSelectionName()
{
    const int currentIndex = m_ui.checkSetSelect->currentIndex();
    const auto currentCheckSetSelectionName = m_checkSetSelectionListModel->checkSetSelectionName(currentIndex);
    const auto checkSetSelectionName = askNewCheckSetSelectionName(currentCheckSetSelectionName);
    if (checkSetSelectionName.isEmpty()) {
        return;
    }

    m_checkSetSelectionListModel->setName(currentIndex, checkSetSelectionName);
}

void CheckSetManageWidget::addCheckSetSelection()
{
    const auto checkSetSelectionName = askNewCheckSetSelectionName(QString());
    if (checkSetSelectionName.isEmpty()) {
        return;
    }

    const int checkSetSelectionIndex = m_checkSetSelectionListModel->addCheckSetSelection(checkSetSelectionName);

    m_ui.checkSetSelect->setCurrentIndex(checkSetSelectionIndex);
    m_ui.enabledChecks->setFocus(Qt::OtherFocusReason);
}

void CheckSetManageWidget::cloneSelectedCheckSetSelection()
{
    const int currentIndex = m_ui.checkSetSelect->currentIndex();
    Q_ASSERT(currentIndex >= 0);
    const auto currentCheckSetSelectionName = m_checkSetSelectionListModel->checkSetSelectionName(currentIndex);
    // pass original name as starting name, as the user might want to enter a variant of it
    const auto checkSetSelectionName = askNewCheckSetSelectionName(currentCheckSetSelectionName);
    if (checkSetSelectionName.isEmpty()) {
        return;
    }

    const int checkSetSelectionIndex = m_checkSetSelectionListModel->cloneCheckSetSelection(checkSetSelectionName, currentIndex);

    m_ui.checkSetSelect->setCurrentIndex(checkSetSelectionIndex);
    m_ui.enabledChecks->setFocus(Qt::OtherFocusReason);
}

void CheckSetManageWidget::removeSelectedCheckSetSelection()
{
    const int selectedCheckSetSelectionIndex = m_ui.checkSetSelect->currentIndex();

    if (selectedCheckSetSelectionIndex == -1) {
        return;
    }

    m_checkSetSelectionListModel->removeCheckSetSelection(selectedCheckSetSelectionIndex);

    const int defaultCheckSetSelectionIndex = m_checkSetSelectionListModel->defaultCheckSetSelectionRow();
    m_ui.checkSetSelect->setCurrentIndex(defaultCheckSetSelectionIndex);
}

void CheckSetManageWidget::onDefaultCheckSetSelectionChanged(const QString& checkSetSelectionId)
{
    const int defaultCheckSetSelectionIndex = m_checkSetSelectionListModel->row(checkSetSelectionId);
    const int selectedCheckSetSelectionIndex = m_ui.checkSetSelect->currentIndex();
    const bool isDefaultCheckSetSelection = (defaultCheckSetSelectionIndex == selectedCheckSetSelectionIndex);

    m_ui.setAsDefaultCheckSetSelectionButton->setEnabled(!isDefaultCheckSetSelection);
}

void CheckSetManageWidget::onSelectedCheckSetSelectionChanged(int selectedCheckSetSelectionIndex)
{
    const bool isDefaultCheckSetSelection = (m_checkSetSelectionListModel->defaultCheckSetSelectionRow() == selectedCheckSetSelectionIndex);
    const bool isCheckSetSelectionSelected = (selectedCheckSetSelectionIndex != -1);

    m_ui.cloneCheckSetSelectionButton->setEnabled(isCheckSetSelectionSelected);
    m_ui.removeCheckSetSelectionButton->setEnabled(isCheckSetSelectionSelected);
    m_ui.editCheckSetSelectionNameButton->setEnabled(isCheckSetSelectionSelected);
    m_ui.setAsDefaultCheckSetSelectionButton->setEnabled(!isDefaultCheckSetSelection);

    m_ui.enabledChecks->blockSignals(true);
    const QString checks = m_checkSetSelectionListModel->checkSetSelectionAsString(selectedCheckSetSelectionIndex);
    m_ui.enabledChecks->setChecks(checks);
    m_ui.enabledChecks->setEnabled(isCheckSetSelectionSelected);
    m_ui.enabledChecks->blockSignals(false);
}

void CheckSetManageWidget::onEnabledChecksChanged(const QString& selection)
{
    const int selectedCheckSetSelectionIndex = m_ui.checkSetSelect->currentIndex();
    m_checkSetSelectionListModel->setSelection(selectedCheckSetSelectionIndex, selection);
}

}

#include "checksetmanagewidget.moc"
#include "moc_checksetmanagewidget.cpp"
