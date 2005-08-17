#ifndef %{APPNAME}IMPL_H
#define %{APPNAME}IMPL_H

#include <q3hbox.h>

#include <qpe/inputmethodinterface.h>
//Added by qt3to4:
#include <QPixmap>

class QPixmap;
class QCheckBox;
class %{APPNAME} : public Q3HBox {
	Q_OBJECT
public:
	%{APPNAME}( QWidget *par, Qt::WFlags f );
	~%{APPNAME}();
	void resetState();
private slots:
	void slotKey(int);
	void slotShift(bool);
	void slotAlt(bool);
	void slotCtrl(bool);
signals:
	void key(ushort,ushort,ushort,bool,bool);
private:
	int m_state;
	QCheckBox *m_alt,*m_shi,*m_ctrl;
};

class %{APPNAME}Impl : public InputMethodInterface
{
public:
    %{APPNAME}Impl();
    virtual ~%{APPNAME}Impl();

#ifndef QT_NO_COMPONENT
    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT
#endif

    virtual QWidget *inputMethod( QWidget *parent, Qt::WFlags f );
    virtual void resetState();
    virtual QPixmap *icon();
    virtual QString name();
    virtual void onKeyPress( QObject *receiver, const char *slot );

private:
    %{APPNAME} *m_pickboard;
    QPixmap *m_icn;
};

#endif
