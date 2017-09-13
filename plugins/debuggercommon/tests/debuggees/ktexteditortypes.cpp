#include <ktexteditor/cursor.h>
#include <ktexteditor/range.h>

int main()
{
    KTextEditor::Cursor cursor(1, 1);

    KTextEditor::Range range(1, 1, 2, 2);
    return 0;
}
