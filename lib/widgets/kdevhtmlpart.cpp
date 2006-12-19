#include <QFile>
#include <qclipboard.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QTextStream>

#include <kaction.h>
#include <kstandardaction.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kmenu.h>
#include <kicon.h>
#include <kmainwindow.h>
#include <khtmlview.h>
#include <khtml_settings.h>
#include <kconfig.h>
#include <ktoolbarpopupaction.h>

#include <kdevmainwindow.h>
#include <kdevdocumentcontroller.h>


#include "kdevhtmlpart.h"

KDevHTMLPart::KDevHTMLPart()
  : KHTMLPart(static_cast<QWidget*>(0L), 0L, DefaultGUI )
{
  setXMLFile(KStandardDirs::locate("data", "kdevelop/kdevhtml_partui.rc"), true);

  connect(browserExtension(), SIGNAL(openUrlRequestDelayed(const KUrl &,const KParts::URLArgs &)),
          this, SLOT(openURLRequest(const KUrl &)) );

  connect(this, SIGNAL(started(KIO::Job *)), this, SLOT(slotStarted(KIO::Job* )));
  connect(this, SIGNAL(completed()), this, SLOT(slotCompleted()));
  connect(this, SIGNAL(canceled(const QString &)), this, SLOT(slotCancelled(const QString &)));

  KActionCollection * actions = actionCollection();// new KActionCollection( this );
  reloadAction = new KAction( KIcon("reload"), i18n( "Reload" ), actions, "doc_reload" );
  reloadAction->setWhatsThis(i18n("<b>Reload</b><p>Reloads the current document."));
  connect(reloadAction, SIGNAL(triggered(bool)), SLOT( slotReload() ));

  stopAction = new KAction( KIcon("stop"), i18n( "Stop" ), actions, "doc_stop" );
  stopAction->setWhatsThis(i18n("<b>Stop</b><p>Stops the loading of current document."));
  connect(stopAction, SIGNAL(triggered(bool)), SLOT( slotStop() ));

  duplicateAction = new KAction( KIcon("window_new"), i18n( "Duplicate Tab" ), actions, "doc_dup" );
  duplicateAction->setWhatsThis(i18n("<b>Duplicate window</b><p>Opens current document in a new window."));
  connect(duplicateAction, SIGNAL(triggered(bool)), SLOT( slotDuplicate() ));

  printAction = KStandardAction::print(this, SLOT(slotPrint()), actions, "print_doc");
  copyAction = KStandardAction::copy(this, SLOT(slotCopy()), actions, "copy_doc_selection");

  connect( this, SIGNAL(popupMenu(const QString &, const QPoint &)), this, SLOT(popup(const QString &, const QPoint &)));
  connect(this, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));

//BEGIN documentation history stuff

  m_backAction = new KToolBarPopupAction(KIcon("back"), i18n("Back"), actions, "browser_back");
  m_backAction->setEnabled( false );
  m_backAction->setToolTip(i18n("Back"));
  m_backAction->setWhatsThis(i18n("<b>Back</b><p>Moves backwards one step in the <b>documentation</b> browsing history."));
  connect(m_backAction, SIGNAL(triggered(bool)), SLOT(slotBack()));

  connect(m_backAction->menu(), SIGNAL(aboutToShow()),
         this, SLOT(slotBackAboutToShow()));
  connect(m_backAction->menu(), SIGNAL(activated(int)),
         this, SLOT(slotPopupActivated(int)));

  m_forwardAction = new KToolBarPopupAction(KIcon("forward"), i18n("Forward"), actions, "browser_forward");
  m_forwardAction->setEnabled( false );
  m_forwardAction->setToolTip(i18n("Forward"));
  m_forwardAction->setWhatsThis(i18n("<b>Forward</b><p>Moves forward one step in the <b>documentation</b> browsing history."));
  connect(m_forwardAction, SIGNAL(triggered(bool)), SLOT(slotForward()));

  connect(m_forwardAction->menu(), SIGNAL(aboutToShow()),
         this, SLOT(slotForwardAboutToShow()));
  connect(m_forwardAction->menu(), SIGNAL(triggered(QAction*)),
         this, SLOT(slotForwardMenuTriggered(QAction*)));

  m_restoring = false;
  m_Current = m_history.end();
//END documentation history stuff

  //settings:
  KConfig *appConfig = KGlobal::config();
  appConfig->setGroup("KHTMLPart");
  setStandardFont(appConfig->readEntry("StandardFont",
      settings()->stdFontName()));
  setFixedFont(appConfig->readEntry("FixedFont",
      settings()->fixedFontName()));
  setZoomFactor(appConfig->readEntry("Zoom", "100").toInt());
}

void KDevHTMLPart::popup( const QString & url, const QPoint & p )
{
//  KMenu popup( i18n( "Documentation Viewer" ), this->widget() );
  KMenu popup(this->widget());

  bool needSep = false;
  QAction* idNewWindow = 0L;
  if (!url.isEmpty() && (m_options & CanOpenInNewWindow))
  {
    idNewWindow = popup.addAction(KIcon("window_new"),i18n("Open in New Tab"));
    idNewWindow->setWhatsThis(i18n("<b>Open in new window</b><p>Opens current link in a new window."));
    needSep = true;
  }
  if (m_options & CanDuplicate)
  {
      popup.addAction(duplicateAction);
      needSep = true;
  }
  if (needSep)
      popup.addSeparator();

  popup.addAction(m_backAction);
  popup.addAction(m_forwardAction);
  popup.addAction(reloadAction);
//  popup->addAction(stopAction);
  popup.addSeparator();

  popup.addAction(copyAction);
  popup.addSeparator();

  popup.addAction(printAction);
  popup.addSeparator();

  QAction * incFontAction = this->action("incFontSizes");
  QAction * decFontAction = this->action("decFontSizes");
  if ( incFontAction && decFontAction )
  {
    popup.addAction(incFontAction);
    popup.addAction(decFontAction);
    popup.addSeparator();
  }

  QAction *ac = action("setEncoding");
  if (ac)
    popup.addAction(ac);

  QAction* r = popup.exec(p);

  if (r == idNewWindow)
  {
    KUrl kurl;
    if (!KUrl(url).path().startsWith("/"))
    {
        kDebug() << "processing relative url: " << url << endl;
        if (url.startsWith("#"))
        {
            kurl = KUrl(KDevHTMLPart::url());
            kurl.setRef(url.mid(1));
        }
        else
            kurl = KUrl(KDevHTMLPart::url().upUrl().url(KUrl::AddTrailingSlash)+url);
    }
    else
        kurl = KUrl(url);

    if (kurl.isValid())
        slotOpenInNewWindow(kurl);
  }
}

void KDevHTMLPart::setContext(const QString &context)
{
  m_context = context;
}


QString KDevHTMLPart::context() const
{
  return m_context;
}


// Note: this function is a copy of code in kdecore/kconfigbase.cpp ;)
static bool isUtf8(const char *buf) {
  int i, n;
  register unsigned char c;
  bool gotone = false;

#define F 0   /* character never appears in text */
#define T 1   /* character appears in plain ASCII text */
#define I 2   /* character appears in ISO-8859 text */
#define X 3   /* character appears in non-ISO extended ASCII (Mac, IBM PC) */

  static const unsigned char text_chars[256] = {
  /*                  BEL BS HT LF    FF CR    */
        F, F, F, F, F, F, F, T, T, T, T, F, T, T, F, F,  /* 0x0X */
        /*                              ESC          */
        F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F,  /* 0x1X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x2X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x3X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x4X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x5X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x6X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, F,  /* 0x7X */
        /*            NEL                            */
        X, X, X, X, X, T, X, X, X, X, X, X, X, X, X, X,  /* 0x8X */
        X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,  /* 0x9X */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xaX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xbX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xcX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xdX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xeX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I   /* 0xfX */
  };

  /* *ulen = 0; */
  for (i = 0; (c = buf[i]); i++) {
    if ((c & 0x80) == 0) {        /* 0xxxxxxx is plain ASCII */
      /*
       * Even if the whole file is valid UTF-8 sequences,
       * still reject it if it uses weird control characters.
       */

      if (text_chars[c] != T)
        return false;

    } else if ((c & 0x40) == 0) { /* 10xxxxxx never 1st byte */
      return false;
    } else {                           /* 11xxxxxx begins UTF-8 */
      int following;

    if ((c & 0x20) == 0) {             /* 110xxxxx */
      following = 1;
    } else if ((c & 0x10) == 0) {      /* 1110xxxx */
      following = 2;
    } else if ((c & 0x08) == 0) {      /* 11110xxx */
      following = 3;
    } else if ((c & 0x04) == 0) {      /* 111110xx */
      following = 4;
    } else if ((c & 0x02) == 0) {      /* 1111110x */
      following = 5;
    } else
      return false;

      for (n = 0; n < following; n++) {
        i++;
        if (!(c = buf[i]))
          goto done;

        if ((c & 0x80) == 0 || (c & 0x40))
          return false;
      }
      gotone = true;
    }
  }
done:
  return gotone;   /* don't claim it's UTF-8 if it's all 7-bit */
}
#undef F
#undef T
#undef I
#undef X

QString KDevHTMLPart::resolveEnvVarsInURL(const QString& url)
{
  // check for environment variables and make necessary translations
  QString path = url;
  int nDollarPos = path.indexOf( '$' );

  // Note: the while loop below is a copy of code in kdecore/kconfigbase.cpp ;)
  while( nDollarPos != -1 && nDollarPos+1 < static_cast<int>(path.length())) {
    // there is at least one $
    if( (path)[nDollarPos+1] == '(' ) {
      int nEndPos = nDollarPos+1;
      // the next character is no $
      while ( (nEndPos <= path.length()) && (path[nEndPos]!=')') )
          nEndPos++;
      nEndPos++;
      QString cmd = path.mid( nDollarPos+2, nEndPos-nDollarPos-3 );

      QString result;
      FILE *fs = popen(QFile::encodeName(cmd).data(), "r");
      if (fs)
      {
         QTextStream ts(fs, QIODevice::ReadOnly);
         result = ts.readAll().trimmed();
         pclose(fs);
      }
      path.replace( nDollarPos, nEndPos-nDollarPos, result );
    } else if( (path)[nDollarPos+1] != '$' ) {
      int nEndPos = nDollarPos+1;
      // the next character is no $
      QString aVarName;
      if (path[nEndPos]=='{')
      {
        while ( (nEndPos <= path.length()) && (path[nEndPos]!='}') )
            nEndPos++;
        nEndPos++;
        aVarName = path.mid( nDollarPos+2, nEndPos-nDollarPos-3 );
      }
      else
      {
        while ( nEndPos <= path.length() && (path[nEndPos].isNumber()
                || path[nEndPos].isLetter() || path[nEndPos]=='_' )  )
            nEndPos++;
        aVarName = path.mid( nDollarPos+1, nEndPos-nDollarPos-1 );
      }
      const char* pEnv = 0;
      if (!aVarName.isEmpty())
           pEnv = getenv( aVarName.toAscii() );
      if( pEnv ) {
        // !!! Sergey A. Sukiyazov <corwin@micom.don.ru> !!!
        // A environment variables may contain values in 8bit
        // locale cpecified encoding or in UTF8 encoding.
        if (isUtf8( pEnv ))
            path.replace( nDollarPos, nEndPos-nDollarPos, QString::fromUtf8(pEnv) );
        else
            path.replace( nDollarPos, nEndPos-nDollarPos, QString::fromLocal8Bit(pEnv) );
      } else
      path.remove( nDollarPos, nEndPos-nDollarPos );
    } else {
      // remove one of the dollar signs
      path.remove( nDollarPos, 1 );
      nDollarPos++;
    }
    nDollarPos = path.indexOf( '$', nDollarPos );
  }

  return path;
}

bool KDevHTMLPart::openURL(const KUrl &url)
{
  QString path = resolveEnvVarsInURL(url.url());
  KUrl newUrl(path);
  KUrl oldUrl = KDevHTMLPart::url();

  bool retval = KHTMLPart::openUrl(newUrl);
  if ( retval )
  {
    emit documentURLChanged( oldUrl, newUrl );
    if ( !m_restoring )
    {
        addHistoryEntry();
    }
  }

  m_backAction->setEnabled( m_Current != m_history.begin() );
  m_forwardAction->setEnabled( m_Current != lastElement() );

  return retval;
}

QLinkedList<DocumentationHistoryEntry>::Iterator KDevHTMLPart::lastElement()
{
    return --m_history.end();
}

void KDevHTMLPart::openURLRequest(const KUrl &url)
{
    openURL( url );
}

void KDevHTMLPart::slotReload( )
{
    openURL( url() );
}

void KDevHTMLPart::slotStop( )
{
    closeUrl();
}

void KDevHTMLPart::slotStarted( KIO::Job * )
{
    stopAction->setEnabled(true);
}

void KDevHTMLPart::slotCompleted( )
{
    stopAction->setEnabled(false);
}

void KDevHTMLPart::slotCancelled( const QString & /*errMsg*/ )
{
    stopAction->setEnabled(false);
}

/*void KDevHTMLPart::slotDuplicate( )
{
    PartController::getInstance()->showDocument(url(), true);
}*/

void KDevHTMLPart::slotPrint( )
{
    view()->print();
}

void KDevHTMLPart::slotBack()
{
    if ( m_Current != m_history.begin() )
    {
        --m_Current;
        m_restoring = true;
        openURL( (*m_Current).url );
        m_restoring = false;
    }
}

void KDevHTMLPart::slotForward()
{
    if (  m_Current != lastElement() )
    {
        ++m_Current;
        m_restoring = true;
        openURL( (*m_Current).url );
        m_restoring = false;
    }
}

void KDevHTMLPart::slotBackAboutToShow()
{
    QMenu *popup = m_backAction->menu();
    popup->clear();

    if ( m_Current == m_history.begin() ) return;

    QLinkedList<DocumentationHistoryEntry>::Iterator it = m_Current;
    --it;

    int i = 0;
    while( i < 10 )
    {
        if ( it == m_history.begin() )
        {
            QAction* action = popup->addAction( (*it).url.url() );
            action->setData((*it).id);
            return;
        }

        QAction* action = popup->addAction( (*it).url.url());
        action->setData((*it).id);
        ++i;
        --it;
    }
}

void KDevHTMLPart::slotForwardAboutToShow()
{
    QMenu *popup = m_forwardAction->menu();
    popup->clear();

    if ( m_Current == lastElement() ) return;

    QLinkedList<DocumentationHistoryEntry>::Iterator it = m_Current;
    ++it;

    int i = 0;
    while( i < 10 )
    {
        if ( it == lastElement() )
        {
            popup->addAction( (*it).url.url() )->setData( (*it).id );
            return;
        }

        popup->addAction( (*it).url.url() )->setData( (*it).id );
        ++i;
        ++it;
    }
}

void KDevHTMLPart::slotForwardMenuTriggered(QAction* action)
{
    int id = action->data().toInt();

    kDebug(9000) << "id: " << id << endl;

    QLinkedList<DocumentationHistoryEntry>::Iterator it = m_history.begin();
    while( it != m_history.end() )
    {
        kDebug(9000) << "(*it).id: " << (*it).id << endl;
        if ( (*it).id == id )
        {
            m_Current = it;
            m_restoring = true;
            openURL( (*m_Current).url );
            m_restoring = false;
            return;
        }
        ++it;
    }
}

void KDevHTMLPart::addHistoryEntry()
{
    QLinkedList<DocumentationHistoryEntry>::Iterator it = m_Current;

    // if We're not already the last entry, we truncate the list here before adding an entry
    if ( it != m_history.end() && it != lastElement() )
    {
        m_history.erase( ++it, m_history.end() );
    }

    DocumentationHistoryEntry newEntry( url() );

    // Only save the new entry if it is different from the last
    if ( newEntry.url != (*m_Current).url )
    {
        m_history.append( newEntry );
        m_Current = lastElement();
    }
}

void KDevHTMLPart::slotCopy( )
{
    QString text = selectedText();
    text.replace( QChar( 0xa0 ), ' ' );
    QClipboard *cb = QApplication::clipboard();
    disconnect( cb, SIGNAL( selectionChanged() ), this, SLOT( slotClearSelection() ) );
    cb->setText(text);
    connect( cb, SIGNAL( selectionChanged() ), this, SLOT( slotClearSelection() ) );
}

void KDevHTMLPart::slotSelectionChanged( )
{
    if (selectedText().isEmpty())
        copyAction->setEnabled(false);
    else
        copyAction->setEnabled(true);
}

#include "kdevhtmlpart.moc"
