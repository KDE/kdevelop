#ifndef _CARET_H_
#define _CARET_H_

class Caret
{
  public:
  long      m_row;
  long      m_idx;
            Caret();
            Caret(const int row, const int idx);
            Caret(const Caret& value);
  bool      operator<(const Caret& compare);
  bool      operator>(const Caret& compare);
  bool      operator<=(const Caret& compare);
  bool      operator>=(const Caret& compare);
  bool      operator==(const Caret& compare);
  bool      operator!=(const Caret& compare);
  Caret     operator=(const Caret& value);
  Caret     operator+(const Caret& value);
  Caret     operator-(const Caret& value);

};

#endif
