/* This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>, Alexander Neundorf <neundorf@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef QEDITLISTBOX_H
#define QEDITLISTBOX_H

#include <qgroupbox.h>
#include <qlistbox.h>

class QLineEdit;
class QComboBox;
class QPushButton;

/**
 * An editable listbox
 *
 * This class provides a editable listbox ;-), this means
 * a listbox which is accompanied by a line edit to enter new
 * items into the listbox and pushbuttons to add and remove
 * items from the listbox and two buttons to move items up and down.
 *
 * \image html keditlistbox.png "KDE Edit List Box Widget"
 *
 */


class QEditListBoxPrivate;

class QEditListBox : public QGroupBox
{
   Q_OBJECT

   Q_PROPERTY( QStringList items READ items WRITE setItems )

public:
    // @since 3.1
    class CustomEditor
    {
    public:
        CustomEditor()
            : m_representationWidget( 0L ),
              m_lineEdit( 0L ) {}
        CustomEditor( QWidget *repWidget, QLineEdit *edit )
            : m_representationWidget( repWidget ),
              m_lineEdit( edit ) {}
        CustomEditor( QComboBox *combo );

        void setRepresentationWidget( QWidget *repWidget ) {
            m_representationWidget = repWidget;
        }
        void setLineEdit( QLineEdit *edit ) {
            m_lineEdit = edit;
        }

        virtual QWidget   *representationWidget() const {
            return m_representationWidget;
        }
        virtual QLineEdit *lineEdit() const {
            return m_lineEdit;
        }

    protected:
        QWidget *m_representationWidget;
        QLineEdit *m_lineEdit;
    };

   public:

      /**
       * Enumeration of the buttons, the listbox offers. Specify them in the
       * constructor in the buttons parameter.
       */
      enum Button { Add = 1, Remove = 2, UpDown = 4, All = Add|Remove|UpDown };

      /**
       * Create an editable listbox.
       *
       * If @p checkAtEntering is true, after every character you type
       * in the line edit QEditListBox will enable or disable
       * the Add-button, depending whether the current content of the
       * line edit is already in the listbox. Maybe this can become a
       * performance hit with large lists on slow machines.
       * If @p checkAtEntering is false,
       * it will be checked if you press the Add-button. It is not
       * possible to enter items twice into the listbox.
       */
      QEditListBox(QWidget *parent = 0, const char *name = 0,
		   bool checkAtEntering=false, int buttons = All );
      /**
       * Create an editable listbox.
       *
       * The same as the other constructor, additionally it takes
       * @p title, which will be the title of the frame around the listbox.
       */
      QEditListBox(const QString& title, QWidget *parent = 0,
		   const char *name = 0, bool checkAtEntering=false,
		   int buttons = All );

      /**
       * Another constructor, which allows to use a custom editing widget
       * instead of the standard QLineEdit widget. E.g. you can use a
       * KURLRequester or a QComboBox as input widget. The custom
       * editor must consist of a lineedit and optionally another widget that
       * is used as representation. A QComboBox or a KURLRequester have a
       * QLineEdit as child-widget for example, so the QComboBox is used as
       * the representation widget.
       *
       * @see KURLRequester::customEditor()
       * @since 3.1
       */
      QEditListBox( const QString& title,
                    const CustomEditor &customEditor,
                    QWidget *parent = 0, const char *name = 0,
                    bool checkAtEntering = false, int buttons = All );

      virtual ~QEditListBox();

      /**
       * Return a pointer to the embedded QListBox.
       */
      QListBox* listBox() const     { return m_listBox; }
      /**
       * Return a pointer to the embedded QLineEdit.
       */
      QLineEdit* lineEdit() const     { return m_lineEdit; }
      /**
       * Return a pointer to the Add button
       */
      QPushButton* addButton() const     { return servNewButton; }
      /**
       * Return a pointer to the Remove button
       */
      QPushButton* removeButton() const     { return servRemoveButton; }
      /**
       * Return a pointer to the Up button
       */
      QPushButton* upButton() const     { return servUpButton; }
      /**
       * Return a pointer to the Down button
       */
      QPushButton* downButton() const     { return servDownButton; }

      /**
       * See QListBox::count()
       */
      int count() const   { return int(m_listBox->count()); }
      /**
       * See QListBox::insertStringList()
       */
      void insertStringList(const QStringList& list, int index=-1);
      /**
       * See QListBox::insertStringList()
       */
      void insertStrList(const QStrList* list, int index=-1);
      /**
       * See QListBox::insertStrList()
       */
      void insertStrList(const QStrList& list, int index=-1);
      /**
       * See QListBox::insertStrList()
       */
      void insertStrList(const char ** list, int numStrings=-1, int index=-1);
      /**
       * See QListBox::insertItem()
       */
      void insertItem(const QString& text, int index=-1) {m_listBox->insertItem(text,index);}
      /**
       * Clears both the listbox and the line edit.
       */
      void clear();
      /**
       * See QListBox::text()
       */
      QString text(int index) const { return m_listBox->text(index); }
      /**
       * See QListBox::currentItem()
       */
      int currentItem() const;
      /**
       * See QListBox::currentText()
       */
      QString currentText() const  { return m_listBox->currentText(); }

      /**
       * @returns a stringlist of all items in the listbox
       */
      QStringList items() const;

      /**
       * Clears the listbox and sets the contents to @p items
       *
       * @since 3.4
       */
      void setItems(const QStringList& items);

   signals:
      void changed();

      /**
       * This signal is emitted when the user adds a new string to the list,
       * the parameter is the added string.
       * @since 3.2
       */
      void added( const QString & text );

      /**
       * This signal is emitted when the user removes a string from the list,
       * the parameter is the removed string.
       * @since 3.2
       */
      void removed( const QString & text );

   protected slots:
      //the names should be self-explaining
      void moveItemUp();
      void moveItemDown();
      void addItem();
      void removeItem();
      void enableMoveButtons(int index);
      void typedSomething(const QString& text);

   private:
      QListBox *m_listBox;
      QPushButton *servUpButton, *servDownButton;
      QPushButton *servNewButton, *servRemoveButton;
      QLineEdit *m_lineEdit;

      //this is called in both ctors, to avoid code duplication
      void init( bool checkAtEntering, int buttons,
                 QWidget *representationWidget = 0L );

   protected:
      virtual void virtual_hook( int id, void* data );
   private:
      //our lovely private d-pointer
      QEditListBoxPrivate *d;
};

#endif
