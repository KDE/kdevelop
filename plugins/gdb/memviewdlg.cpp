/*
    SPDX-FileCopyrightText: 1999 John Birch <jbb@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Vladimir Prus <ghost@cs.msu.su>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "memviewdlg.h"

#include "dbgglobal.h"
#include "debugsession.h"
#include "mi/micommand.h"

#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>

#include <KLocalizedString>

#include <Okteta/ByteArrayColumnView>
#include <Okteta/ByteArrayModel>

#include <QAction>
#include <QActionGroup>
#include <QContextMenuEvent>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QIcon>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QToolBox>
#include <QVBoxLayout>

#include <cctype>

using KDevMI::MI::CommandType;

namespace KDevMI
{
namespace GDB
{

/** Container for controls that select memory range.
     *
    The memory range selection is embedded into memory view widget,
    it's not a standalone dialog. However, we want to have easy way
    to hide/show all controls, so we group them in this class.
*/
class MemoryRangeSelector : public QWidget
{
        Q_OBJECT
    public:
        QLineEdit* startAddressLineEdit;
        QLineEdit* amountLineEdit;
        QPushButton* okButton;
        QPushButton* cancelButton;

    explicit MemoryRangeSelector(QWidget* parent)
    : QWidget(parent)
    {
        auto* l = new QVBoxLayout(this);

        // Form layout: labels + address field
        auto formLayout = new QFormLayout();
        l->addLayout(formLayout);

        startAddressLineEdit = new QLineEdit(this);
        formLayout->addRow(i18nc("@label:textbox", "Start:"), startAddressLineEdit);

        amountLineEdit = new QLineEdit(this);
        formLayout->addRow(i18nc("@label:textbox", "Amount:"), amountLineEdit);

        auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, this);
        l->addWidget(buttonBox);

        okButton = buttonBox->button(QDialogButtonBox::Ok);
        cancelButton = buttonBox->button(QDialogButtonBox::Cancel);

        setLayout(l);

        connect(startAddressLineEdit, &QLineEdit::returnPressed, okButton, [this]() {
                okButton->animateClick();
        });

        connect(amountLineEdit, &QLineEdit::returnPressed, okButton, [this]() {
                okButton->animateClick();
        });
    }
};

MemoryView::MemoryView(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(i18nc("@title:window", "Memory View"));

    initWidget();

    auto debugController = KDevelop::ICore::self()->debugController();
    Q_ASSERT(debugController);

    connect(debugController, &KDevelop::IDebugController::currentSessionChanged,
            this, &MemoryView::currentSessionChanged);
    currentSessionChanged(debugController->currentSession());
}

void MemoryView::currentSessionChanged(KDevelop::IDebugSession* s)
{
    auto *session = qobject_cast<DebugSession*>(s);
    if (!session) {
        m_appHasStarted = false;
        enableOrDisable();
        return;
    }

    connect(session, &DebugSession::debuggerStateChanged, this, [this](DBGStateFlags oldState, DBGStateFlags newState) {
        Q_UNUSED(oldState);
        debuggerStateChanged(newState);
    });
    debuggerStateChanged(session->debuggerState());
}

void MemoryView::initWidget()
{
    auto *l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);

    m_memViewModel = new Okteta::ByteArrayModel(0, -1, this);
    m_memViewView = new Okteta::ByteArrayColumnView(this);
    m_memViewView->setByteArrayModel(m_memViewModel);

    m_memViewModel->setReadOnly(false);
    m_memViewView->setReadOnly(false);
    m_memViewView->setOverwriteMode(true);
    m_memViewView->setOverwriteOnly(true);
    m_memViewModel->setAutoDelete(false);

    m_memViewView->setValueCoding( Okteta::ByteArrayColumnView::HexadecimalCoding );
    m_memViewView->setNoOfGroupedBytes(4);
    m_memViewView->setByteSpacingWidth(2);
    m_memViewView->setGroupSpacingWidth(12);
    m_memViewView->setLayoutStyle(Okteta::AbstractByteArrayView::FullSizeLayoutStyle);


    m_memViewView->setShowsNonprinting(false);
    m_memViewView->setSubstituteChar(QLatin1Char('*'));

    m_memViewView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_memViewView, &QWidget::customContextMenuRequested, this, &MemoryView::memoryViewContextMenuRequested);

    m_rangeSelector = new MemoryRangeSelector(this);
    l->addWidget(m_rangeSelector);

    connect(m_rangeSelector->okButton, &QPushButton::clicked, this, &MemoryView::changeMemoryRange);
    connect(m_rangeSelector->cancelButton, &QPushButton::clicked, this, &MemoryView::hideRangeDialog);
    connect(m_rangeSelector->startAddressLineEdit, &QLineEdit::textChanged, this, &MemoryView::enableOrDisable);

    l->addWidget(m_memViewView);
}

void MemoryView::debuggerStateChanged(DBGStateFlags state)
{
    m_appHasStarted = !state.testFlag(s_appNotStarted);
    enableOrDisable();
}

void MemoryView::hideRangeDialog()
{
    m_rangeSelector->hide();
}

void MemoryView::changeMemoryRange()
{
    auto *session = qobject_cast<DebugSession*>(
        KDevelop::ICore::self()->debugController()->currentSession());
    if (!session) return;

    QString amount = m_rangeSelector->amountLineEdit->text();
    if(amount.isEmpty())
        amount = QStringLiteral("sizeof(%1)").arg(m_rangeSelector->startAddressLineEdit->text());

    session->addCommand(std::make_unique<MI::ExpressionValueCommand>(amount, this, &MemoryView::sizeComputed));
}

void MemoryView::sizeComputed(const QString& size)
{
    addReadMemoryCommand(QLatin1String{"%1 x 1 1 %2"}.arg(m_rangeSelector->startAddressLineEdit->text(), size));
}

void MemoryView::addReadMemoryCommand(const QString& arguments)
{
    auto *session = qobject_cast<DebugSession*>(
        KDevelop::ICore::self()->debugController()->currentSession());
    if (!session) return;

    session->addCommand(MI::DataReadMemory, arguments, this, &MemoryView::memoryRead);
}

void MemoryView::memoryRead(const MI::ResultRecord& r)
{
    const MI::Value& content = r[QStringLiteral("memory")][0][QStringLiteral("data")];
    bool startStringConverted;
    m_memStart = r[QStringLiteral("addr")].literal().toULongLong(&startStringConverted, 16);
    m_memData.resize(content.size());

    m_memStartStr = m_rangeSelector->startAddressLineEdit->text();
    m_memAmountStr = m_rangeSelector->amountLineEdit->text();

    setWindowTitle(i18np("%2 (1 byte)","%2 (%1 bytes)",m_memData.size(),m_memStartStr));
    emit captionChanged(windowTitle());

    for(int i = 0; i < content.size(); ++i)
    {
        m_memData[i] = content[i].literal().toInt(nullptr, 16);
    }

    m_memViewModel->setData(reinterpret_cast<Okteta::Byte*>(m_memData.data()), m_memData.size());

    hideRangeDialog();
}


void MemoryView::memoryEdited(int start, int end)
{
    auto *session = qobject_cast<DebugSession*>(
        KDevelop::ICore::self()->debugController()->currentSession());
    if (!session) return;

    for (auto i = start; i < end; ++i) {
        session->addCommand(MI::GdbSet,
                QStringLiteral("*(char*)(%1 + %2) = %3")
                    .arg(m_memStart)
                    .arg(i)
                    .arg(QString::number(m_memData[i])));
    }
}

void MemoryView::memoryViewContextMenuRequested(const QPoint& viewportPosition)
{
    // Add our actions also to the context menu of m_memViewView because MemoryView's
    // own context menu is difficult to trigger when the range selector is hidden.
    auto* menu = m_memViewView->createStandardContextMenu(viewportPosition);
    if (!menu) {
        menu = new QMenu(m_memViewView);
    } else if (!menu->isEmpty()) {
        menu->addSeparator();
    }
    addActionsAndShowContextMenu(menu, m_memViewView->viewport()->mapToGlobal(viewportPosition));
}

void MemoryView::contextMenuEvent(QContextMenuEvent *e)
{
    addActionsAndShowContextMenu(new QMenu(this), e->globalPos());
}

void MemoryView::addActionsAndShowContextMenu(QMenu* menu, const QPoint& globalPosition)
{
    auto* const reload = menu->addAction(i18nc("@action::inmenu", "&Reload"));
    reload->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    reload->setEnabled(m_appHasStarted && !m_memData.isEmpty());

    QActionGroup* formatGroup = nullptr;
    QActionGroup* groupingGroup = nullptr;
    {
        // make Format menu with action group
        auto* const formatMenu = menu->addMenu(i18nc("@title:menu", "&Format"));
        formatGroup = new QActionGroup(formatMenu);

        QAction *binary = formatGroup->addAction(i18nc("@item:inmenu display format", "&Binary"));
        binary->setData(Okteta::ByteArrayColumnView::BinaryCoding);
        binary->setShortcut(Qt::Key_B);
        formatMenu->addAction(binary);

        QAction *octal = formatGroup->addAction(i18nc("@item:inmenu display format", "&Octal"));
        octal->setData(Okteta::ByteArrayColumnView::OctalCoding);
        octal->setShortcut(Qt::Key_O);
        formatMenu->addAction(octal);

        QAction *decimal = formatGroup->addAction(i18nc("@item:inmenu display format", "&Decimal"));
        decimal->setData(Okteta::ByteArrayColumnView::DecimalCoding);
        decimal->setShortcut(Qt::Key_D);
        formatMenu->addAction(decimal);

        QAction *hex = formatGroup->addAction(i18nc("@item:inmenu display format", "&Hexadecimal"));
        hex->setData(Okteta::ByteArrayColumnView::HexadecimalCoding);
        hex->setShortcut(Qt::Key_H);
        formatMenu->addAction(hex);

        const auto formatActions = formatGroup->actions();
        for (QAction* act : formatActions) {
            act->setCheckable(true);
            act->setChecked(act->data().toInt() ==  m_memViewView->valueCoding());
            act->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        }


        // make Grouping menu with action group
        auto* const groupingMenu = menu->addMenu(i18nc("@title:menu", "&Grouping"));
        groupingGroup = new QActionGroup(groupingMenu);

        QAction *group0 = groupingGroup->addAction(i18nc("@item:inmenu no byte grouping", "&0"));
        group0->setData(0);
        group0->setShortcut(Qt::Key_0);
        groupingMenu->addAction(group0);

        QAction *group1 = groupingGroup->addAction(i18nc("@item:inmenu byte group size", "&1"));
        group1->setData(1);
        group1->setShortcut(Qt::Key_1);
        groupingMenu->addAction(group1);

        QAction *group2 = groupingGroup->addAction(i18nc("@item:inmenu byte group size", "&2"));
        group2->setData(2);
        group2->setShortcut(Qt::Key_2);
        groupingMenu->addAction(group2);

        QAction *group4 = groupingGroup->addAction(i18nc("@item:inmenu byte group size", "&4"));
        group4->setData(4);
        group4->setShortcut(Qt::Key_4);
        groupingMenu->addAction(group4);

        QAction *group8 = groupingGroup->addAction(i18nc("@item:inmenu byte group size", "&8"));
        group8->setData(8);
        group8->setShortcut(Qt::Key_8);
        groupingMenu->addAction(group8);

        QAction *group16 = groupingGroup->addAction(i18nc("@item:inmenu byte group size", "1&6"));
        group16->setData(16);
        group16->setShortcut(Qt::Key_6);
        groupingMenu->addAction(group16);

        const auto groupingActions = groupingGroup->actions();
        for (QAction* act : groupingActions) {
            act->setCheckable(true);
            act->setChecked(act->data().toInt() == m_memViewView->noOfGroupedBytes());
            act->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        }
    }

    auto* const write = menu->addAction(i18nc("@action:inmenu", "Write Changes"));
    write->setIcon(QIcon::fromTheme(QStringLiteral("document-save")));
    write->setEnabled(m_appHasStarted && m_memViewView->isModified());

    auto* const range = menu->addAction(i18nc("@action:inmenu", "Change Memory Range"));
    range->setEnabled(m_appHasStarted && !m_rangeSelector->isVisible());
    range->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));

    auto* const close = menu->addAction(i18nc("@action:inmenu", "Close View"));
    close->setIcon(QIcon::fromTheme(QStringLiteral("window-close")));

    menu->setAttribute(Qt::WA_DeleteOnClose);
    auto* const result = menu->exec(globalPosition);

    if (result == reload)
    {
        // We use m_memStart and m_memAmount stored in this,
        // not textual m_memStartStr and m_memAmountStr,
        // because program position might have changes and expressions
        // are no longer valid.
        addReadMemoryCommand(QStringLiteral("%1 x 1 1 %2").arg(m_memStart).arg(m_memData.size()));
    }

    if (result && formatGroup && formatGroup == result->actionGroup())
        m_memViewView->setValueCoding( (Okteta::ByteArrayColumnView::ValueCoding)result->data().toInt());

    if (result && groupingGroup && groupingGroup == result->actionGroup())
        m_memViewView->setNoOfGroupedBytes(result->data().toInt());

    if (result == write)
    {
        memoryEdited(0, m_memData.size());
        m_memViewView->setModified(false);
    }

    if (result == range)
    {
        m_rangeSelector->startAddressLineEdit->setText(m_memStartStr);
        m_rangeSelector->amountLineEdit->setText(m_memAmountStr);

        m_rangeSelector->show();
        m_rangeSelector->startAddressLineEdit->setFocus();
    }

    if (result == close)
        deleteLater();
}

void MemoryView::enableOrDisable()
{
    const auto enable = m_appHasStarted && !m_rangeSelector->startAddressLineEdit->text().isEmpty();
    m_rangeSelector->okButton->setEnabled(enable);
}


MemoryViewerWidget::MemoryViewerWidget(CppDebuggerPlugin* /*plugin*/, QWidget* parent)
: QWidget(parent)
{
    setWindowIcon(QIcon::fromTheme(QStringLiteral("server-database"), windowIcon()));
    setWindowTitle(i18nc("@title:window", "Memory Viewer"));

    auto * newMemoryViewerAction = new QAction(this);
    newMemoryViewerAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    newMemoryViewerAction->setText(i18nc("@action", "New Memory Viewer"));
    newMemoryViewerAction->setToolTip(i18nc("@info:tooltip", "Open a new memory viewer"));
    newMemoryViewerAction->setIcon(QIcon::fromTheme(QStringLiteral("window-new")));
    connect(newMemoryViewerAction, &QAction::triggered, this , &MemoryViewerWidget::slotAddMemoryView);
    addAction(newMemoryViewerAction);

    auto *l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);

    m_toolBox = new QToolBox(this);
    m_toolBox->setContentsMargins(0, 0, 0, 0);
    l->addWidget(m_toolBox);

    setLayout(l);

    // Start with one empty memory view.
    slotAddMemoryView();
}

void MemoryViewerWidget::slotAddMemoryView()
{
    auto* widget = new MemoryView(this);
    m_toolBox->addItem(widget, widget->windowTitle());
    m_toolBox->setCurrentIndex(m_toolBox->indexOf(widget));

    connect(widget, &MemoryView::captionChanged,
            this, &MemoryViewerWidget::slotChildCaptionChanged);
}

void MemoryViewerWidget::slotChildCaptionChanged(const QString& caption)
{
    const auto* s = static_cast<const QWidget*>(sender());
    auto* ncs = const_cast<QWidget*>(s);
    QString cap = caption;
    // Prevent interpreting '&' as accelerator specifier.
    cap.replace(QLatin1Char('&'), QLatin1String("&&"));
    m_toolBox->setItemText(m_toolBox->indexOf(ncs), cap);
}

} // end of namespace GDB
} // end of namespace KDevMI

#include "memviewdlg.moc"
#include "moc_memviewdlg.cpp"
