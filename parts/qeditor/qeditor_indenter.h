#ifndef QEDITOR_INDENTER_H
#define QEDITOR_INDENTER_H

#include <qstring.h>
#include <qvariant.h>
#include <qmap.h>
#include <private/qrichtext_p.h>

class QEditorPart;
class KDialogBase;
class QEditor;

class QEditorIndenter: public QTextIndent{
public:
    QEditorIndenter( QEditor* );
    virtual ~QEditorIndenter();

    virtual QWidget* createConfigPage( QEditorPart*, KDialogBase* =0, const char* =0 ) { return 0; }

    virtual const QMap<QString, QVariant>& values() const { return m_values; }
    virtual void updateValues( const QMap<QString, QVariant>& values ) { m_values = values; }

    QEditor* editor() const { return m_editor; }

protected:
    QEditor* m_editor;
    QMap<QString, QVariant> m_values;
};


#endif
