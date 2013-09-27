/*
 * GDB Debugger Support
 *
 * Copyright 2000 John Birch <jbb@kdevelop.org>
 * Copyright 2006 Vladimir Prus  <ghost@cs.msu.su>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2013 Vlas Puhov <vlas.puhov@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "disassemblewidget.h"
#include "gdbcommand.h"
#include "debuggerplugin.h"

#include <kdebug.h>
#include <kdeversion.h>
#include <ktextedit.h>
#include <kglobalsettings.h>

#include <QShowEvent>
#include <QHideEvent>
#include <QAction>
#include <QMenu>
#include <QBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QSplitter>
#include <QHeaderView>

#include <klocale.h>

#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <debugger/interfaces/idebugsession.h>
#include "debugsession.h"

#include "registers/registersmanager.h"

using namespace GDBMI;

namespace GDBDebugger
{

SelectAddrDialog::SelectAddrDialog(QWidget* parent)
    : KDialog(parent)
{
    QWidget *widget = new QWidget;
    m_ui.setupUi(widget);
    setMainWidget(widget);
    setCaption(i18n("Address Selector"));
    
    connect(m_ui.comboBox, SIGNAL(editTextChanged(QString)), 
            this, SLOT(validateInput()) );
    connect(m_ui.comboBox, SIGNAL(returnPressed()),
            this, SLOT(itemSelected()) );
    connect(this, SIGNAL(okClicked()), this, SLOT(itemSelected()) );
}

bool SelectAddrDialog::hasValidAddress() const
{
    bool ok;
    m_ui.comboBox->currentText().toLongLong(&ok, 16);

    return ok;
}

void SelectAddrDialog::updateOkState()
{
    enableButtonOk( hasValidAddress() );
}

void SelectAddrDialog::validateInput()
{
    updateOkState();
}

void SelectAddrDialog::itemSelected()
{
    QString text = m_ui.comboBox->currentText();
    if( hasValidAddress() && m_ui.comboBox->findText(text) < 0 )
        m_ui.comboBox->addItem(text);
}



DisassembleWindow::DisassembleWindow(QWidget *parent, DisassembleWidget* widget)
    : QTreeWidget(parent)
{
    /*context menu commands */{
    m_selectAddrAction = new QAction(i18n("Change &address"), this);
    m_selectAddrAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(m_selectAddrAction, SIGNAL(triggered()), widget, SLOT(slotChangeAddress()));

    m_jumpToLocation = new QAction(KIcon("debug-execute-to-cursor"), i18n("&Jump to Cursor"), this);
    m_jumpToLocation->setWhatsThis(i18n("Sets the execution pointer to the current cursor position."));
    connect(m_jumpToLocation,SIGNAL(triggered()), widget, SLOT(jumpToCursor()));

    m_runUntilCursor = new QAction(KIcon("debug-run-cursor"), i18n("&Run to Cursor"), this);
    m_runUntilCursor->setWhatsThis(i18n("Continues execution until the cursor position is reached."));
    connect(m_runUntilCursor,SIGNAL(triggered()), widget, SLOT(runToCursor()));
    }
}

void DisassembleWindow::contextMenuEvent(QContextMenuEvent *e)
{
        QMenu popup(this);
        popup.addAction(m_selectAddrAction);
        popup.addAction(m_jumpToLocation);
        popup.addAction(m_runUntilCursor);
        popup.exec(e->globalPos());
}
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
const KIcon DisassembleWidget::icon_=KIcon("go-next");

DisassembleWidget::DisassembleWidget(CppDebuggerPlugin* plugin, QWidget *parent)
        : QWidget(parent),
        active_(false),
        lower_(0),
        upper_(0),
        address_(0),
        m_splitter(new QSplitter(this))
{
        QVBoxLayout* topLayout = new QVBoxLayout(this);
    
        QHBoxLayout* controlsLayout = new QHBoxLayout;

        topLayout->addLayout(controlsLayout);


    {   // initialize disasm/registers views
        topLayout->addWidget(m_splitter);

        //topLayout->setMargin(0);

        m_disassembleWindow = new DisassembleWindow(m_splitter, this);

        m_disassembleWindow->setWhatsThis(i18n("<b>Machine code display</b><p>"
                        "A machine code view into your running "
                        "executable with the current instruction "
                        "highlighted. You can step instruction by "
                        "instruction using the debuggers toolbar "
                        "buttons of \"step over\" instruction and "
                        "\"step into\" instruction."));

        m_disassembleWindow->setFont(KGlobalSettings::fixedFont());
        m_disassembleWindow->setSelectionMode(QTreeWidget::SingleSelection);
        m_disassembleWindow->setColumnCount(ColumnCount);
        m_disassembleWindow->setUniformRowHeights(true);
        m_disassembleWindow->setRootIsDecorated(false);

        m_disassembleWindow->setHeaderLabels(QStringList() << "" << i18n("Address") << i18n("Function") << i18n("Instruction"));

        m_splitter->setStretchFactor(0, 1);
        m_splitter->setContentsMargins(0, 0, 0, 0);

        m_registersManager = new RegistersManager(m_splitter);

        m_config = KGlobal::config()->group("Disassemble/Registers View");

        QByteArray state = m_config.readEntry<QByteArray>("splitterState", QByteArray());
        if (!state.isEmpty()) {
            m_splitter->restoreState(state);
        }

    }
    
    setLayout(topLayout);
    
    setWindowIcon( KIcon("system-run") );
    setWindowTitle(i18n("Disassemble/Registers View"));
    
    KDevelop::IDebugController* pDC=KDevelop::ICore::self()->debugController();
    Q_ASSERT(pDC);
    
    connect(pDC, 
            SIGNAL(currentSessionChanged(KDevelop::IDebugSession*)),
            SLOT(currentSessionChanged(KDevelop::IDebugSession*)));

    connect(plugin, SIGNAL(reset()), this, SLOT(slotDeactivate()));
    
    m_dlg = new SelectAddrDialog(this);
    
    // show the data if debug session is active
    KDevelop::IDebugSession* pS = pDC->currentSession();

    currentSessionChanged(pS);
    
    if(pS && !pS->currentAddr().isEmpty())
        slotShowStepInSource(pS->currentUrl(), pS->currentLine(), pS->currentAddr());
}

void DisassembleWidget::jumpToCursor() {
    DebugSession *s = qobject_cast<DebugSession*>(KDevelop::ICore::
            self()->debugController()->currentSession());
    if (s && s->isRunning()) {
        QString address = m_disassembleWindow->selectedItems().at(0)->text(Address);
        s->jumpToMemoryAddress(address);
    }
}

void DisassembleWidget::runToCursor(){
    DebugSession *s = qobject_cast<DebugSession*>(KDevelop::ICore::
            self()->debugController()->currentSession());
    if (s && s->isRunning()) {
        QString address = m_disassembleWindow->selectedItems().at(0)->text(Address);
        s->runUntil(address);
    }
}

void DisassembleWidget::currentSessionChanged(KDevelop::IDebugSession* s)
{
    DebugSession *session = qobject_cast<DebugSession*>(s);
    
    enableControls( session != NULL ); // disable if session closed

    m_registersManager->setSession(session);

    if (session) {
        connect(session, SIGNAL(showStepInSource(KUrl,int,QString)),
                SLOT(slotShowStepInSource(KUrl,int,QString)));
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
            item->setIcon(Icon, icon_);
            bFound = true;  // need to process all items to clear icons
        }
        else if(!item->icon(Icon).isNull()) item->setIcon(Icon, QIcon());
    }

    return bFound;
}

/***************************************************************************/

void DisassembleWidget::slotActivate(bool activate)
{
    kDebug(9012) << "Disassemble widget active: " << activate;

    if (active_ != activate)
    {
        active_ = activate;
        if (active_)
        {
            m_registersManager->updateRegisters();
            if (!displayCurrent())
                disassembleMemoryRegion();
        }
    }
}

/***************************************************************************/

void DisassembleWidget::slotShowStepInSource(   const KUrl &, int,
                                                const QString &currentAddress)
{
    address_ = currentAddress.toULong(&ok,16);
    if (!active_)
        return;

    if (!displayCurrent())
        disassembleMemoryRegion();

    m_registersManager->updateRegisters();
}

void DisassembleWidget::updateExecutionAddressHandler(const GDBMI::ResultRecord& r)
{
    const GDBMI::Value& content = r["asm_insns"];
    const GDBMI::Value& pc = content[0];
    if( pc.hasField("address") ){
        QString addr = pc["address"].literal();
        address_ = addr.toULong(&ok,16);

        disassembleMemoryRegion(addr);
    }
}

/***************************************************************************/

void DisassembleWidget::disassembleMemoryRegion(const QString& from, const QString& to)
{
    DebugSession *s = qobject_cast<DebugSession*>(KDevelop::ICore::
            self()->debugController()->currentSession());
    if(!s || !s->isRunning()) return;

    //only get $pc
    if (from.isEmpty()){
        s->addCommandToFront(
                    new GDBCommand(DataDisassemble, "-s \"$pc\" -e \"$pc+1\" -- 0", this, &DisassembleWidget::updateExecutionAddressHandler ) );
    }else{

        QString cmd = (to.isEmpty())?
        QString("-s %1 -e \"%1 + 256\" -- 0").arg(from ):
        QString("-s %1 -e %2+1 -- 0").arg(from).arg(to); // if both addr set
        
        s->addCommandToFront(
        new GDBCommand(DataDisassemble, cmd, this, &DisassembleWidget::disassembleMemoryHandler ) );

   }
}

/***************************************************************************/

void DisassembleWidget::disassembleMemoryHandler(const GDBMI::ResultRecord& r)
{
    const GDBMI::Value& content = r["asm_insns"];
    QString currentFunction;

    m_disassembleWindow->clear();

    for(int i = 0; i < content.size(); ++i)
    {
        const GDBMI::Value& line = content[i];

        QString addr, fct, offs, inst;

        if( line.hasField("address") )   addr = line["address"].literal();
        if( line.hasField("func-name") ) fct  = line["func-name"].literal();
        if( line.hasField("offset") )    offs = line["offset"].literal();
        if( line.hasField("inst") )      inst = line["inst"].literal();

        //We use offset at the same column where function is.
        if(currentFunction == fct){
            if(!fct.isEmpty()){
                fct = QString("+") + offs;
            }
        }else { currentFunction = fct; }

        m_disassembleWindow->addTopLevelItem(new QTreeWidgetItem(m_disassembleWindow,
                                                                 QStringList() << QString() << addr << fct << inst));

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

void DisassembleWidget::slotDeactivate()
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

    if( m_dlg->exec() == KDialog::Rejected) return;

    unsigned long addr = m_dlg->getAddr().toULong(&ok,16);

    if (addr < lower_ || addr > upper_ || !displayCurrent())
        disassembleMemoryRegion(m_dlg->getAddr());
}

void SelectAddrDialog::setAddress ( const QString& address )
{
     m_ui.comboBox->setCurrentItem ( address, true );
}

}

#include "disassemblewidget.moc"
