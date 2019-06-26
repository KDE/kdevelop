/* This file is part of KDevelop
    Copyright 2008 Alexander Dymo <adymo@kdevelop.org>

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
#include "projectfilterconfigpage.h"

#include <QKeyEvent>

#include <KMessageWidget>
#include <KLocalizedString>

#include <interfaces/iproject.h>
#include <project/projectmodel.h>

#include "ui_projectfiltersettings.h"

#include <debug.h>
#include "filtermodel.h"
#include "comboboxdelegate.h"
#include "projectfilterprovider.h"

using namespace KDevelop;

ProjectFilterConfigPage::ProjectFilterConfigPage(ProjectFilterProvider* provider, const ProjectConfigOptions& options, QWidget* parent)
    : ProjectConfigPage<ProjectFilterSettings>(provider, options, parent)
    , m_model(new FilterModel(this))
    , m_projectFilterProvider(provider)
    , m_ui(new Ui::ProjectFilterSettings)
{
    m_ui->setupUi(this);

    m_ui->messageWidget->hide();

    m_ui->filters->setSelectionMode(QAbstractItemView::SingleSelection);
    m_ui->filters->setModel(m_model);
    m_ui->filters->setRootIsDecorated(false);
    m_ui->filters->header()->setSectionResizeMode(FilterModel::Pattern, QHeaderView::Stretch);
    m_ui->filters->header()->setSectionResizeMode(FilterModel::Targets, QHeaderView::ResizeToContents);
    m_ui->filters->header()->setSectionResizeMode(FilterModel::Inclusive, QHeaderView::ResizeToContents);
    m_ui->filters->setItemDelegateForColumn(FilterModel::Targets,
        new ComboBoxDelegate(QVector<ComboBoxDelegate::Item>{
                {i18n("Files"),             static_cast<int>(Filter::Files)},
                {i18n("Folders"),           static_cast<int>(Filter::Folders)},
                {i18n("Files and Folders"), static_cast<int>(Filter::Folders | Filter::Files)}}
            , this));
    m_ui->filters->setItemDelegateForColumn(FilterModel::Inclusive,
        new ComboBoxDelegate(QVector<ComboBoxDelegate::Item>{
                {i18n("Exclude"), false},
                {i18n("Include"), true}}
            , this));
    m_ui->filters->installEventFilter(this);
    m_ui->filters->setDragEnabled(true);
    m_ui->filters->setDragDropMode(QAbstractItemView::InternalMove);
    m_ui->filters->setAutoScroll(true);

    reset();
    selectionChanged();

    connect(m_ui->filters->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &ProjectFilterConfigPage::selectionChanged);
    connect(this, &ProjectFilterConfigPage::changed, this, &ProjectFilterConfigPage::selectionChanged);
    connect(m_model, &FilterModel::dataChanged, this, &ProjectFilterConfigPage::emitChanged);
    connect(m_model, &FilterModel::rowsInserted, this, &ProjectFilterConfigPage::emitChanged);
    connect(m_model, &FilterModel::rowsRemoved, this, &ProjectFilterConfigPage::emitChanged);
    connect(m_model, &FilterModel::modelReset, this, &ProjectFilterConfigPage::emitChanged);
    connect(m_model, &FilterModel::rowsMoved, this, &ProjectFilterConfigPage::emitChanged);

    connect(m_ui->add, &QPushButton::clicked, this, &ProjectFilterConfigPage::add);
    connect(m_ui->remove, &QPushButton::clicked, this, &ProjectFilterConfigPage::remove);
    connect(m_ui->moveUp, &QPushButton::clicked, this, &ProjectFilterConfigPage::moveUp);
    connect(m_ui->moveDown, &QPushButton::clicked, this, &ProjectFilterConfigPage::moveDown);
}

ProjectFilterConfigPage::~ProjectFilterConfigPage()
{
}

void ProjectFilterConfigPage::apply()
{
    ProjectConfigPage::apply();
    writeFilters(m_model->filters(), project()->projectConfiguration());
    m_projectFilterProvider->updateProjectFilters(project());
}

void ProjectFilterConfigPage::reset()
{
    ProjectConfigPage::reset();
    m_model->setFilters(readFilters(project()->projectConfiguration()));
}

void ProjectFilterConfigPage::defaults()
{
    ProjectConfigPage::defaults();
    m_model->setFilters(defaultFilters());
}

bool ProjectFilterConfigPage::eventFilter(QObject* object, QEvent* event)
{
    if (object == m_ui->filters && event->type() == QEvent::KeyRelease) {
        auto* key = static_cast<QKeyEvent*>(event);
        if (key->key() == Qt::Key_Delete && key->modifiers() == Qt::NoModifier && m_ui->filters->currentIndex().isValid()) {
            // workaround https://bugs.kde.org/show_bug.cgi?id=324451
            // there is no other way I see to figure out whether an editor is showing...
            auto* editor = m_ui->filters->viewport()->findChild<QWidget*>();
            if (!editor || !editor->isVisible()) {
                // editor is not showing
                remove();
                return true; // eat event
            }
        }
    }

    return ProjectConfigPage::eventFilter(object, event);
}

void ProjectFilterConfigPage::selectionChanged()
{
    bool hasSelection = m_ui->filters->currentIndex().isValid();
    int row = -1;
    if (hasSelection) {
        row = m_ui->filters->currentIndex().row();
    }
    m_ui->remove->setEnabled(hasSelection);

    m_ui->moveDown->setEnabled(hasSelection && row != m_model->rowCount() - 1);
    m_ui->moveUp->setEnabled(hasSelection && row != 0);
}

void ProjectFilterConfigPage::add()
{
    m_model->insertRows(m_model->rowCount(), 1);
    const QModelIndex index = m_model->index(m_model->rowCount() - 1, FilterModel::Pattern, QModelIndex());
    m_ui->filters->setCurrentIndex(index);
    m_ui->filters->edit(index);
}

void ProjectFilterConfigPage::remove()
{
    Q_ASSERT(m_ui->filters->currentIndex().isValid());
    m_model->removeRows(m_ui->filters->currentIndex().row(), 1);
}

void ProjectFilterConfigPage::moveUp()
{
    Q_ASSERT(m_ui->filters->currentIndex().isValid());
    m_model->moveFilterUp(m_ui->filters->currentIndex().row());
}

void ProjectFilterConfigPage::moveDown()
{
    Q_ASSERT(m_ui->filters->currentIndex().isValid());
    m_model->moveFilterDown(m_ui->filters->currentIndex().row());
}

void ProjectFilterConfigPage::checkFilters()
{
    // check for errors, only show one error at once
    QString errorText;
    const auto filters = m_model->filters();
    for (const Filter& filter : filters) {
        const QString &pattern = filter.pattern.pattern();
        if (pattern.isEmpty()) {
            errorText = i18n("A filter with an empty pattern will match all items. Use <code>\"*\"</code> to make this explicit.");
            break;
        } else if (pattern.endsWith(QLatin1Char('/')) && filter.targets == Filter::Files) {
            errorText = i18n("A filter ending on <code>\"/\"</code> can never match a file.");
            break;
        }
    }

    if (!errorText.isEmpty()) {
        m_ui->messageWidget->setMessageType(KMessageWidget::Error);
        m_ui->messageWidget->setText(errorText);
        m_ui->messageWidget->animatedShow();
    } else {
        m_ui->messageWidget->animatedHide();
    }
}

void ProjectFilterConfigPage::emitChanged()
{
    checkFilters();

    emit changed();
}

QString ProjectFilterConfigPage::fullName() const
{
    return i18n("Configure Project Filter");
}

QIcon ProjectFilterConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("view-filter"));
}

QString ProjectFilterConfigPage::name() const
{
    return i18n("Project Filter");
}
