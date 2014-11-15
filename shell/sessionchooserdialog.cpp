/* This file is part of KDevelop

    Copyright 2008 Andreas Pakulat <apaku@gmx.de>
    Copyright 2010 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "sessionchooserdialog.h"
#include "sessioncontroller.h"
#include "core.h"

#include <QLineEdit>
#include <QPushButton>
#include <KLocalizedString>
#include <KMessageBox>
#include <QListView>
#include <QKeyEvent>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QVBoxLayout>

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
    m_deleteButton->setIcon(QIcon::fromTheme("edit-delete"));
    m_deleteButton->setToolTip(i18nc("@info", "Delete session"));
    m_deleteButton->hide();
    connect(m_deleteButton, &QPushButton::clicked, this, &SessionChooserDialog::deleteButtonPressed);

    m_deleteButtonTimer.setInterval(500);
    m_deleteButtonTimer.setSingleShot(true);
    connect(&m_deleteButtonTimer, &QTimer::timeout, this, &SessionChooserDialog::showDeleteButton);

    view->setMouseTracking(true);
    view->installEventFilter(this);
    filter->installEventFilter(this);
    connect(filter, &QLineEdit::textChanged, this, &SessionChooserDialog::filterTextChanged);

    setWindowTitle(i18n("Pick a Session"));

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Close);
    auto mainLayout = new QVBoxLayout(this);
    m_mainWidget = new QWidget(this);
    mainLayout->addWidget(m_mainWidget);

    QPushButton *okButton = m_buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::Key_Return);
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mainLayout->addWidget(m_buttonBox);
    okButton->setText(i18n("Run"));
    okButton->setIcon(QIcon::fromTheme("media-playback-start"));
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
            tooltip = i18n("Active session.\npid %1, app %2, host %3",
                           info.holderPid, info.holderApp, info.holderHostname);
            state = i18n("Running");
        }

        m_model->setData(m_model->index(row, 1),
                         !info.isRunning ? QIcon::fromTheme("") : QIcon::fromTheme("media-playback-start"),
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
    }
    if(object == m_filter && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down || keyEvent->key() == Qt::Key_Return) {
            QModelIndex currentIndex = m_view->selectionModel()->currentIndex();
            int selectRow = -1;
            switch (keyEvent->key()) {
            case Qt::Key_Up:
                if(!currentIndex.isValid()) {
                    selectRow = m_model->rowCount()-1;
                } else if(currentIndex.row()-1 >= 0) {
                    selectRow = currentIndex.row()-1;
                }
                break;
            case Qt::Key_Down:
                if(!currentIndex.isValid()) {
                    selectRow = 0;
                } else if(currentIndex.row()+1 < m_model->rowCount()) {
                    selectRow = currentIndex.row()+1;
                }
                break;
            case Qt::Key_Return:
                accept();
                return false;
            default:
                return false;
            }

            if (selectRow != -1) {
                    m_view->selectionModel()->setCurrentIndex(m_model->index(selectRow, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            }
            return true;
        }
    }

    return false;
}

QWidget* SessionChooserDialog::mainWidget() const
{
    return m_mainWidget;
}

void SessionChooserDialog::deleteButtonPressed()
{
    if(m_deleteCandidateRow == -1)
        return;

    QModelIndex index = m_model->index(m_deleteCandidateRow, 0);
    const QString uuid = m_model->data(index, Qt::DisplayRole).toString();

    TryLockSessionResult result = SessionController::tryLockSession( uuid );
    if( !result.lock ) {
        const QString errCaption = i18nc("@title", "Cannot Delete Session");
        QString errText = i18nc("@info", "<p>Cannot delete a locked session.");

        if( result.runInfo.holderPid != -1 ) {
            errText += i18nc("@info", "<p>The session is locked by %1 on %2 (PID %3).",
                                result.runInfo.holderApp, result.runInfo.holderHostname, result.runInfo.holderPid);
        }

        KMessageBox::error( this, errText, errCaption );
        return;
    }

    const QString text = i18nc("@info", "The session and all contained settings will be deleted. The projects will stay unaffected. Do you really want to continue?");
    const QString caption = i18nc("@title", "Delete Session");
    const KGuiItem deleteItem = KStandardGuiItem::del();
    const KGuiItem cancelItem = KStandardGuiItem::cancel();

    if(KMessageBox::warningYesNo(this, text, caption, deleteItem, cancelItem) == KMessageBox::Yes) {
        QModelIndex index = m_model->index(m_deleteCandidateRow, 0);
        const QString uuid = m_model->data(index, Qt::DisplayRole).toString();

        SessionController::deleteSessionFromDisk(result.lock);

        m_model->removeRows( m_deleteCandidateRow, 1 );
        m_deleteCandidateRow = -1;
    }
}

