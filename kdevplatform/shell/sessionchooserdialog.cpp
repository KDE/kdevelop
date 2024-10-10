/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2010 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sessionchooserdialog.h"
#include "sessioncontroller.h"
#include "core.h"

#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KMessageBox>

using namespace KDevelop;

SessionChooserDialog::SessionChooserDialog(QListView* view, QAbstractItemModel* model, QLineEdit* filter)
    : m_view(view), m_model(model), m_filter(filter), m_deleteCandidateRow(-1)
{
    m_updateStateTimer.setInterval(5000);
    m_updateStateTimer.setSingleShot(false);
    m_updateStateTimer.start();
    connect(&m_updateStateTimer, &QTimer::timeout, this, &SessionChooserDialog::updateState);
    connect(view, &QListView::doubleClicked, this, &SessionChooserDialog::doubleClicked);
    connect(view, &QListView::entered, this, &SessionChooserDialog::itemEntered);

    m_deleteButton = new QPushButton(view->viewport());
    m_deleteButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    m_deleteButton->setToolTip(i18nc("@info:tooltip", "Delete session"));
    m_deleteButton->hide();
    connect(m_deleteButton, &QPushButton::clicked, this, &SessionChooserDialog::deleteButtonPressed);

    m_deleteButtonTimer.setInterval(500);
    m_deleteButtonTimer.setSingleShot(true);
    connect(&m_deleteButtonTimer, &QTimer::timeout, this, &SessionChooserDialog::showDeleteButton);

    view->setMouseTracking(true);
    view->installEventFilter(this);
    filter->installEventFilter(this);
    connect(filter, &QLineEdit::textChanged, this, &SessionChooserDialog::filterTextChanged);

    setWindowTitle(i18nc("@title:window", "Pick a Session"));

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Close);
    auto mainLayout = new QVBoxLayout(this);
    m_mainWidget = new QWidget(this);
    mainLayout->addWidget(m_mainWidget);

    QPushButton *okButton = m_buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::Key_Return);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SessionChooserDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SessionChooserDialog::reject);
    mainLayout->addWidget(m_buttonBox);
    okButton->setText(i18nc("@action:button", "Run"));
    okButton->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
}

void SessionChooserDialog::filterTextChanged()
{
    m_view->selectionModel()->setCurrentIndex(m_model->index(0, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    bool enabled = m_view->model()->rowCount(QModelIndex())>0;
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
    m_deleteButton->setVisible(false);
}

void SessionChooserDialog::doubleClicked(const QModelIndex& index)
{
    if(m_model->flags(index) & Qt::ItemIsEnabled)
        accept();
}

void SessionChooserDialog::updateState() {
    // Sometimes locking may take some time, so we stop the timer, to prevent an 'avalanche' of events
    m_updateStateTimer.stop();
    for(int row = 0; row < m_model->rowCount(); ++row)
    {
        QString session = m_model->index(row, 0).data().toString();

        if(session.isEmpty()) //create new session
            continue;

        QString state, tooltip;
        SessionRunInfo info = SessionController::sessionRunInfo(session);
        if(info.isRunning)
        {
            tooltip = i18nc("@info:tooltip", "Active session.\npid %1, app %2, host %3",
                           info.holderPid, info.holderApp, info.holderHostname);
            state = i18n("Running");
        }

        m_model->setData(m_model->index(row, 1),
                         !info.isRunning ? QIcon() : QIcon::fromTheme(QStringLiteral("media-playback-start")),
                         Qt::DecorationRole);
        m_model->setData(m_model->index(row, 1), tooltip, Qt::ToolTipRole);
        m_model->setData(m_model->index(row, 2), state, Qt::DisplayRole);
    }

    m_updateStateTimer.start();
}

void SessionChooserDialog::itemEntered(const QModelIndex& index)
{
    // The last row says "Create new session", we don't want to delete that
    if(index.row() == m_model->rowCount()-1) {
        m_deleteButton->hide();
        m_deleteButtonTimer.stop();
        return;
    }

    // align the delete-button to stay on the right border of the item
    // we need the right most column's index
    QModelIndex in = m_model->index( index.row(), 1 );
    const QRect rect = m_view->visualRect(in);
    m_deleteButton->resize(rect.height(), rect.height());

    QPoint p(rect.right() - m_deleteButton->size().width(), rect.top()+rect.height()/2-m_deleteButton->height()/2);
    m_deleteButton->move(p);

    m_deleteCandidateRow = index.row();
    m_deleteButtonTimer.start();
}

void SessionChooserDialog::showDeleteButton()
{
    m_deleteButton->show();
}

bool SessionChooserDialog::eventFilter(QObject* object, QEvent* event)
{
    if(object == m_view && event->type() == QEvent::Leave ) {
        m_deleteButtonTimer.stop();
        m_deleteButton->hide();
        // don't eat the event, pass on
   } else if (object == m_filter && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return) {
            accept();
            // don't eat the event, pass on
        } else if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down) {
            QModelIndex currentIndex = m_view->selectionModel()->currentIndex();
            int selectRow = -1;
            if (keyEvent->key() == Qt::Key_Up) {
                if(!currentIndex.isValid()) {
                    selectRow = m_model->rowCount()-1;
                } else if(currentIndex.row()-1 >= 0) {
                    selectRow = currentIndex.row()-1;
                }
            } else {
                if(!currentIndex.isValid()) {
                    selectRow = 0;
                } else if(currentIndex.row()+1 < m_model->rowCount()) {
                    selectRow = currentIndex.row()+1;
                }
            }

            if (selectRow != -1) {
                    m_view->selectionModel()->setCurrentIndex(m_model->index(selectRow, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            }
            return true; // eat event
        }
    }

    return QDialog::eventFilter(object, event);
}

QWidget* SessionChooserDialog::mainWidget() const
{
    return m_mainWidget;
}

void SessionChooserDialog::deleteButtonPressed()
{
    if(m_deleteCandidateRow == -1)
        return;

    QModelIndex uuidIndex = m_model->index(m_deleteCandidateRow, 0);
    QModelIndex sessionNameIndex = m_model->index(m_deleteCandidateRow, 3);
    const QString uuid = m_model->data(uuidIndex, Qt::DisplayRole).toString();
    const QString sessionName = m_model->data(sessionNameIndex, Qt::DisplayRole).toString();

    TryLockSessionResult result = SessionController::tryLockSession( uuid );
    if( !result.lock ) {
        const QString errCaption = i18nc("@title:window", "Cannot Delete Session");
        QString errText = i18nc("@info", "<p>Cannot delete a locked session.");

        if( result.runInfo.holderPid != -1 ) {
            errText += i18nc("@info", "<p>The session <b>%1</b> is locked by %2 on %3 (PID %4).",
                                sessionName, result.runInfo.holderApp, result.runInfo.holderHostname, result.runInfo.holderPid);
        }

        KMessageBox::error( this, errText, errCaption );
        return;
    }

    const QString text = i18nc("@info", "The session <b>%1</b> and all contained settings will be deleted. The projects will stay unaffected. Do you really want to continue?", sessionName);
    const QString caption = i18nc("@title:window", "Delete Session");
    const KGuiItem deleteItem = KStandardGuiItem::del();
    const KGuiItem cancelItem = KStandardGuiItem::cancel();

    if (KMessageBox::warningTwoActions(this, text, caption, deleteItem, cancelItem) == KMessageBox::PrimaryAction) {
        SessionController::deleteSessionFromDisk(result.lock);

        m_model->removeRows( m_deleteCandidateRow, 1 );
        m_deleteCandidateRow = -1;
    }
}

#include "moc_sessionchooserdialog.cpp"
