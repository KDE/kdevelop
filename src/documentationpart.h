#ifndef __DOCUMENTATIONPART_H__
#define __DOCUMENTATIONPART_H__


#include <khtml_part.h>

class KAction;
class DocumentationHistoryEntry;
class KParts::ReadOnlyPart;

class DocumentationPart : public KHTMLPart
{
  Q_OBJECT

public:

  DocumentationPart();

  void setContext(const QString &context);
  QString context() const;
  virtual bool openURL(const KURL &url);
  static QString resolveEnvVarsInURL(const QString& url);

signals:
  void fileNameChanged(KParts::ReadOnlyPart *part);

private slots:

  void slotStarted(KIO::Job *);
  void slotCompleted();
  void slotCancelled(const QString &errMsg);

  void openURLRequest(const KURL &url);
  void popup( const QString & url, const QPoint & p );

  void slotReload();
  void slotStop();
  void slotDuplicate();
  void slotPrint();

	void slotBack();
	void slotForward();
	void slotBackAboutToShow();
	void slotForwardAboutToShow();
  
	void slotPopupActivated( int id );
	void addHistoryEntry();
  
  
private:

	QValueList< DocumentationHistoryEntry > m_history;
	QValueList< DocumentationHistoryEntry >::Iterator m_Current;

	KToolBarPopupAction* m_backAction;
	KToolBarPopupAction* m_forwardAction;
	
	bool m_restoring;

  QString m_context;
  KAction *stopAction;
  KAction *reloadAction;
  KAction *duplicateAction;
  KAction *printAction;
};


#endif
