#include "editor.h"


#include "codecompletion_iface.h"
#include "KEditorDebugIface.h"


KEditor::CodeCompletionDocumentIface::CodeCompletionDocumentIface(KEditor::Document *parent, KEditor::Editor *editor)
  : KEditor::DocumentInterface(parent, editor){
}




KEditor::CodeCompletionDocumentIface *KEditor::CodeCompletionDocumentIface::interface(KEditor::Document *doc){
  return static_cast<KEditor::CodeCompletionDocumentIface*>(doc->queryInterface("KEditor::CodeCompletionDocumentIface"));
}

/*void KEditor::CodeCompletionDocumentIface::showArgHint ( const QStringList& functionList, const QString& strWrapping, const QString& strDelimiter)
{
}*/
//#include "codecompletion_iface.moc.cpp"
