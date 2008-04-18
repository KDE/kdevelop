
#ifndef BREAKPOINT_WIDGET_Hd41d8cd98f00b204e9800998ecf8427e
#define BREAKPOINT_WIDGET_Hd41d8cd98f00b204e9800998ecf8427e

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QSplitter>
#include <QBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QLabel>
#include <QBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QWhatsThis>

#include <klocale.h>
#include <KIcon>

#include "kdebug.h"

#include "gdbcontroller.h"
#include "breakpointcontroller.h"

namespace GDBDebugger
{
    class CppDebuggerPlugin;

    class SmallLineEdit : public QLineEdit
    {
    public:
        SmallLineEdit(QWidget *parent)
        : QLineEdit(parent)
        {}

        QSize sizeHint() const
        {
            QSize s = QLineEdit::sizeHint();
            int width = QFontMetrics(font()).width("99");
            return QSize(width, s.height());
        }
    };

    class BreakpointDetails : public QWidget
    {
        Q_OBJECT
    public:
        BreakpointDetails(QWidget *parent) : QWidget(parent)
        {
            QVBoxLayout* layout = new QVBoxLayout(this);
            layout->setContentsMargins(11, 0, 0, 11);
            
            status_ = new QLabel(this);
            status_->setText("Breakpoint is active");
            status_->hide();
            layout->addWidget(status_);

            QGridLayout* hitsLayout = new QGridLayout();
            layout->addLayout(hitsLayout);

            hitsLayout->setContentsMargins(0, 0, 0, 0);

            hits_ = new QLabel(i18n("Not hit yet"), this);
            hitsLayout->addWidget(hits_, 0, 0, 1, 3);

            QFrame* frame = new QFrame(this);
            frame->setFrameShape(QFrame::HLine);
            hitsLayout->addWidget(frame, 1, 0, 1, 3);
            
            QLabel *l2 = new QLabel(i18n("Ignore"), this);
            hitsLayout->addWidget(l2, 2, 0);
            
            ignore_ = new SmallLineEdit(this);
            hitsLayout->addWidget(ignore_, 2, 1);

            QLabel *l3 = new QLabel(i18n("next hits"), this);
            hitsLayout->addWidget(l3, 2, 2);

            
            layout->addStretch();
        }

        void setItem(NewBreakpoint *b)
        {
            if (!b)
            {
                status_->hide();
                hits_->setEnabled(false);
                ignore_->setEnabled(false);
                return;
            }

            status_->show();
            hits_->setEnabled(true);
            ignore_->setEnabled(true);

            if (b->pending())
                status_->setText("Breakpoint is <a href=pending>pending</a>");
            else if (b->dirty())
                status_->setText("Breakpoint is <a href=dirty>dirty</a>");
            else
                status_->setText("Breakpoint is active");

            if (b->hitCount())
                // FIXME: i18n
                hits_->setText(QString("Hit %1 times").arg(b->hitCount()));
            else
                hits_->setText(i18n("Not hit yet"));

            connect(status_, SIGNAL(linkActivated(const QString&)),
                    this, SLOT(showExplanation(const QString&)));
        }

    private slots:
        void showExplanation(const QString& link)
        {
            QPoint pos = status_->mapToGlobal(status_->geometry().topLeft());
            if (link == "pending")
            {
                QWhatsThis::showText(pos,
                                     "<b>Breakpoint is pending</b>"
                                     "<p>Pending breakpoints are those that are "
                                     "communucated to GDB, but which are not yet "
                                     "installed in the target, because GDB cannot "
                                     "find the function or the file the breakpoint "
                                     "refers too. Most common case is a breakpoint "
                                     "in a shared library.  GDB will insert this "
                                     "breakpoint only when the library is loaded.",
                                     status_);
            }
            else if (link == "dirty") 
            {
                QWhatsThis::showText(pos,
                                     "<b>Breakpoint is dirty</b>"
                                     "<p>The breakpoint is not yet communicated "
                                     "to GDB.",
                                     status_);
            }
        }

    private:
        QLabel* status_;
        QLabel* hits_;
        QLineEdit* ignore_;
    };
    
    class BreakpointWidget : public QWidget
    {
        Q_OBJECT
    public:
        BreakpointWidget(CppDebuggerPlugin* plugin, 
                         GDBController* controller, 
                         QWidget *parent)
        : QWidget(parent), firstShow_(true), controller_(controller)
        {
            setWindowTitle(i18n("Debugger Breakpoints"));
            setWhatsThis(i18n("<b>Breakpoint list</b><p>"
                              "Displays a list of breakpoints with "
                              "their current status. Clicking on a "
                              "breakpoint item allows you to change "
                              "the breakpoint and will take you "
                              "to the source in the editor window."));
            setWindowIcon( KIcon("process-stop") );

            QHBoxLayout *layout = new QHBoxLayout(this);
            QSplitter *s = new QSplitter(this);
            layout->addWidget(s);

            table_ = new QTableView(s);
            table_->setSelectionBehavior(QAbstractItemView::SelectRows);
            table_->setSelectionMode(QAbstractItemView::SingleSelection);
            table_->horizontalHeader()->setHighlightSections(false);
            table_->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
            details_ = new BreakpointDetails(s);

            s->setStretchFactor(0, 2);

            table_->verticalHeader()->hide();
            
            table_->setModel(controller->breakpoints());

            connect(table_->selectionModel(), 
                    SIGNAL(selectionChanged(const QItemSelection&,
                                            const QItemSelection&)),
                    this, SLOT(slotSelectionChanged(const QItemSelection &,
                                                    const QItemSelection&)));

            connect(controller->breakpoints(), 
                    SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
                    this, 
                    SLOT(slotDataChanged(const QModelIndex&, const QModelIndex&)));

            connect(controller, SIGNAL(breakpointHit(int)),
                    this, SLOT(slotBreakpointHit(int)));
            
            setupPopupMenu();
        }

        void setupPopupMenu() 
        {
            popup_ = new QMenu(this);

            QMenu* newBreakpoint = popup_->addMenu( i18nc("New breakpoint", "New") );

            QAction* action = newBreakpoint->addAction(
                i18nc("Code breakpoint", "Code"), 
                this, 
                SLOT(slotAddBlankBreakpoint()) );
            // Use this action also to provide a local shortcut
            action->setShortcut(QKeySequence(Qt::Key_B + Qt::CTRL,
                                             Qt::Key_C));
            addAction(action);

            newBreakpoint->addAction( 
                i18nc("Data breakpoint", "Data write"),
                this, SLOT(slotAddBlankWatchpoint()));
            newBreakpoint->addAction( 
                i18nc("Data read breakpoint", "Data read"), 
                this, SLOT(slotAddBlankReadWatchpoint()));

            #if 0
            m_breakpointShow = m_ctxMenu->addAction( i18n( "Show text" ) );


            m_breakpointEdit = m_ctxMenu->addAction( i18n( "Edit" ) );
            m_breakpointEdit->setShortcut(Qt::Key_Enter);

            m_breakpointDisable = m_ctxMenu->addAction( i18n( "Disable" ) );
            #endif

            QAction* breakpointDelete = popup_->addAction(
                KIcon("breakpoint_delete"), 
                i18n( "Delete" ), 
                this,
                SLOT(slotRemoveBreakpoint()));
            breakpointDelete->setShortcut(Qt::Key_Delete);
            addAction(breakpointDelete);

            #if 0
            m_ctxMenu->addSeparator();

            m_breakpointDisableAll = m_ctxMenu->addAction( i18n( "Disable all") );
            m_breakpointEnableAll = m_ctxMenu->addAction( i18n( "Enable all") );
            m_breakpointDeleteAll = m_ctxMenu->addAction( i18n( "Delete all"), this, SLOT(slotRemoveAllBreakpoints()));
            #endif

#if 0
            connect( m_ctxMenu,     SIGNAL(triggered(QAction*)),
                     this,          SLOT(slotContextMenuSelect(QAction*)) );
#endif
        }

        void contextMenuEvent(QContextMenuEvent* event)
        {
#if 0
            Breakpoint *bp = breakpoints()->breakpointForIndex(indexAt(event->pos()));

            if (!bp)
            {
                bp = breakpoints()->breakpointForIndex(currentIndex());
            }

            if (bp)
            {
                m_breakpointShow->setEnabled(bp->hasFileAndLine());

                if (bp->isEnabled( ))
                {
                    m_breakpointDisable->setText( i18n("Disable") );
                }
                else
                {
                    m_breakpointDisable->setText( i18n("Enable") );
                }
            }
            else
            {
                m_breakpointShow->setEnabled(false);
            }

            m_breakpointDisable->setEnabled(bp);
            m_breakpointDelete->setEnabled(bp);
            m_breakpointEdit->setEnabled(bp);

            bool has_bps = !breakpoints()->breakpoints().isEmpty();
            m_breakpointDisableAll->setEnabled(has_bps);
            m_breakpointEnableAll->setEnabled(has_bps);
            m_breakpointDelete->setEnabled(has_bps);

            m_ctxMenuBreakpoint = bp;
            m_ctxMenu->popup( event->globalPos() );
#endif
            popup_->popup(event->globalPos());
        }


#if 0
        void slotContextMenuSelect( QAction* action )
        {
            #if 0
            int                  col;
            Breakpoint          *bp = m_ctxMenuBreakpoint;

            if ( action == m_breakpointShow ) {
                if (FilePosBreakpoint* fbp = dynamic_cast<FilePosBreakpoint*>(bp))
                    emit gotoSourcePosition(fbp->fileName(), fbp->lineNum()-1);

            } else if ( action == m_breakpointEdit ) {
                col = currentIndex().column();
                if (col == BreakpointController::Location || col ==  BreakpointController::Condition || col == BreakpointController::IgnoreCount)
                    openPersistentEditor(model()->index(currentIndex().row(), col, QModelIndex()));

            } else if ( action == m_breakpointDisable ) {
                bp->setEnabled( !bp->isEnabled( ) );
                bp->sendToGdb();

            } else if ( action == m_breakpointDisableAll || action == m_breakpointEnableAll ) {
                foreach (Breakpoint* breakpoint, breakpoints()->breakpoints())
                {
                    breakpoint->setEnabled(action == m_breakpointEnableAll);
                    breakpoint->sendToGdb();
                }
            }
            #endif
        }
#endif

        void showEvent(QShowEvent * event)
        {
            if (firstShow_)
            {
                /* FIXME: iterate over all possible names. */
                int id_width = QFontMetrics(font()).width("MMWrite");
                QHeaderView* header = table_->horizontalHeader();
                int width = header->width();

                header->resizeSection(0, 32);
                width -= 32;
                header->resizeSection(1, 32);
                width -= 32;
                header->resizeSection(2, id_width);
                width -= id_width;                
                header->resizeSection(3, width/2);
                header->resizeSection(4, width/2);
                firstShow_ = false;
            }
        }
        
    private:
        void edit(NewBreakpoint *n)
        {
            QModelIndex index = controller_->breakpoints()
                ->indexForItem(n, NewBreakpoint::location_column);
            table_->setCurrentIndex(index);
            table_->edit(index);
        }

    private slots:

        void slotAddBlankBreakpoint()
        {
            edit(controller_->breakpoints()->breakpointsItem()
                 ->addCodeBreakpoint());
        }       

        void slotAddBlankWatchpoint()
        {
            edit(controller_->breakpoints()->breakpointsItem()
                 ->addWatchpoint());
        }

        void slotAddBlankReadWatchpoint()
        {
            edit(controller_->breakpoints()->breakpointsItem()
                 ->addReadWatchpoint());
        }

        void slotRemoveBreakpoint()
        {
            QItemSelectionModel* sel = table_->selectionModel();
            QModelIndexList selected = sel->selectedIndexes();
            if (!selected.empty())
            {
                controller_->breakpoints()->breakpointsItem()->remove(
                    selected.first());
            }
        }

        void slotSelectionChanged(const QItemSelection& selected,
                                  const QItemSelection&)
        {
            details_->setItem(
                static_cast<NewBreakpoint*>(
                    controller_->breakpoints()->itemForIndex(
                        selected.indexes().first())));
        }

        void slotBreakpointHit(int id)
        {
            /* This method will not do the right thing if we hit a breakpoint
               that is added in GDB outside kdevelop.  In this case we'll
               first try to find the breakpoint, and fail, and only then
               update the breakpoint table and notice the new one.  */
            NewBreakpoint *b = controller_->breakpoints()->breakpointsItem()
                ->breakpointById(id);
            if (b)
            {
                QModelIndex index = controller_->breakpoints()
                    ->indexForItem(b, 0);
                table_->selectionModel()->select(
                    index, 
                    QItemSelectionModel::Rows
                    | QItemSelectionModel::ClearAndSelect);
            }
        }

        void slotDataChanged(const QModelIndex& index, const QModelIndex&)
        {
            /* This method works around another problem with highliting
               the current breakpoint -- we select it before the new
               hit count is read, so it remains stale.  For that reason,
               we get to notice when breakpoint changes.  */
            details_->setItem(
                static_cast<NewBreakpoint*>(
                    controller_->breakpoints()->itemForIndex(index)));            
        }

    private:
        QTableView* table_;
        BreakpointDetails* details_;
        QMenu* popup_;
        bool firstShow_;
        GDBController* controller_;
    };

}

#endif
