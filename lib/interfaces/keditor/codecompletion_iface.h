#ifndef __CODECOMPLETION_IFACE_H__
#define __CODECOMPLETION_IFACE_H__


#include "interface.h"


namespace KEditor {
  
  class CompletionEntry {
  public:
    QString type;
    QString text;
    QString prefix;
    QString postfix;
    QString comment;
    
    bool operator==( const CompletionEntry &c ) const {
      return ( c.type == type &&
	       c.text == text &&
	       c.postfix == postfix &&
	       c.prefix == prefix &&
	       c.comment == comment);
    }
  };

  
  class CodeCompletionDocumentIface : public KEditor::DocumentInterface
    {
      Q_OBJECT
	
	public:
      
      CodeCompletionDocumentIface(Document *parent, Editor *editor);
      static CodeCompletionDocumentIface *interface(Document *doc);

      virtual void showArgHint ( QStringList functionList, const QString& strWrapping, const QString& strDelimiter ) = 0;
      virtual void showCompletionBox(QValueList<CompletionEntry> complList,int offset=0)=0;

    signals:
      void completionAborted();
      void completionDone();
      
      
    };
  
  
}


#endif
