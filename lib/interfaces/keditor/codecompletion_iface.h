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
    
    bool operator==( const CompletionEntry &c ) const {
      return ( c.type == type &&
	       c.text == text &&
	       c.postfix == postfix &&
	       c.prefix == prefix);
    }
  };

  
  class CodeCompletionDocumentIface : public KEditor::DocumentInterface
    {
      Q_OBJECT
	
	public:
      
      CodeCompletionDocumentIface(Document *parent, Editor *editor);
      static CodeCompletionDocumentIface *interface(Document *doc);

      virtual void showCompletionBox(QValueList<CompletionEntry>* complList)=0;

    signals:
      void completionAborted(KEditor::Document *doc);
      void completionDone(KEditor::Document *doc);
      
      
    };
  
  
}


#endif
