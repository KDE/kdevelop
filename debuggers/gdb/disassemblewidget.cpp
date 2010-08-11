/*
 * GDB Debugger Support
 *
 * Copyright 2000 John Birch <jbb@kdevelop.org>
 * Copyright 2006 Vladimir Prus  <ghost@cs.msu.su>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#include <stdlib.h>
#include <klocale.h>

#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <debugger/interfaces/idebugsession.h>
#include "debugsession.h"

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
    
    connect(m_ui.comboBox, SIGNAL(editTextChanged(const QString&)), 
            this, SLOT(validateInput()) );
    connect(m_ui.comboBox, SIGNAL(returnPressed()),
            this, SLOT(itemSelected()) );
    connect(this, SIGNAL(okClicked()), this, SLOT(itemSelected()) );
}

bool SelectAddrDialog::hasValidAddress() const
{
    bool ok;
    m_ui.comboBox->currentText().toInt(&ok, 16);

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


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
const KIcon DisassembleWidget::icon_=KIcon("go-next");

DisassembleWidget::DisassembleWidget(CppDebuggerPlugin* plugin, QWidget *parent)
        : QTreeWidget(parent),
        active_(false),
        lower_(0),
        upper_(0),
        address_(0)
{
    setToolTip(i18n("<b>Machine code display</b><p>"
                    "A machine code view into your running "
                    "executable with the current instruction "
                    "highlighted. You can step instruction by "
                    "instruction using the debuggers toolbar "
                    "buttons of \"step over\" instruction and "
                    "\"step into\" instruction."));
    setWindowIcon( KIcon("system-run") );
    setWindowTitle(i18n("Disassemble View"));
    setFont(KGlobalSettings::fixedFont());
    setSelectionMode(SingleSelection);
    setColumnCount(ColumnCount);
    setUniformRowHeights(true);
    setRootIsDecorated(false);

    setHeaderLabels(QStringList() << "" << i18n("Address") << i18n("Function") << i18n("Offset") << i18n("Instruction"));

    KDevelop::IDebugController* pDC=KDevelop::ICore::self()->debugController();
    Q_ASSERT(pDC);
    
    connect(pDC, 
            SIGNAL(currentSessionChanged(KDevelop::IDebugSession*)),
            SLOT(currentSessionChanged(KDevelop::IDebugSession*)));

    connect(plugin, SIGNAL(reset()), this, SLOT(clear()));
    connect(plugin, SIGNAL(reset()), this, SLOT(slotDeactivate()));
    
    // context menu command
    selectAddrAction_ = new QAction(i18n("Change &address"), this);
    selectAddrAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(selectAddrAction_, SIGNAL(triggered()), this, SLOT(slotChangeAddress()));

    dlg=new SelectAddrDialog(this);
    
    // show the data if debug session is active
    KDevelop::IDebugSession* pS=pDC->currentSession();
    if(pS && !pS->currentAddr().isEmpty())
    {
        currentSessionChanged(pS);
        slotShowStepInSource(pS->currentFile(), pS->currentLine(), pS->currentAddr());
    }
}


void DisassembleWidget::currentSessionChanged(KDevelop::IDebugSession* s)
{
    DebugSession *session = qobject_cast<DebugSession*>(s);
    if (!session) return;
    connect(session, SIGNAL(gdbShowStepInSource(QString,int,QString)),
                SLOT(slotShowStepInSource(QString,int,QString)));
}


/***************************************************************************/

DisassembleWidget::~DisassembleWidget()
{}

/***************************************************************************/

bool DisassembleWidget::displayCurrent()
{
    if(address_ < lower_ || address_ > upper_) return false;

    bool bFound=false;
    for (int line=0; line < topLevelItemCount(); line++)
    {
        QTreeWidgetItem* item = topLevelItem(line);
        unsigned long address = strtoul(item->text(Address).toLatin1(), 0, 0);

        if (address == address_)
        {
            // put cursor at start of line and highlight the line
            setCurrentItem(item);
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
        if (active_ && address_)
        {
            if (address_ < lower_ || address_ > upper_ || !displayCurrent())
                getAsmToDisplay();
        }
    }
}

/***************************************************************************/

void DisassembleWidget::slotShowStepInSource(   const QString &, int,
                                                const QString &currentAddress)
{
    kDebug();

    currentAddress_ = currentAddress;
    address_ = strtoul(currentAddress.toLatin1(), 0, 0);
    if (!active_)
        return;

    if (address_ < lower_ || address_ > upper_ || !displayCurrent())
        getAsmToDisplay();
}

/***************************************************************************/

void DisassembleWidget::getAsmToDisplay(const QString& addr)
{
    kDebug();

    if (address_)
    {
        Q_ASSERT(!currentAddress_.isNull());

        QString cmd = QString("-s %1 -e \"%1 + 128\" -- 0").
            arg( addr.isEmpty() ? "$pc" : addr );
        DebugSession *s = qobject_cast<DebugSession*>(KDevelop::ICore::self()->debugController()->currentSession());
        if (s) {
            s->addCommandToFront(
                        new GDBCommand(DataDisassemble, cmd, this, &DisassembleWidget::memoryRead ) );
        }
    }
}

/***************************************************************************/

void DisassembleWidget::memoryRead(const GDBMI::ResultRecord& r)
{
  const GDBMI::Value& content = r["asm_insns"];
  QString rawdata;

  clear();

  for(int i = 0; i < content.size(); ++i)
  {
    const GDBMI::Value& line = content[i];

    QString addr, fct, offs, inst;
    
    if( line.hasField("address") )   addr = line["address"].literal();
    if( line.hasField("func-name") ) fct  = line["func-name"].literal();
    if( line.hasField("offset") )    offs = line["offset"].literal();
    if( line.hasField("inst") )      inst = line["inst"].literal();

    addTopLevelItem(new QTreeWidgetItem(this, QStringList() << QString() << addr << fct << offs << inst));

    if (i == 0) {
      lower_ = strtoul(addr.toLatin1(), 0, 0);
    } else  if (i == content.size()-1) {
      upper_ = strtoul(addr.toLatin1(), 0, 0);
    }
  }

  displayCurrent();

  resizeColumnToContents(Icon);       // make Icon always visible
  resizeColumnToContents(Address);    // make entire address always visible
}


void DisassembleWidget::showEvent(QShowEvent*)
{
    slotActivate(true);

    for (int i = 0; i < model()->columnCount(); ++i)
        resizeColumnToContents(i);
}

void DisassembleWidget::hideEvent(QHideEvent*)
{
    slotActivate(false);
}

void DisassembleWidget::slotDeactivate()
{
    slotActivate(false);
}

void DisassembleWidget::slotChangeAddress()
{
    if(!dlg) return;
    dlg->updateOkState();
    
    if( dlg->exec() == KDialog::Rejected) return;

    unsigned long addr = strtoul(dlg->getAddr().toLatin1(), 0, 0);

    if (addr < lower_ || addr > upper_ || !displayCurrent())
        getAsmToDisplay(dlg->getAddr());
}

void DisassembleWidget::contextMenuEvent(QContextMenuEvent* e)
{
    QMenu popup(this);
    popup.addAction(selectAddrAction_);
    
    popup.exec(e->globalPos());
}
    
/***************************************************************************/

}

#include "disassemblewidget.moc"
