#include <kdialog.h>
#include <klocale.h>

/********************************************************************************
** Form generated from reading ui file 'runnerwindow.ui'
**
** Created: Mon Jul 21 10:27:16 2008
**      by: Qt User Interface Compiler version 4.4.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_RUNNERWINDOW_H
#define UI_RUNNERWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QSpacerItem>
#include <QtGui/QSplitter>
#include <QtGui/QToolBar>
#include <QtGui/QTreeView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RunnerWindow
{
public:
    QAction *actionStart;
    QAction *actionStop;
    QAction *actionSelectAll;
    QAction *actionUnselectAll;
    QAction *actionAbout;
    QAction *actionExit;
    QAction *actionCollapseAll;
    QAction *actionExpandAll;
    QAction *actionMinimalUpdate;
    QAction *actionColumns;
    QAction *actionResults;
    QAction *actionSettings;
    QAction *actionToolbars;
    QAction *actionReload;
    QVBoxLayout *verticalLayout_2;
    QToolBar *runnerToolBar;
    QHBoxLayout *horizontalLayout_3;
    QSplitter *splitter;
    QFrame *runnerFrame;
    QVBoxLayout *verticalLayout;
    QFrame *frame_3;
    QHBoxLayout *horizontalLayout_2;
    QProgressBar *progressRun;
    QTreeView *treeRunner;
    QFrame *frame_2;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *spacer_2;
    QLabel *labelNumTotalText;
    QLabel *labelNumTotal;
    QLabel *labelNumSelectedText;
    QLabel *labelNumSelected;
    QLabel *labelNumRunText;
    QLabel *labelNumRun;
    QHBoxLayout *_18;
    QLabel *labelNumSuccessPic;
    QLabel *labelNumSuccess;
    QHBoxLayout *_14;
    QLabel *labelNumExceptionsPic;
    QLabel *labelNumExceptions;
    QHBoxLayout *_19;
    QLabel *labelNumFatalsPic;
    QLabel *labelNumFatals;
    QHBoxLayout *_17;
    QLabel *labelNumErrorsPic;
    QLabel *labelNumErrors;
    QHBoxLayout *_15;
    QLabel *labelNumWarningsPic;
    QLabel *labelNumWarnings;
    QHBoxLayout *_16;
    QLabel *labelNumInfosPic;
    QLabel *labelNumInfos;
    QSpacerItem *spacer;
    QTreeView *treeResults;

    void setupUi(QWidget *RunnerWindow)
    {
    if (RunnerWindow->objectName().isEmpty())
        RunnerWindow->setObjectName(QString::fromUtf8("RunnerWindow"));
    RunnerWindow->resize(1138, 490);
    QIcon icon;
    icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/qxrunner_16x16.png")), QIcon::Normal, QIcon::Off);
    RunnerWindow->setWindowIcon(icon);
    actionStart = new QAction(RunnerWindow);
    actionStart->setObjectName(QString::fromUtf8("actionStart"));
    QIcon icon1;
    icon1.addPixmap(QPixmap(QString::fromUtf8(":/icons/play.png")), QIcon::Normal, QIcon::Off);
    actionStart->setIcon(icon1);
    actionStop = new QAction(RunnerWindow);
    actionStop->setObjectName(QString::fromUtf8("actionStop"));
    QIcon icon2;
    icon2.addPixmap(QPixmap(QString::fromUtf8(":/icons/stop.png")), QIcon::Normal, QIcon::Off);
    actionStop->setIcon(icon2);
    actionSelectAll = new QAction(RunnerWindow);
    actionSelectAll->setObjectName(QString::fromUtf8("actionSelectAll"));
    QIcon icon3;
    icon3.addPixmap(QPixmap(QString::fromUtf8(":/icons/checkedbox.png")), QIcon::Normal, QIcon::Off);
    actionSelectAll->setIcon(icon3);
    actionUnselectAll = new QAction(RunnerWindow);
    actionUnselectAll->setObjectName(QString::fromUtf8("actionUnselectAll"));
    QIcon icon4;
    icon4.addPixmap(QPixmap(QString::fromUtf8(":/icons/uncheckedbox.png")), QIcon::Normal, QIcon::Off);
    actionUnselectAll->setIcon(icon4);
    actionAbout = new QAction(RunnerWindow);
    actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
    actionExit = new QAction(RunnerWindow);
    actionExit->setObjectName(QString::fromUtf8("actionExit"));
    actionCollapseAll = new QAction(RunnerWindow);
    actionCollapseAll->setObjectName(QString::fromUtf8("actionCollapseAll"));
    QIcon icon5;
    icon5.addPixmap(QPixmap(QString::fromUtf8(":/icons/expanded.png")), QIcon::Normal, QIcon::Off);
    actionCollapseAll->setIcon(icon5);
    actionExpandAll = new QAction(RunnerWindow);
    actionExpandAll->setObjectName(QString::fromUtf8("actionExpandAll"));
    QIcon icon6;
    icon6.addPixmap(QPixmap(QString::fromUtf8(":/icons/collapsed.png")), QIcon::Normal, QIcon::Off);
    actionExpandAll->setIcon(icon6);
    actionMinimalUpdate = new QAction(RunnerWindow);
    actionMinimalUpdate->setObjectName(QString::fromUtf8("actionMinimalUpdate"));
    actionColumns = new QAction(RunnerWindow);
    actionColumns->setObjectName(QString::fromUtf8("actionColumns"));
    QIcon icon7;
    icon7.addPixmap(QPixmap(QString::fromUtf8(":/icons/columns.png")), QIcon::Normal, QIcon::Off);
    actionColumns->setIcon(icon7);
    actionResults = new QAction(RunnerWindow);
    actionResults->setObjectName(QString::fromUtf8("actionResults"));
    QIcon icon8;
    icon8.addPixmap(QPixmap(QString::fromUtf8(":/icons/info.png")), QIcon::Normal, QIcon::Off);
    actionResults->setIcon(icon8);
    actionSettings = new QAction(RunnerWindow);
    actionSettings->setObjectName(QString::fromUtf8("actionSettings"));
    QIcon icon9;
    icon9.addPixmap(QPixmap(QString::fromUtf8(":/icons/properties.png")), QIcon::Normal, QIcon::Off);
    actionSettings->setIcon(icon9);
    actionToolbars = new QAction(RunnerWindow);
    actionToolbars->setObjectName(QString::fromUtf8("actionToolbars"));
    actionReload = new QAction(RunnerWindow);
    actionReload->setObjectName(QString::fromUtf8("actionReload"));
    verticalLayout_2 = new QVBoxLayout(RunnerWindow);
    verticalLayout_2->setSpacing(6);
    verticalLayout_2->setMargin(6);
    verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
    runnerToolBar = new QToolBar(RunnerWindow);
    runnerToolBar->setObjectName(QString::fromUtf8("runnerToolBar"));
    runnerToolBar->setOrientation(Qt::Horizontal);

    verticalLayout_2->addWidget(runnerToolBar);

    horizontalLayout_3 = new QHBoxLayout();
    horizontalLayout_3->setSpacing(6);
    horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
    splitter = new QSplitter(RunnerWindow);
    splitter->setObjectName(QString::fromUtf8("splitter"));
    splitter->setOrientation(Qt::Horizontal);
    runnerFrame = new QFrame(splitter);
    runnerFrame->setObjectName(QString::fromUtf8("runnerFrame"));
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(runnerFrame->sizePolicy().hasHeightForWidth());
    runnerFrame->setSizePolicy(sizePolicy);
    runnerFrame->setMinimumSize(QSize(300, 0));
    runnerFrame->setBaseSize(QSize(300, 0));
    runnerFrame->setFrameShape(QFrame::StyledPanel);
    runnerFrame->setFrameShadow(QFrame::Raised);
    verticalLayout = new QVBoxLayout(runnerFrame);
    verticalLayout->setSpacing(6);
    verticalLayout->setMargin(6);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    frame_3 = new QFrame(runnerFrame);
    frame_3->setObjectName(QString::fromUtf8("frame_3"));
    frame_3->setFrameShape(QFrame::NoFrame);
    frame_3->setFrameShadow(QFrame::Plain);
    horizontalLayout_2 = new QHBoxLayout(frame_3);
    horizontalLayout_2->setSpacing(6);
    horizontalLayout_2->setMargin(6);
    horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
    progressRun = new QProgressBar(frame_3);
    progressRun->setObjectName(QString::fromUtf8("progressRun"));
    QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy1.setHorizontalStretch(1);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(progressRun->sizePolicy().hasHeightForWidth());
    progressRun->setSizePolicy(sizePolicy1);
    progressRun->setMaximumSize(QSize(16777215, 16));
    progressRun->setValue(0);
    progressRun->setOrientation(Qt::Horizontal);

    horizontalLayout_2->addWidget(progressRun);


    verticalLayout->addWidget(frame_3);

    treeRunner = new QTreeView(runnerFrame);
    treeRunner->setObjectName(QString::fromUtf8("treeRunner"));
    QSizePolicy sizePolicy2(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(treeRunner->sizePolicy().hasHeightForWidth());
    treeRunner->setSizePolicy(sizePolicy2);
    treeRunner->setAlternatingRowColors(true);
    treeRunner->setSelectionMode(QAbstractItemView::SingleSelection);
    treeRunner->setIndentation(15);
    treeRunner->setRootIsDecorated(false);
    treeRunner->setWordWrap(false);
    treeRunner->setHeaderHidden(true);

    verticalLayout->addWidget(treeRunner);

    frame_2 = new QFrame(runnerFrame);
    frame_2->setObjectName(QString::fromUtf8("frame_2"));
    frame_2->setFrameShape(QFrame::NoFrame);
    frame_2->setFrameShadow(QFrame::Plain);
    horizontalLayout = new QHBoxLayout(frame_2);
    horizontalLayout->setSpacing(6);
    horizontalLayout->setMargin(6);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    spacer_2 = new QSpacerItem(6, 12, QSizePolicy::Fixed, QSizePolicy::Minimum);

    horizontalLayout->addItem(spacer_2);

    labelNumTotalText = new QLabel(frame_2);
    labelNumTotalText->setObjectName(QString::fromUtf8("labelNumTotalText"));

    horizontalLayout->addWidget(labelNumTotalText);

    labelNumTotal = new QLabel(frame_2);
    labelNumTotal->setObjectName(QString::fromUtf8("labelNumTotal"));

    horizontalLayout->addWidget(labelNumTotal);

    labelNumSelectedText = new QLabel(frame_2);
    labelNumSelectedText->setObjectName(QString::fromUtf8("labelNumSelectedText"));

    horizontalLayout->addWidget(labelNumSelectedText);

    labelNumSelected = new QLabel(frame_2);
    labelNumSelected->setObjectName(QString::fromUtf8("labelNumSelected"));

    horizontalLayout->addWidget(labelNumSelected);

    labelNumRunText = new QLabel(frame_2);
    labelNumRunText->setObjectName(QString::fromUtf8("labelNumRunText"));

    horizontalLayout->addWidget(labelNumRunText);

    labelNumRun = new QLabel(frame_2);
    labelNumRun->setObjectName(QString::fromUtf8("labelNumRun"));

    horizontalLayout->addWidget(labelNumRun);

    _18 = new QHBoxLayout();
    _18->setSpacing(3);
    _18->setMargin(0);
    _18->setObjectName(QString::fromUtf8("_18"));
    labelNumSuccessPic = new QLabel(frame_2);
    labelNumSuccessPic->setObjectName(QString::fromUtf8("labelNumSuccessPic"));
    labelNumSuccessPic->setPixmap(QPixmap(QString::fromUtf8(":/icons/success.png")));

    _18->addWidget(labelNumSuccessPic);

    labelNumSuccess = new QLabel(frame_2);
    labelNumSuccess->setObjectName(QString::fromUtf8("labelNumSuccess"));

    _18->addWidget(labelNumSuccess);


    horizontalLayout->addLayout(_18);

    _14 = new QHBoxLayout();
    _14->setSpacing(3);
    _14->setMargin(0);
    _14->setObjectName(QString::fromUtf8("_14"));
    labelNumExceptionsPic = new QLabel(frame_2);
    labelNumExceptionsPic->setObjectName(QString::fromUtf8("labelNumExceptionsPic"));
    labelNumExceptionsPic->setPixmap(QPixmap(QString::fromUtf8(":/icons/exception.png")));

    _14->addWidget(labelNumExceptionsPic);

    labelNumExceptions = new QLabel(frame_2);
    labelNumExceptions->setObjectName(QString::fromUtf8("labelNumExceptions"));

    _14->addWidget(labelNumExceptions);


    horizontalLayout->addLayout(_14);

    _19 = new QHBoxLayout();
    _19->setSpacing(3);
    _19->setMargin(0);
    _19->setObjectName(QString::fromUtf8("_19"));
    labelNumFatalsPic = new QLabel(frame_2);
    labelNumFatalsPic->setObjectName(QString::fromUtf8("labelNumFatalsPic"));
    labelNumFatalsPic->setPixmap(QPixmap(QString::fromUtf8(":/icons/fatal.png")));

    _19->addWidget(labelNumFatalsPic);

    labelNumFatals = new QLabel(frame_2);
    labelNumFatals->setObjectName(QString::fromUtf8("labelNumFatals"));

    _19->addWidget(labelNumFatals);


    horizontalLayout->addLayout(_19);

    _17 = new QHBoxLayout();
    _17->setSpacing(3);
    _17->setMargin(0);
    _17->setObjectName(QString::fromUtf8("_17"));
    labelNumErrorsPic = new QLabel(frame_2);
    labelNumErrorsPic->setObjectName(QString::fromUtf8("labelNumErrorsPic"));
    labelNumErrorsPic->setPixmap(QPixmap(QString::fromUtf8(":/icons/error.png")));

    _17->addWidget(labelNumErrorsPic);

    labelNumErrors = new QLabel(frame_2);
    labelNumErrors->setObjectName(QString::fromUtf8("labelNumErrors"));

    _17->addWidget(labelNumErrors);


    horizontalLayout->addLayout(_17);

    _15 = new QHBoxLayout();
    _15->setSpacing(3);
    _15->setMargin(0);
    _15->setObjectName(QString::fromUtf8("_15"));
    labelNumWarningsPic = new QLabel(frame_2);
    labelNumWarningsPic->setObjectName(QString::fromUtf8("labelNumWarningsPic"));
    labelNumWarningsPic->setPixmap(QPixmap(QString::fromUtf8(":/icons/warning.png")));

    _15->addWidget(labelNumWarningsPic);

    labelNumWarnings = new QLabel(frame_2);
    labelNumWarnings->setObjectName(QString::fromUtf8("labelNumWarnings"));

    _15->addWidget(labelNumWarnings);


    horizontalLayout->addLayout(_15);

    _16 = new QHBoxLayout();
    _16->setSpacing(3);
    _16->setMargin(0);
    _16->setObjectName(QString::fromUtf8("_16"));
    labelNumInfosPic = new QLabel(frame_2);
    labelNumInfosPic->setObjectName(QString::fromUtf8("labelNumInfosPic"));
    labelNumInfosPic->setPixmap(QPixmap(QString::fromUtf8(":/icons/info.png")));

    _16->addWidget(labelNumInfosPic);

    labelNumInfos = new QLabel(frame_2);
    labelNumInfos->setObjectName(QString::fromUtf8("labelNumInfos"));

    _16->addWidget(labelNumInfos);


    horizontalLayout->addLayout(_16);

    spacer = new QSpacerItem(28, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout->addItem(spacer);


    verticalLayout->addWidget(frame_2);

    splitter->addWidget(runnerFrame);
    treeRunner->raise();
    frame_3->raise();
    frame_2->raise();
    treeResults = new QTreeView(splitter);
    treeResults->setObjectName(QString::fromUtf8("treeResults"));
    treeResults->setWindowModality(Qt::NonModal);
    treeResults->setMaximumSize(QSize(16777215, 10000));
    treeResults->setAlternatingRowColors(true);
    treeResults->setIndentation(30);
    treeResults->setRootIsDecorated(false);
    treeResults->setWordWrap(true);
    splitter->addWidget(treeResults);

    horizontalLayout_3->addWidget(splitter);


    verticalLayout_2->addLayout(horizontalLayout_3);


    runnerToolBar->addAction(actionStart);
    runnerToolBar->addAction(actionStop);
    runnerToolBar->addAction(actionCollapseAll);
    runnerToolBar->addAction(actionExpandAll);
    runnerToolBar->addAction(actionSelectAll);
    runnerToolBar->addAction(actionUnselectAll);
    runnerToolBar->addAction(actionReload);
    runnerToolBar->addAction(actionResults);

    retranslateUi(RunnerWindow);

    QMetaObject::connectSlotsByName(RunnerWindow);
    } // setupUi

    void retranslateUi(QWidget *RunnerWindow)
    {
    RunnerWindow->setWindowTitle(tr2i18n("QxRunner", 0));
    actionStart->setText(tr2i18n("&Start", 0));
    actionStop->setText(tr2i18n("S&top", 0));
    actionSelectAll->setText(tr2i18n("Select &All", 0));
    actionUnselectAll->setText(tr2i18n("&Unselect All", 0));
    actionAbout->setText(tr2i18n("&About", 0));
    actionExit->setText(tr2i18n("E&xit", 0));
    actionCollapseAll->setText(tr2i18n("&Collapse All", 0));
    actionExpandAll->setText(tr2i18n("&Expand All", 0));
    actionMinimalUpdate->setText(tr2i18n("&Minimal Update", 0));
    actionColumns->setText(tr2i18n("C&olumns...", 0));
    actionResults->setText(tr2i18n("&Results", 0));
    actionSettings->setText(tr2i18n("&Settings...", 0));
    actionToolbars->setText(tr2i18n("&Toolbars", 0));
    actionReload->setText(tr2i18n("Reload", 0));
    runnerToolBar->setWindowTitle(tr2i18n("Ru&nner Toolbar", 0));
    labelNumTotalText->setText(tr2i18n("Total:", 0));
    labelNumTotal->setText(tr2i18n("0", 0));
    labelNumSelectedText->setText(tr2i18n("Selected:", 0));
    labelNumSelected->setText(tr2i18n("0", 0));
    labelNumRunText->setText(tr2i18n("Run:", 0));
    labelNumRun->setText(tr2i18n("0", 0));

#ifndef UI_QT_NO_TOOLTIP
    labelNumSuccessPic->setToolTip(tr2i18n("Success", 0));
#endif // QT_NO_TOOLTIP

    labelNumSuccessPic->setText(QString());
    labelNumSuccess->setText(tr2i18n("0", 0));

#ifndef UI_QT_NO_TOOLTIP
    labelNumExceptionsPic->setToolTip(tr2i18n("Exceptions", 0));
#endif // QT_NO_TOOLTIP

    labelNumExceptionsPic->setText(QString());
    labelNumExceptions->setText(tr2i18n("0", 0));

#ifndef UI_QT_NO_TOOLTIP
    labelNumFatalsPic->setToolTip(tr2i18n("Fatals", 0));
#endif // QT_NO_TOOLTIP

    labelNumFatalsPic->setText(QString());
    labelNumFatals->setText(tr2i18n("0", 0));

#ifndef UI_QT_NO_TOOLTIP
    labelNumErrorsPic->setToolTip(tr2i18n("Errors", 0));
#endif // QT_NO_TOOLTIP

    labelNumErrorsPic->setText(QString());
    labelNumErrors->setText(tr2i18n("0", 0));

#ifndef UI_QT_NO_TOOLTIP
    labelNumWarningsPic->setToolTip(tr2i18n("Warnings", 0));
#endif // QT_NO_TOOLTIP

    labelNumWarningsPic->setText(QString());
    labelNumWarnings->setText(tr2i18n("0", 0));

#ifndef UI_QT_NO_TOOLTIP
    labelNumInfosPic->setToolTip(tr2i18n("Infos", 0));
#endif // QT_NO_TOOLTIP

    labelNumInfosPic->setText(QString());
    labelNumInfos->setText(tr2i18n("0", 0));
    } // retranslateUi

};

namespace Ui {
    class RunnerWindow: public Ui_RunnerWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // RUNNERWINDOW_H

