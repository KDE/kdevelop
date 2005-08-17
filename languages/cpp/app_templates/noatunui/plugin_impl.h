
#ifndef %{APPNAMEUC}_IMPL_H
#define %{APPNAMEUC}_IMPL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <noatun/plugin.h>
#include <noatun/app.h>

// Pref dialog pointer global now for position saving
#include <noatun/pref.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QShowEvent>
#include <QWheelEvent>
#include <QEvent>
#include <QDropEvent>
#include <QCloseEvent>

class Player;
class QSlider;
class QPushButton;
class KStatusBar;

/**
 * @short Main window class
 * @author %{AUTHOR} <%{EMAIL}>
 * @version 0.1
 * Note: You can user _every_ widget for implementing your userinterface.
 */
class %{APPNAME}UI : public QWidget, public UserInterface
{
Q_OBJECT
public:
    %{APPNAME}UI();
    virtual ~%{APPNAME}UI();
    void load(const QString& url);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void closeEvent(QCloseEvent*);
    virtual void showEvent(QShowEvent*e);
    /** RMB -> context menu  */
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual bool eventFilter(QObject*, QEvent*);
    /** volume control  */
    virtual void wheelEvent(QWheelEvent *e);

protected:
    QSlider *seeker() const { return mSeeker; }
    KStatusBar *statusBar() const { return mStatusBar; }

public slots:
    void slotPlaying();
    void slotStopped();
    void slotPaused();

    /** "show/hide playlist" button  */
    void playlistShown();
    void playlistHidden();

    void slotTimeout();
    void sliderMoved(int seconds);
    void changeLoopType(int t);
    void skipToWrapper(int second);

signals:
    void skipTo( int ); // emitted by skipToWrapper()

private slots:
    void changeStatusbar(const QString& text, const QString &text2=0);
    void changeCaption(const QString& text);
    void popup();

private:
    QPushButton *mBack, *mStop, *mPlay, *mForward, *mPlaylist, *mPopup, *mLoop;
    QSlider *mSeeker, *mVolume;
    KStatusBar *mStatusBar;
};

#endif // %{APPNAMEUC}_IMPL_H

