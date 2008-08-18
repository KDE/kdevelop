#include <kdialog.h>
#include <klocale.h>

/********************************************************************************
** Form generated from reading ui file 'runnerwindow.ui'
**
** Created: Sun Aug 17 17:19:55 2008
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
    QVBoxLayout *verticalLayout_3;
    QToolBar *runnerToolBar;
    QHBoxLayout *horizontalLayout_3;
    QSplitter *splitter;
    QFrame *runnerFrame;
    QVBoxLayout *verticalLayout;
    QFrame *frame_3;
    QHBoxLayout *horizontalLayout_4;
    QProgressBar *progressRun;
    QTreeView *treeRunner;
    QFrame *frame_4;
    QHBoxLayout *horizontalLayout_5;
    QSpacerItem *spacer_2;
    QLabel *labelNumRunText;
    QLabel *labelNumRun;
    QLabel *labelElapsedText;
    QLabel *labelElapsed;
    QLabel *label;
    QSpacerItem *spacer;

    void setupUi(QWidget *RunnerWindow)
    {
    if (RunnerWindow->objectName().isEmpty())
        RunnerWindow->setObjectName(QString::fromUtf8("RunnerWindow"));
    RunnerWindow->resize(789, 540);
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
    verticalLayout_3 = new QVBoxLayout(RunnerWindow);
    verticalLayout_3->setSpacing(6);
    verticalLayout_3->setMargin(6);
    verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
    runnerToolBar = new QToolBar(RunnerWindow);
    runnerToolBar->setObjectName(QString::fromUtf8("runnerToolBar"));
    runnerToolBar->setOrientation(Qt::Horizontal);

    verticalLayout_3->addWidget(runnerToolBar);

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
    runnerFrame->setMinimumSize(QSize(250, 0));
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
    horizontalLayout_4 = new QHBoxLayout(frame_3);
    horizontalLayout_4->setSpacing(6);
    horizontalLayout_4->setMargin(6);
    horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
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

    horizontalLayout_4->addWidget(progressRun);


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

    frame_4 = new QFrame(runnerFrame);
    frame_4->setObjectName(QString::fromUtf8("frame_4"));
    frame_4->setFrameShape(QFrame::NoFrame);
    frame_4->setFrameShadow(QFrame::Plain);
    horizontalLayout_5 = new QHBoxLayout(frame_4);
    horizontalLayout_5->setSpacing(6);
    horizontalLayout_5->setMargin(6);
    horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
    spacer_2 = new QSpacerItem(198, 17, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout_5->addItem(spacer_2);

    labelNumRunText = new QLabel(frame_4);
    labelNumRunText->setObjectName(QString::fromUtf8("labelNumRunText"));
    labelNumRunText->setTextFormat(Qt::LogText);

    horizontalLayout_5->addWidget(labelNumRunText);

    labelNumRun = new QLabel(frame_4);
    labelNumRun->setObjectName(QString::fromUtf8("labelNumRun"));
    labelNumRun->setTextFormat(Qt::LogText);

    horizontalLayout_5->addWidget(labelNumRun);

    labelElapsedText = new QLabel(frame_4);
    labelElapsedText->setObjectName(QString::fromUtf8("labelElapsedText"));
    labelElapsedText->setTextFormat(Qt::LogText);

    horizontalLayout_5->addWidget(labelElapsedText);

    labelElapsed = new QLabel(frame_4);
    labelElapsed->setObjectName(QString::fromUtf8("labelElapsed"));
    labelElapsed->setTextFormat(Qt::LogText);

    horizontalLayout_5->addWidget(labelElapsed);

    label = new QLabel(frame_4);
    label->setObjectName(QString::fromUtf8("label"));
    label->setTextFormat(Qt::LogText);

    horizontalLayout_5->addWidget(label);

    spacer = new QSpacerItem(198, 17, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout_5->addItem(spacer);


    verticalLayout->addWidget(frame_4);

    splitter->addWidget(runnerFrame);

    horizontalLayout_3->addWidget(splitter);


    verticalLayout_3->addLayout(horizontalLayout_3);


    runnerToolBar->addAction(actionStart);
    runnerToolBar->addAction(actionStop);
    runnerToolBar->addAction(actionCollapseAll);
    runnerToolBar->addAction(actionExpandAll);
    runnerToolBar->addAction(actionSelectAll);
    runnerToolBar->addAction(actionUnselectAll);
    runnerToolBar->addAction(actionReload);

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
    labelNumRunText->setText(tr2i18n("Ran", 0));
    labelNumRun->setText(tr2i18n("0", 0));
    labelElapsedText->setText(tr2i18n("tests in", 0));
    labelElapsed->setText(tr2i18n("0.000", 0));
    label->setText(tr2i18n("seconds", 0));
    } // retranslateUi

};

namespace Ui {
    class RunnerWindow: public Ui_RunnerWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // RUNNERWINDOW_H

