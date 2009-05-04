/*
    KSysGuard, the KDE System Guard

        Copyright (C) 2007 Trent Waddington <trent.waddington@gmail.com>
	Copyright (c) 2008 John Tapsell <tapsell@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.


*/

#include <klocale.h>
#include "KTextEditVT.h"

#include "KTextEditVT.moc"
#include <kglobalsettings.h>

KTextEditVT::KTextEditVT(QWidget* parent)
	: QTextEdit( parent )
{
	mParseAnsi = true;
	escape_sequence = false;
	escape_CSI = false;
	escape_OSC = false;
	escape_number1 = -1;
	escape_number_seperator = false;
	escape_number2 = -1;
	escape_code = 0;
	setFont( KGlobalSettings::fixedFont() );
}


void KTextEditVT::insertVTChar(const QChar & c) {
	if(escape_sequence) {
		if(escape_CSI || escape_OSC) {
			if(c.isDigit()) {
				if(!escape_number_seperator) {
					if(escape_number1 == -1)
						escape_number1 = c.digitValue();
					else 
						escape_number1 = escape_number1*10 + c.digitValue();
				} else {
					if(escape_number2 == -1)
						escape_number2 = c.digitValue();
					else 
						escape_number2 = escape_number2*10 + c.digitValue();

				}
			} else if(c == ';') {
				escape_number_seperator = true;
			} else if(escape_OSC && c==7) { //Throw away any letters that are not OSC
				escape_code = c; 
			} else if(escape_CSI)
				escape_code = c;
		} else if(c=='[') {
			escape_CSI = true;
		} else if(c==']') {
			escape_OSC = true;
		}
		else if(c=='(' || c==')') {}
		else
			escape_code = c;
		if(!escape_code.isNull()) {
			//We've read in the whole escape sequence.  Now parse it
			if(escape_code == 'm') { // change color
				switch(escape_number2){
					case 0: //all off
						setFontWeight(QFont::Normal);
						setTextColor(Qt::black);
						break;
					case 1: //bold
						setFontWeight(QFont::Bold);
						break;
					case 31: //red
						setTextColor(Qt::red);
						break;
					case 32: //green
						setTextColor(Qt::green);
						break;
					case 33: //yellow
						setTextColor(Qt::yellow);
						break;
					case 34: //blue
						setTextColor(Qt::blue);
						break;
					case 35: //magenta
						setTextColor(Qt::magenta);
						break;
					case 36: //cyan
						setTextColor(Qt::cyan);
						break;
					case -1:
					case 30: //black
					case 39: //reset
					case 37: //white
						setTextColor(Qt::black);
						break;
				}
			}
			escape_code = 0;
			escape_number1 = -1;
			escape_number2 = -1;
			escape_CSI = false;
			escape_OSC = false;
			escape_sequence = false;
			escape_number_seperator = false;
		}
	} else if(c == 0x0d) {
		insertPlainText(QChar('\n'));
	} else if(c.isPrint() || c == '\n') { 
		insertPlainText(QChar(c));
	} else if(mParseAnsi) {
		if(c == 127 || c == 8) { // delete or backspace, respectively
			textCursor().deletePreviousChar();
		} else if(c==27) { // escape key
			escape_sequence = true;
		} else if(c==0x9b) { // CSI - equivalent to esc [
			escape_sequence = true;
			escape_CSI = true;
		} else if(c==0x9d) { // OSC - equivalent to esc ]
			escape_sequence = true;
			escape_OSC = true;
		}

	} else if(!c.isNull()) {
		insertPlainText("[");
		QByteArray num;
		num.setNum(c.toAscii());
		insertPlainText(num);
		insertPlainText("]");
	}
}

void KTextEditVT::insertVTText(const QByteArray & string)
{
	int size= string.size();
	for(int i =0; i < size; i++)
		insertVTChar(QChar(string.at(i)));
}

void KTextEditVT::insertVTText(const QString & string)
{
	int size= string.size();
	for(int i =0; i < size; i++)
		insertVTChar(string.at(i));
}

void KTextEditVT::setParseAnsiEscapeCodes(bool parseAnsi)
{
	mParseAnsi = parseAnsi;
}

bool KTextEditVT::parseAnsiEscapeCodes() const
{
	return mParseAnsi;
}

