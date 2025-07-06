/*
    SPDX-FileCopyrightText: 2000 John Birch <jbb@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "disassemblewidget.h"

#include "midebuggerplugin.h"
#include "debuglog.h"
#include "midebugsession.h"
#include "mi/micommand.h"
#include "registers/registersmanager.h"

#include <debugger/interfaces/idebugsession.h>
#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <util/autoorientedsplitter.h>

#include <KLocalizedString>
#include <KSharedConfig>

#include <QShowEvent>
#include <QHideEvent>
#include <QAction>
#include <QMenu>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSplitter>
#include <QFontDatabase>
#include <QValidator>

using namespace KDevMI;
using namespace KDevMI::MI;

namespace {
unsigned long addressFromString(QStringView stringAddress, bool* ok)
{
    return stringAddress.toULong(ok, 16);
}

unsigned long knownValidAddressFromString(QStringView stringAddress)
{
    Q_ASSERT(!stringAddress.isEmpty());
    bool ok;
    const auto address = addressFromString(stringAddress, &ok);
    Q_ASSERT(ok);
    return address;
}

class AddressValidator : public QValidator
{
    Q_OBJECT
public:
    explicit AddressValidator(QObject* parent = nullptr)
        : QValidator(parent)
    {
    }

    [[nodiscard]] State validate(QString& input, int& pos) const override
    {
        Q_UNUSED(pos)
        bool ok;
        addressFromString(input, &ok);
        return ok ? Acceptable : Intermediate;
    }
};

} // unnamed namespace

SelectAddressDialog::SelectAddressDialog(QWidget* parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);
    setWindowTitle(i18nc("@title:window", "Address Selector"));

    m_ui.comboBox->setValidator(new AddressValidator(m_ui.comboBox));

    connect(m_ui.comboBox, &KHistoryComboBox::editTextChanged, this, &SelectAddressDialog::updateOkState);
    updateOkState();

    connect(m_ui.buttonBox, &QDialogButtonBox::accepted, this, [this] {
        // Pressing the Enter key adds the current text to the history automatically, but clicking the
        // OK button does not. Add it manually here so that clicking OK is equivalent to pressing Enter.
        m_ui.comboBox->addToHistory(m_ui.comboBox->currentText());
    });
}

QString SelectAddressDialog::address() const
{
    return m_ui.comboBox->currentText();
}

void SelectAddressDialog::clearHistory()
{
    m_ui.comboBox->clearEditText();
    m_ui.comboBox->clearHistory();
}

bool SelectAddressDialog::hasValidAddress() const
{
    bool ok;
    addressFromString(m_ui.comboBox->currentText(), &ok);
    return ok;
}

void SelectAddressDialog::updateOkState()
{
    m_ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(hasValidAddress());
}

DisassembleWindow::DisassembleWindow(QWidget *parent, DisassembleWidget* widget)
    : QTreeWidget(parent)
{
    /*context menu commands */{
    m_selectAddrAction = new QAction(i18nc("@action", "Change &Address"), this);
    m_selectAddrAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(m_selectAddrAction, &QAction::triggered, widget, &DisassembleWidget::slotChangeAddress);

    m_jumpToLocation = new QAction(QIcon::fromTheme(QStringLiteral("debug-execute-to-cursor")), i18nc("@action", "&Jump to Cursor"), this);
    m_jumpToLocation->setWhatsThis(i18nc("@info:whatsthis", "Sets the execution pointer to the current cursor position."));
    connect(m_jumpToLocation,&QAction::triggered, widget, &DisassembleWidget::jumpToCursor);

    m_runUntilCursor = new QAction(QIcon::fromTheme(QStringLiteral("debug-run-cursor")), i18nc("@action", "&Run to Cursor"), this);
    m_runUntilCursor->setWhatsThis(i18nc("@info:whatsthis", "Continues execution until the cursor position is reached."));
    connect(m_runUntilCursor,&QAction::triggered, widget, &DisassembleWidget::runToCursor);

    m_disassemblyFlavorAtt = new QAction(i18nc("@option:check", "&AT&&T"), this);
    m_disassemblyFlavorAtt->setToolTip(i18nc("@info:tooltip", "GDB will use the AT&T disassembly flavor (e.g. mov 0xc(%ebp),%eax)."));
    m_disassemblyFlavorAtt->setData(DisassemblyFlavorATT);
    m_disassemblyFlavorAtt->setCheckable(true);

    m_disassemblyFlavorIntel = new QAction(i18nc("@option:check", "&Intel"), this);
    m_disassemblyFlavorIntel->setToolTip(i18nc("@info:tooltip", "GDB will use the Intel disassembly flavor (e.g. mov eax, DWORD PTR [ebp+0xc])."));
    m_disassemblyFlavorIntel->setData(DisassemblyFlavorIntel);
    m_disassemblyFlavorIntel->setCheckable(true);

    m_disassemblyFlavorActionGroup = new QActionGroup(this);
    m_disassemblyFlavorActionGroup->setExclusive(true);
    m_disassemblyFlavorActionGroup->addAction(m_disassemblyFlavorAtt);
    m_disassemblyFlavorActionGroup->addAction(m_disassemblyFlavorIntel);
    connect(m_disassemblyFlavorActionGroup, &QActionGroup::triggered, widget, &DisassembleWidget::setDisassemblyFlavor);
    }
}

void DisassembleWindow::setDisassemblyFlavor(DisassemblyFlavor flavor)
{
    m_disassemblyFlavorAtt->setChecked(flavor == DisassemblyFlavorATT);
    m_disassemblyFlavorIntel->setChecked(flavor == DisassemblyFlavorIntel);
}

void DisassembleWindow::contextMenuEvent(QContextMenuEvent *e)
{
        QMenu popup(this);
        popup.addAction(m_selectAddrAction);
        popup.addAction(m_jumpToLocation);
        popup.addAction(m_runUntilCursor);
        QMenu* disassemblyFlavorMenu = popup.addMenu(i18nc("@title:menu", "Disassembly Flavor"));
        disassemblyFlavorMenu->addAction(m_disassemblyFlavorAtt);
        disassemblyFlavorMenu->addAction(m_disassemblyFlavorIntel);
        popup.exec(e->globalPos());
}
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
DisassembleWidget::DisassembleWidget(MIDebuggerPlugin*, QWidget* parent)
        : QWidget(parent),
        m_splitter(new KDevelop::AutoOrientedSplitter(this))
{
        auto* topLayout = new QVBoxLayout(this);
        topLayout->setContentsMargins(0, 0, 0, 0);

        auto* controlsLayout = new QHBoxLayout;

        topLayout->addLayout(controlsLayout);


    {   // initialize disasm/registers views
        topLayout->addWidget(m_splitter);

        //topLayout->setContentsMargins(0, 0, 0, 0);

        m_disassembleWindow = new DisassembleWindow(m_splitter, this);

        m_disassembleWindow->setWhatsThis(i18nc("@info:whatsthis", "<b>Machine code display</b><p>"
                        "A machine code view into your running "
                        "executable with the current instruction "
                        "highlighted. You can step instruction by "
                        "instruction using the debuggers toolbar "
                        "buttons of \"step over\" instruction and "
                        "\"step into\" instruction."));

        m_disassembleWindow->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        m_disassembleWindow->setSelectionMode(QTreeWidget::SingleSelection);
        m_disassembleWindow->setColumnCount(ColumnCount);
        m_disassembleWindow->setUniformRowHeights(true);
        m_disassembleWindow->setRootIsDecorated(false);

        m_disassembleWindow->setHeaderLabels(QStringList{
            QString(),
            i18nc("@title:column", "Address"),
            i18nc("@title:column", "Function"),
            i18nc("@title:column", "Instruction")
        });

        m_splitter->setStretchFactor(0, 1);
        m_splitter->setContentsMargins(0, 0, 0, 0);

        m_registersManager = new RegistersManager(m_splitter);

        m_config = KSharedConfig::openConfig()->group(QStringLiteral("Disassemble/Registers View"));

        QByteArray state = m_config.readEntry<QByteArray>("splitterState", QByteArray());
        if (!state.isEmpty()) {
            m_splitter->restoreState(state);
        }

    }

    setLayout(topLayout);

    setWindowIcon( QIcon::fromTheme(QStringLiteral("system-run"), windowIcon()) );
    setWindowTitle(i18nc("@title:window", "Disassemble/Registers View"));

    KDevelop::IDebugController* pDC=KDevelop::ICore::self()->debugController();
    Q_ASSERT(pDC);

    connect(pDC,
            &KDevelop::IDebugController::currentSessionChanged,
            this, &DisassembleWidget::currentSessionChanged);

    m_dlg = new SelectAddressDialog(this);

    // show the data if debug session is active
    KDevelop::IDebugSession* pS = pDC->currentSession();

    currentSessionChanged(pS, nullptr);

    if(pS && !pS->currentAddr().isEmpty())
        slotShowStepInSource(pS->currentUrl(), pS->currentLine(), pS->currentAddr());
}

void DisassembleWidget::jumpToCursor() {
    auto *s = qobject_cast<MIDebugSession*>(KDevelop::ICore::
            self()->debugController()->currentSession());
    if (s && s->isRunning()) {
        QString address = m_disassembleWindow->selectedItems().at(0)->text(Address);
        s->jumpToMemoryAddress(address);
    }
}

void DisassembleWidget::runToCursor(){
    auto *s = qobject_cast<MIDebugSession*>(KDevelop::ICore::
            self()->debugController()->currentSession());
    if (s && s->isRunning()) {
        QString address = m_disassembleWindow->selectedItems().at(0)->text(Address);
        s->runUntil(address);
    }
}

void DisassembleWidget::currentSessionChanged(KDevelop::IDebugSession* iSession,
                                              KDevelop::IDebugSession* iPreviousSession)
{
    if (auto* const previousSession = qobject_cast<MIDebugSession*>(iPreviousSession)) {
        disconnect(previousSession, nullptr, this, nullptr);

        // The disassembly flavor in the next debug session may differ from that in the previous one.
        m_disassemblyFlavorActionsUpToDate = false;

        // Clear out all addresses of the previous debug session because
        // they are unlikely to be valid or useful in the next session.
        m_upToDate = true;
        m_regionDisassemblyFlavorUpToDate = true;
        m_currentAddress.reset();
        m_regionFirst.reset();
        m_regionLast.reset();
        m_disassembleWindow->clear();
        m_dlg->clearHistory();
    }

    auto* const session = qobject_cast<MIDebugSession*>(iSession);

    enableControls( session != nullptr ); // disable if session closed

    m_registersManager->setSession(session);

    if (session) {
        connect(session, &MIDebugSession::showStepInSource,
                this, &DisassembleWidget::slotShowStepInSource);
        connect(session,&MIDebugSession::showStepInDisassemble,this, &DisassembleWidget::update);
    }
}


/***************************************************************************/

DisassembleWidget::~DisassembleWidget()
{
   m_config.writeEntry("splitterState", m_splitter->saveState());
}

bool DisassembleWidget::isMemoryRegionDisplayed() const
{
    Q_ASSERT(m_regionFirst.isValid() == m_regionLast.isValid());
    return m_regionFirst.isValid();
}

bool DisassembleWidget::displayCurrent()
{
    if (!isMemoryRegionDisplayed() || !m_currentAddress.isValid()) {
        return false;
    }
    if (m_currentAddress < m_regionFirst || m_currentAddress > m_regionLast) {
        return false; // cannot highlight the current address because it is outside of the displayed memory region
    }

    bool bFound=false;
    for (int line=0; line < m_disassembleWindow->topLevelItemCount(); line++)
    {
        QTreeWidgetItem* item = m_disassembleWindow->topLevelItem(line);
        const auto address = knownValidAddressFromString(item->text(Address));
        if (address == m_currentAddress.integer()) {
            // put cursor at start of line and highlight the line
            m_disassembleWindow->setCurrentItem(item);
            item->setIcon(Icon, QIcon::fromTheme(QStringLiteral("go-next")));
            bFound = true;  // need to process all items to clear icons
        }
        else if(!item->icon(Icon).isNull()) item->setIcon(Icon, QIcon());
    }

    return bFound;
}

/***************************************************************************/

void DisassembleWidget::slotActivate(bool activate)
{
    qCDebug(DEBUGGERCOMMON) << "Disassemble widget active: " << activate;

    if (m_active == activate) {
        return;
    }
    m_active = activate;

    updateIfNeeded();
}

/***************************************************************************/

void DisassembleWidget::slotShowStepInSource(const QUrl&, int,
        const QString& currentAddress)
{
    update(currentAddress);
}

void DisassembleWidget::disassembleMemoryRegion(QStringView from, QStringView to, HandlerMethod handlerMethod)
{
    Q_ASSERT(!from.isEmpty());

    auto *s = qobject_cast<MIDebugSession*>(KDevelop::ICore::
            self()->debugController()->currentSession());
    if(!s || !s->isRunning()) return;

    const auto command = to.isEmpty() ? QLatin1String{"-s %1 -e \"%1 + 256\" -- 0"}.arg(from)
                                      : QLatin1String{"-s %1 -e %2+1 -- 0"}.arg(from, to);
    s->addCommand(DataDisassemble, command, this, handlerMethod);
}

/***************************************************************************/

void DisassembleWidget::disassembleMemoryHandler(const ResultRecord& r)
{
    const Value& content = r[QStringLiteral("asm_insns")];
    QString currentFunction;

    m_regionDisassemblyFlavorUpToDate = true;

    m_disassembleWindow->clear();

    for(int i = 0; i < content.size(); ++i)
    {
        const Value& line = content[i];

        QString addr, fct, offs, inst;

        if( line.hasField(QStringLiteral("address")) )   addr = line[QStringLiteral("address")].literal();
        if( line.hasField(QStringLiteral("func-name")) ) fct  = line[QStringLiteral("func-name")].literal();
        if( line.hasField(QStringLiteral("offset")) )    offs = line[QStringLiteral("offset")].literal();
        if( line.hasField(QStringLiteral("inst")) )      inst = line[QStringLiteral("inst")].literal();

        //We use offset at the same column where function is.
        if(currentFunction == fct){
            if(!fct.isEmpty()){
                fct = QLatin1Char('+') + offs;
            }
        }else { currentFunction = fct; }

        m_disassembleWindow->addTopLevelItem(new QTreeWidgetItem(m_disassembleWindow,
                                                                 QStringList{QString(), addr, fct, inst}));

        if (i == 0) {
            m_regionFirst = addr;
        } else  if (i == content.size()-1) {
            m_regionLast = addr;
        }
    }

  displayCurrent();

  m_disassembleWindow->resizeColumnToContents(Icon);       // make Icon always visible
  m_disassembleWindow->resizeColumnToContents(Address);    // make entire address always visible
}


void DisassembleWidget::showEvent(QShowEvent*)
{
    slotActivate(true);

    //it doesn't work for large names of functions
//    for (int i = 0; i < m_disassembleWindow->model()->columnCount(); ++i)
//        m_disassembleWindow->resizeColumnToContents(i);
}

void DisassembleWidget::hideEvent(QHideEvent*)
{
    slotActivate(false);
}

void DisassembleWidget::enableControls(bool enabled)
{
    m_disassembleWindow->setEnabled(enabled);
}

void DisassembleWidget::slotChangeAddress()
{
    if (!m_disassembleWindow->selectedItems().isEmpty()) {
        m_dlg->setAddress(m_disassembleWindow->selectedItems().first()->text(Address));
    }

    if (m_dlg->exec() == QDialog::Rejected)
        return;

    disassembleMemoryRegion(m_dlg->address());
}

void SelectAddressDialog::setAddress ( const QString& address )
{
     m_ui.comboBox->setCurrentItem ( address, true );
}

void DisassembleWidget::update(const QString &address)
{
    m_currentAddress = address;
    m_upToDate = false;
    updateIfNeeded();
}

void DisassembleWidget::updateIfNeeded()
{
    if (!m_active) {
        return; // updating is not useful at this time, so postpone it to optimize
    }

    if (!m_upToDate) {
        m_upToDate = true;
        if (!displayCurrent()) {
            disassembleMemoryRegion(m_currentAddress.string());
        }
        m_registersManager->updateRegisters();
    }

    if (!m_disassemblyFlavorActionsUpToDate) {
        updateDisassemblyFlavor();
    }
}

void DisassembleWidget::setDisassemblyFlavor(QAction* action)
{
    auto* s = qobject_cast<MIDebugSession*>(KDevelop::ICore::
            self()->debugController()->currentSession());
    if(!s || !s->isRunning()) {
        return;
    }

    DisassemblyFlavor disassemblyFlavor = static_cast<DisassemblyFlavor>(action->data().toInt());
    QString cmd;
    switch(disassemblyFlavor)
    {
    default:
        // unknown flavor, do not build a GDB command
        break;
    case DisassemblyFlavorATT:
        cmd = QStringLiteral("disassembly-flavor att");
        break;
    case DisassemblyFlavorIntel:
        cmd = QStringLiteral("disassembly-flavor intel");
        break;
    }
    qCDebug(DEBUGGERCOMMON) << "Disassemble widget set " << cmd;

    if (!cmd.isEmpty()) {
        s->addCommand(GdbSet, cmd, this, &DisassembleWidget::setDisassemblyFlavorHandler);
    }
}

void DisassembleWidget::setDisassemblyFlavorHandler(const ResultRecord& r)
{
    if (isMemoryRegionDisplayed() && r.reason == QLatin1String("done")) {
        m_regionDisassemblyFlavorUpToDate = false;
        disassembleMemoryRegion(m_regionFirst.string(), m_regionLast.string(),
                                &DisassembleWidget::refreshRegionDisassemblyFlavorHandler);
    }
}

void DisassembleWidget::refreshRegionDisassemblyFlavorHandler(const MI::ResultRecord& r)
{
    if (m_regionDisassemblyFlavorUpToDate) {
        qCDebug(DEBUGGERCOMMON) << "a new memory region disassembled right after a disassembly flavor change";
        // A new memory region was disassembled with the updated disassembly flavor. Therefore,
        // refreshing its flavor is not needed. Furthermore, applying our result here would wrongly
        // replace the new region range with an obsolete one. So just ignore it and do nothing.
    } else {
        disassembleMemoryHandler(r);
    }
}

void DisassembleWidget::updateDisassemblyFlavor()
{
    auto* s = qobject_cast<MIDebugSession*>(KDevelop::ICore::
            self()->debugController()->currentSession());
    if(!s || !s->isRunning()) {
        return;
    }

    // Mark as up to date now rather than in the handler function so as to execute the MI command once.
    m_disassemblyFlavorActionsUpToDate = true;
    s->addCommand(GdbShow, QStringLiteral("disassembly-flavor"), this, &DisassembleWidget::showDisassemblyFlavorHandler);
}

void DisassembleWidget::showDisassemblyFlavorHandler(const ResultRecord& r)
{
    const Value& value = r[QStringLiteral("value")];
    qCDebug(DEBUGGERCOMMON) << "Disassemble widget disassembly flavor" << value.literal();

    DisassemblyFlavor disassemblyFlavor = DisassemblyFlavorUnknown;
    if (value.literal() == QLatin1String("att")) {
        disassemblyFlavor = DisassemblyFlavorATT;
    } else if (value.literal() == QLatin1String("intel")) {
        disassemblyFlavor = DisassemblyFlavorIntel;
    } else if (value.literal() == QLatin1String("default")) {
        disassemblyFlavor = DisassemblyFlavorATT;
    }
    m_disassembleWindow->setDisassemblyFlavor(disassemblyFlavor);
}

auto DisassembleWidget::StoredAddress::operator=(const QString& address) -> StoredAddress&
{
    m_string = address;
    m_integer = knownValidAddressFromString(m_string);
    return *this;
}

void DisassembleWidget::StoredAddress::reset()
{
    m_string.clear();
    m_integer = 0;
}

bool DisassembleWidget::StoredAddress::isValid() const
{
    return !m_string.isEmpty();
}

const QString& DisassembleWidget::StoredAddress::string() const
{
    return m_string;
}
auto DisassembleWidget::StoredAddress::integer() const -> AddressInteger
{
    return m_integer;
}

bool DisassembleWidget::StoredAddress::operator==(const StoredAddress& other) const
{
    return m_integer == other.m_integer;
}
bool DisassembleWidget::StoredAddress::operator!=(const StoredAddress& other) const
{
    return !(*this == other);
}
bool DisassembleWidget::StoredAddress::operator<(const StoredAddress& other) const
{
    return m_integer < other.m_integer;
}
bool DisassembleWidget::StoredAddress::operator>(const StoredAddress& other) const
{
    return other < *this;
}

#include "disassemblewidget.moc"
#include "moc_disassemblewidget.cpp"
