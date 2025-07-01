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

using namespace KDevMI;
using namespace KDevMI::MI;


SelectAddressDialog::SelectAddressDialog(QWidget* parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);
    setWindowTitle(i18nc("@title:window", "Address Selector"));

    connect(m_ui.comboBox, &KHistoryComboBox::editTextChanged,
            this, &SelectAddressDialog::validateInput);
    connect(m_ui.comboBox, QOverload<const QString&>::of(&KHistoryComboBox::returnPressed),
            this, &SelectAddressDialog::itemSelected);
}

QString SelectAddressDialog::address() const
{
    return hasValidAddress() ? m_ui.comboBox->currentText() : QString();
}

bool SelectAddressDialog::hasValidAddress() const
{
    bool ok;
    m_ui.comboBox->currentText().toLongLong(&ok, 16);

    return ok;
}

void SelectAddressDialog::updateOkState()
{
    m_ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(hasValidAddress());
}

void SelectAddressDialog::validateInput()
{
    updateOkState();
}

void SelectAddressDialog::itemSelected()
{
    QString text = m_ui.comboBox->currentText();
    if( hasValidAddress() && m_ui.comboBox->findText(text) < 0 )
        m_ui.comboBox->addItem(text);
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
    switch(flavor)
    {
    case DisassemblyFlavorUnknown:
        m_disassemblyFlavorAtt->setChecked(false);
        m_disassemblyFlavorIntel->setChecked(false);
        break;
    case DisassemblyFlavorATT:
        m_disassemblyFlavorAtt->setChecked(true);
        m_disassemblyFlavorIntel->setChecked(false);
        break;
    case DisassemblyFlavorIntel:
        m_disassemblyFlavorAtt->setChecked(false);
        m_disassemblyFlavorIntel->setChecked(true);
        break;
    }
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
        active_(false),
        lower_(0),
        upper_(0),
        address_(0),
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

/***************************************************************************/

bool DisassembleWidget::displayCurrent()
{
    if(address_ < lower_ || address_ > upper_) return false;

    bool bFound=false;
    for (int line=0; line < m_disassembleWindow->topLevelItemCount(); line++)
    {
        QTreeWidgetItem* item = m_disassembleWindow->topLevelItem(line);
        unsigned long address = item->text(Address).toULong(&ok,16);

        if (address == address_)
        {
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

    if (active_ != activate)
    {
        active_ = activate;
        if (active_)
        {
            updateDisassemblyFlavor();
            m_registersManager->updateRegisters();
            if (!displayCurrent())
                disassembleMemoryRegion();
        }
    }
}

/***************************************************************************/

void DisassembleWidget::slotShowStepInSource(const QUrl&, int,
        const QString& currentAddress)
{
    update(currentAddress);
}

void DisassembleWidget::updateExecutionAddressHandler(const ResultRecord& r)
{
    const Value& content = r[QStringLiteral("asm_insns")];
    const Value& pc = content[0];
    if( pc.hasField(QStringLiteral("address")) ){
        QString addr = pc[QStringLiteral("address")].literal();
        address_ = addr.toULong(&ok,16);

        disassembleMemoryRegion(addr);
    }
}

/***************************************************************************/

void DisassembleWidget::disassembleMemoryRegion(const QString& from, const QString& to)
{
    auto *s = qobject_cast<MIDebugSession*>(KDevelop::ICore::
            self()->debugController()->currentSession());
    if(!s || !s->isRunning()) return;

    //only get $pc
    if (from.isEmpty()){
        s->addCommand(DataDisassemble, QStringLiteral("-s \"$pc\" -e \"$pc+1\" -- 0"),
                      this, &DisassembleWidget::updateExecutionAddressHandler);
    }else{

        QString cmd = (to.isEmpty())?
        QStringLiteral("-s %1 -e \"%1 + 256\" -- 0").arg(from ):
        QStringLiteral("-s %1 -e %2+1 -- 0").arg(from, to); // if both addr set

        s->addCommand(DataDisassemble, cmd,
                      this, &DisassembleWidget::disassembleMemoryHandler);
   }
}

/***************************************************************************/

void DisassembleWidget::disassembleMemoryHandler(const ResultRecord& r)
{
    const Value& content = r[QStringLiteral("asm_insns")];
    QString currentFunction;

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
            lower_ = addr.toULong(&ok,16);
        } else  if (i == content.size()-1) {
            upper_ = addr.toULong(&ok,16);
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
    if(!m_dlg) return;
    m_dlg->updateOkState();

    if (!m_disassembleWindow->selectedItems().isEmpty()) {
        m_dlg->setAddress(m_disassembleWindow->selectedItems().first()->text(Address));
    }

    if (m_dlg->exec() == QDialog::Rejected)
        return;

    unsigned long addr = m_dlg->address().toULong(&ok,16);

    if (addr < lower_ || addr > upper_ || !displayCurrent())
        disassembleMemoryRegion(m_dlg->address());
}

void SelectAddressDialog::setAddress ( const QString& address )
{
     m_ui.comboBox->setCurrentItem ( address, true );
}

void DisassembleWidget::update(const QString &address)
{
    if (!active_) {
        return;
    }

    address_ = address.toULong(&ok, 16);
    if (!displayCurrent()) {
        disassembleMemoryRegion();
    }
    m_registersManager->updateRegisters();
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
    if (r.reason == QLatin1String("done") && active_) {
        disassembleMemoryRegion();
    }
}

void DisassembleWidget::updateDisassemblyFlavor()
{
    auto* s = qobject_cast<MIDebugSession*>(KDevelop::ICore::
            self()->debugController()->currentSession());
    if(!s || !s->isRunning()) {
        return;
    }

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

#include "moc_disassemblewidget.cpp"
