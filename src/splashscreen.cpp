#include <qlayout.h>
#include <qpixmap.h>
#include <qapplication.h>


#include <kdebug.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kapplication.h>


#include "splashscreen.h"
#include "splashscreen.moc"

SplashScreen::SplashScreen()
  : QObject()
{
  m_splash = new QWidget(0, "splash", WStyle_NoBorderEx | WStyle_Customize | WStyle_Tool);

  QVBoxLayout *vbox = new QVBoxLayout(m_splash);

  QLabel *pixmap = new QLabel(m_splash);
  QPixmap pm;
  pm.load(locate("appdata", "pics/gideon-splash.png"));
  pixmap->setPixmap(pm);
  vbox->addWidget(pixmap);

  m_message = new QLabel(m_splash);
  QPalette pal(m_message->palette());
  pal.setColor(QPalette::Active, QColorGroup::Foreground, Qt::white);
  m_message->setPalette(pal);
  m_message->setBackgroundColor(Qt::black);
  vbox->addWidget(m_message);

  showMessage(i18n("Starting core application"));

  // m_splash->resize(m_splash->sizeHint());

  QRect rect = QApplication::desktop()->screenGeometry(
    QApplication::desktop()->screenNumber(QPoint(0,0)));
  m_splash->move(rect.x() + (rect.width() - m_splash->sizeHint().width()) / 2,
     rect.y() + (rect.height() - m_splash->sizeHint().height()) / 2);
  m_splash->setFixedSize(m_splash->sizeHint());

//  m_splash->setGeometry(QApplication::desktop()->width()/2-m_splash->width()/2,
//              QApplication::desktop()->height()/2-m_splash->height()/2,
//              m_splash->width(), m_splash->height());

  m_splash->show();
}


SplashScreen::~SplashScreen()
{
  delete m_splash;
}


void SplashScreen::showMessage(const QString &message)
{
  kdDebug(9000) << "SPLASH: message=" << message << endl;

  m_message->setText(message);
  kapp->processEvents();
}
