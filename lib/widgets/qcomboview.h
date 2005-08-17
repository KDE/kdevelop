//Added by qt3to4:
#include <QWheelEvent>
#include <QFocusEvent>
#include <QPaintEvent>
#include <Q3StrList>
#include <QEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QMouseEvent>
/**********************************************************************
**
**
** Definition of QComboView class
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
** Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
**********************************************************************/

#ifndef QCOMBOVIEW_H
#define QCOMBOVIEW_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

#ifndef QT_NO_COMBOBOX

/**
@file qcomboview.h
QComboView class.
*/

class Q3StrList;
class QStringList;
class QLineEdit;
class QValidator;
class Q3ListView;
class Q3ListViewItem;
class QComboViewData;

/**
QComboView - a combo with a QListView as a popup widget.
This means that you can have a treeview inside of a combo. Otherwise it works
in the same way as QComboBox and have similar API.
*/
class Q_EXPORT QComboView : public QWidget
{
    Q_OBJECT
    Q_ENUMS( Policy )
    Q_PROPERTY( bool editable READ editable WRITE setEditable )
//    Q_PROPERTY( int count READ count )
    Q_PROPERTY( QString currentText READ currentText WRITE setCurrentText DESIGNABLE false )
//    Q_PROPERTY( QListView *currentItem READ currentItem WRITE setCurrentItem )
    Q_PROPERTY( bool autoResize READ autoResize WRITE setAutoResize DESIGNABLE false )
    Q_PROPERTY( int sizeLimit READ sizeLimit WRITE setSizeLimit )
//    Q_PROPERTY( int maxCount READ maxCount WRITE setMaxCount )
    Q_PROPERTY( Policy insertionPolicy READ insertionPolicy WRITE setInsertionPolicy )
    Q_PROPERTY( bool autoCompletion READ autoCompletion WRITE setAutoCompletion )
    Q_PROPERTY( bool duplicatesEnabled READ duplicatesEnabled WRITE setDuplicatesEnabled )
    Q_OVERRIDE( bool autoMask DESIGNABLE true SCRIPTABLE true )

public:
//    QComboView( QWidget* parent=0, const char* name=0 );
    QComboView( bool rw, QWidget* parent=0, const char* name=0 );
    ~QComboView();

    int childCount() const;

    Q3ListViewItem *currentItem() const;
    virtual void setCurrentItem( Q3ListViewItem * );
    virtual void setCurrentActiveItem( Q3ListViewItem * );

    bool	autoResize()	const;
    virtual void setAutoResize( bool );
    QSize	sizeHint() const;

    void	setPalette( const QPalette & );
    void	setFont( const QFont & );
    void	setEnabled( bool );

    virtual void setSizeLimit( int );
    int		sizeLimit() const;

/*    virtual void setMaxCount( int );
    int		maxCount() const;*/

    enum Policy { NoInsertion, AtTop, AtCurrent, AtBottom,
		  AfterCurrent, BeforeCurrent };

    virtual void setInsertionPolicy( Policy policy );
    Policy	insertionPolicy() const;

    virtual void setValidator( const QValidator * );
    const QValidator * validator() const;

    virtual void setListView( Q3ListView * );
    Q3ListView *	listView() const;

    virtual void setLineEdit( QLineEdit *edit );
    QLineEdit*	lineEdit() const;

    virtual void setAutoCompletion( bool );
    bool	autoCompletion() const;

    bool	eventFilter( QObject *object, QEvent *event );

    void	setDuplicatesEnabled( bool enable );
    bool	duplicatesEnabled() const;

    bool	editable() const;
    void	setEditable( bool );

    virtual void popup();

    QString currentText() const;
    void setCurrentText( const QString& );

public slots:
    virtual void clear();
    void	clearValidator();
    void	clearEdit();
    virtual void setEditText( const QString &);

signals:
    void	activated( Q3ListViewItem * item );
    void	highlighted( Q3ListViewItem * item );
    void	activated( const QString &);
    void	highlighted( const QString &);
    void	textChanged( const QString &);
    void    focusGranted();
    void    focusLost();

private slots:
    void	internalActivate( Q3ListViewItem * );
    void	internalHighlight( Q3ListViewItem * );
    void	internalClickTimeout();
    void	returnPressed();
    void checkState(Q3ListViewItem*);

protected:
    void	paintEvent( QPaintEvent * );
    void	resizeEvent( QResizeEvent * );
    void	mousePressEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseDoubleClickEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent *e );
    void	focusInEvent( QFocusEvent *e );
    void	focusOutEvent( QFocusEvent *e );
    void	wheelEvent( QWheelEvent *e );
    void	styleChange( QStyle& );

    void	updateMask();

private:
    void	setUpListView();
    void	setUpLineEdit();
    void	popDownListView();
    void	reIndex();
    void	currentChanged();
    Q3ListViewItem *completionIndex( const QString &, Q3ListViewItem * ) const;

    QComboViewData	*d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QComboView( const QComboView & );
    QComboView &operator=( const QComboView & );
#endif
};


#endif // QT_NO_COMBOBOX

#endif // QCOMBOVIEW_H
