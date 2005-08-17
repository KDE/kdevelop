
#include "plugin_%{APPNAMELC}_impl.h"
#include "plugin_%{APPNAMELC}_impl.moc"

#include <noatun/playlist.h>
#include <noatun/stdaction.h>
#include <noatun/app.h>
#include <noatun/player.h>
#include <noatun/controls.h>
#include <noatun/effects.h>

#include <qpushbutton.h>
#include <q3dragobject.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qobject.h>
#include <q3objectdict.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QShowEvent>
#include <QWheelEvent>
#include <QGridLayout>
#include <QEvent>
#include <QDropEvent>
#include <QCloseEvent>

#include <kpopupmenu.h>
#include <kstatusbar.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kurldrag.h>
//#include <kfiledialog.h>
//#include <kconfig.h>

%{APPNAME}UI::%{APPNAME}UI() : QWidget(0,"%{APPNAME}UI"), UserInterface()
{
    setAcceptDrops(true);
    static const int buttonSize=32;

    mBack=new QPushButton(this);
    mBack->setFixedSize(buttonSize,buttonSize);
    mBack->setPixmap(BarIcon("noatunback"));
    connect(mBack, SIGNAL(clicked()), napp->player(), SLOT(back()));
    QToolTip::add(mBack,i18n("Back"));

    mStop=new QPushButton(this);
    mStop->setFixedSize(buttonSize,buttonSize);
    mStop->setPixmap(BarIcon("noatunstop"));
    connect(mStop, SIGNAL(clicked()), napp->player(), SLOT(stop()));
    QToolTip::add(mStop, i18n("Stop"));

    mPlay=new QPushButton(this);
    mPlay->setToggleButton(true);
    mPlay->setFixedSize(buttonSize,buttonSize);
    mPlay->setPixmap(BarIcon("noatunplay"));
    connect(mPlay, SIGNAL(clicked()), napp->player(), SLOT(playpause()));
    QToolTip::add(mPlay, i18n("Play"));

    mForward=new QPushButton(this);
    mForward->setFixedSize(buttonSize,buttonSize);
    mForward->setPixmap(BarIcon("noatunforward"));
    connect(mForward, SIGNAL(clicked()), napp->player(), SLOT(forward()));
    QToolTip::add(mForward, i18n("Forward"));

    mPlaylist=new QPushButton(this);
    mPlaylist->setToggleButton(true);
    mPlaylist->setFixedSize(buttonSize,buttonSize);
    mPlaylist->setPixmap(BarIcon("noatunplaylist"));
    connect(mPlaylist, SIGNAL(clicked()), napp->player(), SLOT(toggleListView()));
    QToolTip::add(mPlaylist, i18n("Playlist"));

    mLoop=new QPushButton(this);
    mLoop->setFixedSize(buttonSize,buttonSize);
    mLoop->setPixmap(BarIcon("noatunloopnone"));
    connect(mLoop, SIGNAL(clicked()), napp->player(), SLOT(loop()));
    QToolTip::add(mLoop, i18n("Change loop style"));

    mPopup=new QPushButton(this);
    mPopup->setFixedSize(buttonSize,buttonSize);
    mPopup->setPixmap(BarIcon("noatun"));
    connect(mPopup, SIGNAL(clicked()), SLOT(popup()));


    mVolume=new L33tSlider(0,100,10,0, Qt::Horizontal, this);
    mVolume->setValue(napp->player()->volume());
    mSeeker=new L33tSlider(0,1000,10,0, Qt::Horizontal, this);

    mStatusBar=new KStatusBar(this);


    QGridLayout *l=new QGridLayout(this);
    l->addWidget(mBack,0,0);
    l->addWidget(mStop,0,1);
    l->addWidget(mPlay,0,2);
    l->addWidget(mForward,0,3);
    l->addWidget(mPlaylist,0,4, Qt::AlignLeft);
    l->addWidget(mLoop,0,5);
    l->addWidget(mPopup,0,6);
    l->addColSpacing(4, buttonSize+8);

    l->addMultiCellWidget(mVolume,1,1,0,6);
    l->addMultiCellWidget(mSeeker,2,2,0,6);
    l->addMultiCellWidget(mStatusBar,3,3,0,6);

    statusBar()->message(i18n("No File Loaded"));
    statusBar()->insertItem("--:--/--:--", 1, 0, true);

    connect(napp, SIGNAL(hideYourself()), this, SLOT(hide()) );
    connect(napp, SIGNAL(showYourself()), this, SLOT(show()) );

    connect(napp->player(), SIGNAL(playing()), this, SLOT(slotPlaying()));
    connect(napp->player(), SIGNAL(stopped()), this, SLOT(slotStopped()));
    connect(napp->player(), SIGNAL(paused()), this, SLOT(slotPaused()));
    napp->player()->handleButtons();

    connect(napp->player(), SIGNAL(timeout()), this, SLOT(slotTimeout()));
    connect(napp->player(), SIGNAL(loopTypeChange(int)), this, SLOT(changeLoopType(int)));

    /* This skipToWrapper is needed to pass milliseconds to Player() as everybody
     * below the GUI is based on milliseconds instead of some unprecise thingy
     * like seconds or mille */
    connect(seeker(), SIGNAL(userChanged(int)), this, SLOT(skipToWrapper(int)));
    connect(this, SIGNAL(skipTo(int)), napp->player(), SLOT(skipTo(int)));
    connect(seeker(), SIGNAL(sliderMoved(int)), SLOT(sliderMoved(int)));

    connect(mVolume, SIGNAL(sliderMoved(int)), napp->player(), SLOT(setVolume(int)));
    connect(mVolume, SIGNAL(userChanged(int)), napp->player(), SLOT(setVolume(int)));


    connect(napp->player(), SIGNAL(playlistShown()), SLOT(playlistShown()));
    connect(napp->player(), SIGNAL(playlistHidden()), SLOT(playlistHidden()));

    // Event Filter for the RMB
    for (Q3PtrListIterator<QObject> i(*children()); i.current(); ++i)
        (*i)->installEventFilter(this);

    setCaption("Noatun");
    setIcon(BarIcon("noatun"));
    show();

    // What it is now, stay.
    setFixedSize(minimumSize());
}

%{APPNAME}UI::~%{APPNAME}UI()
{
    /** save settings here  */
}

void %{APPNAME}UI::closeEvent(QCloseEvent*)
{
    unload();
}

void %{APPNAME}UI::showEvent(QShowEvent*e)
{
    QWidget::showEvent(e);
}

void %{APPNAME}UI::dragEnterEvent(QDragEnterEvent *event)
{
    // accept uri drops only
    event->accept(KURLDrag::canDecode(event));
}

void %{APPNAME}UI::dropEvent(QDropEvent *event)
{
    KURL::List uri;
    if (KURLDrag::decode(event, uri))
    {
        for (KURL::List::Iterator i = uri.begin(); i != uri.end(); ++i)
            napp->player()->openFile(*i, false);
    }
}

void %{APPNAME}UI::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mouseReleaseEvent(e);
    if (e->button()!=Qt::RightButton) return;
    NoatunStdAction::ContextMenu::showContextMenu();
}

void %{APPNAME}UI::changeStatusbar(const QString& text, const QString &text2)
{
    if (!text2.isNull())
        statusBar()->changeItem(text2, 1);

    statusBar()->message(!text.isNull() ? text : napp->player()->current().title());
}

void %{APPNAME}UI::changeCaption(const QString& text)
{
    setCaption(text);
}

void %{APPNAME}UI::popup()
{
    NoatunStdAction::ContextMenu::showContextMenu(
        mapToGlobal(mPopup->geometry().bottomLeft()) );
}

void %{APPNAME}UI::slotPlaying()
{
    changeStatusbar(napp->player()->current().title(), napp->player()->lengthString());
    mPlay->setOn(true);
    mStop->setEnabled(true);
    mPlay->setPixmap(BarIcon("noatunpause"));
}

void %{APPNAME}UI::slotStopped()
{
    if (!napp->player()->current()) return;
    changeStatusbar(i18n("No File Loaded"), napp->player()->lengthString());
    mStop->setEnabled(false);
    mPlay->setOn(false);
    seeker()->setValue(0);
    mPlay->setPixmap(BarIcon("noatunplay"));
}

void %{APPNAME}UI::slotPaused()
{
    mStop->setEnabled(true);
    mPlay->setOn(false);
    mPlay->setPixmap(BarIcon("noatunplay"));
}

void %{APPNAME}UI::slotTimeout()
{
    mVolume->setValue(napp->player()->volume());

    if (!napp->player()->current()) return;
    if (static_cast<L33tSlider*>(seeker())->currentlyPressed()) return;
    if (seeker())
    {
        seeker()->setRange ( 0, (int)napp->player()->getLength()/1000 );
        seeker()->setValue ( (int)napp->player()->getTime()/1000 );
    }
    changeStatusbar(0, napp->player()->lengthString());
}

void %{APPNAME}UI::sliderMoved(int seconds)
{
    if (napp->player()->current())
        changeStatusbar(0, napp->player()->lengthString(seconds*1000));
}

void %{APPNAME}UI::skipToWrapper(int second)
{
    emit skipTo((long)(second*1000));
}

void %{APPNAME}UI::changeLoopType(int t)
{
    static const int time=2000;
    switch (t)
    {
        case(Player::None):
            statusBar()->message(i18n("No looping"), time);
            mLoop->setPixmap(BarIcon("noatunloopnone"));
            break;
        case(Player::Song):
            statusBar()->message(i18n("Song looping"), time);
            mLoop->setPixmap(BarIcon("noatunloopsong"));
            break;
        case(Player::Playlist):
            statusBar()->message(i18n("Playlist looping"), time);
            mLoop->setPixmap(BarIcon("noatunloopplaylist"));
            break;
        case(Player::Random):
            statusBar()->message(i18n("Random play"), time);
            mLoop->setPixmap(BarIcon("noatunlooprandom"));
    }
}

bool %{APPNAME}UI::eventFilter(QObject *o, QEvent *e)
{
    if ((e->type() == QEvent::MouseButtonRelease)
            && ((static_cast<QMouseEvent*>(e))->button()==Qt::RightButton))
    {
        mouseReleaseEvent(static_cast<QMouseEvent*>(e));
        return true;
    }

    if (e->type() == QEvent::Wheel)
    {
        wheelEvent(static_cast<QWheelEvent*>(e));
        return true;
    }
    return QWidget::eventFilter(o, e);
}

void %{APPNAME}UI::playlistShown()
{
    mPlaylist->setOn(true);
}

void %{APPNAME}UI::playlistHidden()
{
    mPlaylist->setOn(false);
}

void %{APPNAME}UI::wheelEvent(QWheelEvent *e)
{
    int delta=e->delta();
    mVolume->setValue(mVolume->value()+(delta/120));
    napp->player()->setVolume(mVolume->value()+(delta/120));
}

