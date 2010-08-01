#include "StdAfx.h"
#include "Text.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma region CText

void CText::FireOnChange (
    unsigned int start_line, unsigned int start_position, 
    unsigned int old_end_line, unsigned int old_end_position, 
    unsigned int new_end_line, unsigned int new_end_position)
{
    ASSERT (start_line < GetLinesCount ());
    ASSERT (start_position <= GetLineLength (start_line));
    ASSERT (new_end_line < GetLinesCount ());
    ASSERT (new_end_position <= GetLineLength (new_end_line));
    ASSERT (old_end_line > start_line || (old_end_line == start_line && old_end_position >= start_position));
    ASSERT (new_end_line > start_line || (new_end_line == start_line && new_end_position >= start_position));

    if (listener != NULL && (start_line != old_end_line || start_position != old_end_position || start_line != new_end_line || start_position != new_end_position))
        listener->OnChange (start_line, start_position, old_end_line, old_end_position, new_end_line, new_end_position);
}

void CText::SetListener (CTextListener *listener)
{
    ASSERT (CText::listener == NULL);
    ASSERT (listener != NULL);

    CText::listener = listener;
}

#pragma endregion

#pragma region CCharBufferText

unsigned int CCharBufferText::GetLastLineEndingBefore (unsigned int position)
{
    if (lines_count == 0) return lines_count;

    if (line_start [0] + line_length [0] > position) return lines_count;

    unsigned int last_line = lines_count - 1;
    if (line_start [last_line] + line_length [last_line] <= position) return last_line;

    unsigned int a = 0;
    unsigned int b = last_line;

    while (b - a > 1)
    {
        unsigned int m = (a + b) / 2;

        if (line_start [m] + line_length [m] > position) b = m;
        else a = m;
    }

    return a;
}

unsigned int CCharBufferText::GetFirstLineStartingAfter (unsigned int position)
{
    if (lines_count == 0) return lines_count;

    if (line_start [0] >= position) return 0;

    unsigned int last_line = lines_count - 1;
    if (line_start [last_line] < position) return lines_count;

    unsigned int a = 0;
    unsigned int b = last_line;

    while (b - a > 1)
    {
        unsigned int m = (a + b) / 2;

        if (line_start [m] < position) a = m;
        else b = m;
    }

    return b;
}

unsigned int CCharBufferText::GetLine (unsigned int position)
{
    if (lines_count == 0) return lines_count;

    if (line_start [0] > position) return lines_count;

    unsigned int last_line = lines_count - 1;
    if (line_start [last_line] <= position) return last_line;

    unsigned int a = 0;
    unsigned int b = last_line;

    while (b - a > 1)
    {
        unsigned int m = (a + b) / 2;

        if (line_start [m] > position) b = m;
        else a = m;
    }

    return a;
}

void CCharBufferText::SplitIntoLines (unsigned int start, unsigned int count, std::vector <unsigned int> &starts, std::vector <unsigned int> &lengths)
{
    ASSERT (start <= data.GetSize ());
    ASSERT (start + count <= data.GetSize ());
    ASSERT (starts.size () == 0);
    ASSERT (lengths.size () == 0);

    UNICHAR buffer [65536];

    unsigned int ls = 0;
    UNICHAR pch = 0;
    for (unsigned int i = 0; i < count; i++)
    {
        if (i % 65536 == 0)
            data.GetCharsRange (start + i, min (65536, count - i), buffer);

        UNICHAR ch = buffer [i % 65536];
        if (ch == L'\n' || ch == L'\r')
        {
            if (pch == 0 || pch == ch)
            {
                starts.push_back (start + ls);
                lengths.push_back (i - ls);
                pch = ch;
            }
            else
            {
                pch = 0;
            }

            ls = i + 1;
        }
        else pch = 0;
    }

    starts.push_back (start + ls);
    lengths.push_back (count - ls);
}

void CCharBufferText::ReplaceRange (std::vector <unsigned int> &v, unsigned int start, unsigned int count, std::vector <unsigned int> &replacement, unsigned int replacement_start, unsigned int replacement_count)
{
    unsigned int overlap = min (count, replacement_count);

    if (overlap > 0)
        memcpy (&v [start], &replacement [replacement_start], overlap * sizeof (unsigned int));

    if (replacement_count > count)
        v.insert (v.begin () + start + count, replacement.begin () + replacement_start + count, replacement.begin () + replacement_start + replacement_count);
    else if (replacement_count < count)
        v.erase (v.begin () + start + replacement_count, v.begin () + start + count);
}

void CCharBufferText::SetUndoManager (CUndoManager *undo_manager)
{
    ASSERT (CCharBufferText::undo_manager == NULL);
    ASSERT (undo_manager != NULL);

    CCharBufferText::undo_manager = undo_manager;
}

CCharBufferText::CCharBufferText (CCharBuffer &data) : data (data), undo_manager (NULL)
{
    SplitIntoLines (0, data.GetSize (), line_start, line_length);

    lines_count = line_start.size ();
    ASSERT (lines_count > 0);

    data.SetListener (this);
}

unsigned int CCharBufferText::GetLinesCount ()
{
    return line_length.size ();
}

unsigned int CCharBufferText::GetLineLength (unsigned int line)
{
    ASSERT (line < line_length.size ());

    return line_length [line];
}

UNICHAR CCharBufferText::GetCharAt (unsigned int line, unsigned int position)
{
    ASSERT (line < line_length.size ());
    ASSERT (position < line_length [line]);

    UNICHAR ch;

    data.GetCharsRange (line_start [line] + position, 1, &ch);

    return ch;
}

void CCharBufferText::GetCharsRange (unsigned int line, unsigned int start_position, unsigned int count, UNICHAR buffer [])
{
    ASSERT (line < line_length.size ());
    ASSERT (start_position <= line_length [line]);
    ASSERT (start_position + count <= line_length [line]);
    ASSERT (count == 0 || buffer != NULL);

    data.GetCharsRange (line_start [line] + start_position, count, buffer);
}

void CCharBufferText::InsertCharAt (unsigned int line, unsigned int position, UNICHAR character)
{
    ASSERT (line < line_length.size ());
    ASSERT (position <= line_length [line]);

    data.ReplaceCharsRange (line_start [line] + position, 0, 1, &character);
}

void CCharBufferText::SetCharAt (unsigned int line, unsigned int position, UNICHAR character)
{
    ASSERT (line < line_length.size ());
    ASSERT (position < line_length [line]);

    data.ReplaceCharsRange (line_start [line] + position, 1, 1, &character);
}

void CCharBufferText::RemoveCharAt (unsigned int line, unsigned int position)
{
    ASSERT (line < line_length.size ());
    ASSERT (position < line_length [line]);

    data.ReplaceCharsRange (line_start [line] + position, 1, 0, NULL);
}

void CCharBufferText::ReplaceCharsRange (unsigned int line, unsigned int start_position, unsigned int count, unsigned int replacement_length, UNICHAR replacement [])
{
    ASSERT (line < line_length.size ());
    ASSERT (start_position <= line_length [line]);
    ASSERT (start_position + count <= line_length [line]);
    ASSERT (replacement_length == 0 || replacement != NULL);

    data.ReplaceCharsRange (line_start [line] + start_position, count, replacement_length, replacement);
}

void CCharBufferText::BreakLineAt (unsigned int line, unsigned int position)
{
    static UNICHAR EOL [] = {'\r', '\n'};

    ASSERT (line < line_length.size ());
    ASSERT (position <= line_length [line]);

    data.ReplaceCharsRange (line_start [line] + position, 0, 2, EOL);
}

void CCharBufferText::JoinLines (unsigned int line)
{
    ASSERT (line < line_length.size () - 1);

    unsigned int delta = line_start [line + 1] - line_start [line] - line_length [line];

    data.ReplaceCharsRange (line_start [line + 1] - delta, delta, 0, NULL);
}

void CCharBufferText::InsertLineAt (unsigned int line, unsigned int length, UNICHAR characters [])
{
    ASSERT (line <= line_length.size () - 1);

    InsertLinesAt (line, 1, &length, &characters);
}

void CCharBufferText::RemoveLineAt (unsigned int line)
{
    ASSERT (line < line_length.size ());

    RemoveLinesAt (line, 1);
}

void CCharBufferText::InsertLinesAt (unsigned int line, unsigned int count, unsigned int length [], UNICHAR *characters [])
{
    static UNICHAR EOL [] = {'\r', '\n'};

    ASSERT (line <= GetLinesCount ());
    ASSERT (count == 0 || length != NULL);
    ASSERT (count == 0 || characters != NULL);

    unsigned int l = 0;
    for (unsigned int i = 0; i < count; i++)
        l += length [i] + 2;

    UNICHAR *buffer = new UNICHAR [l];

    if (line < lines_count)
    {
        unsigned int pos = 0;
        for (unsigned int i = 0; i < count; i++)
        {
            memcpy (&buffer [pos], characters [i], length [i] * sizeof (UNICHAR));
            pos += length [i];
            memcpy (&buffer [pos], EOL, 2 * sizeof (UNICHAR));
            pos += 2;
        }

        data.ReplaceCharsRange (line_start [line], 0, l, buffer);
    }
    else
    {
        unsigned int pos = 0;
        for (unsigned int i = 0; i < count; i++)
        {
            memcpy (&buffer [pos], EOL, 2 * sizeof (UNICHAR));
            pos += 2;
            memcpy (&buffer [pos], characters [i], length [i] * sizeof (UNICHAR));
            pos += length [i];
        }

        data.ReplaceCharsRange (data.GetSize (), 0, l, buffer);
    }

    delete [] buffer;
}

void CCharBufferText::RemoveLinesAt (unsigned int line, unsigned int count)
{
    ASSERT (line <= GetLinesCount ());
    ASSERT (line + count <= GetLinesCount ());

    unsigned int start = line_start [line];
    unsigned int end_line = line + count;
    unsigned int end = end_line < lines_count ? line_start [end_line] : data.GetSize ();

    data.ReplaceCharsRange (start, end - start, 0, NULL);
}

void CCharBufferText::OnChange (unsigned int start, unsigned int old_count, unsigned int new_count)
{
    unsigned int c_start_line = GetLine (start);
    ASSERT (c_start_line < lines_count);
    unsigned int c_start_position = min (start - line_start [c_start_line], line_length [c_start_line]);

    unsigned int c_old_end_line = GetLine (start + old_count);
    ASSERT (c_old_end_line < lines_count);
    unsigned int c_old_end_position = min (start + old_count - line_start [c_old_end_line], line_length [c_old_end_line]);

    int delta = new_count - old_count;

    unsigned int before = GetLastLineEndingBefore (start);

    if (before > 0 && before < lines_count && line_length [before] == 0) before -= 1;

    unsigned int start_line = before >= lines_count ? 0 : before + 1;
    unsigned int start_pos = before >= lines_count ? 0 : line_start [before] + line_length [before];
    unsigned int after = GetFirstLineStartingAfter (start + old_count);

    if (after < lines_count && line_length [after] == 0) after += 1;

    unsigned int end_line = after >= lines_count ? lines_count : after;
    unsigned int end_pos = after >= lines_count ? data.GetSize () : line_start [after] + delta;
    unsigned int count = end_pos - start_pos;

    vector <unsigned int> starts;
    vector <unsigned int> lengths;

    SplitIntoLines (start_pos, count, starts, lengths);

    int c = starts.size ();
    ASSERT (c > 0);

    if (c == 1 && before < lines_count && after < lines_count) // Join three lines
    {
        line_length [before] += lengths [0] + line_length [after];
        line_length.erase (line_length.begin () + before + 1, line_length.begin () + after + 1);
        line_start.erase (line_start.begin () + before + 1, line_start.begin () + after + 1);

        if (delta != 0)
        {
            unsigned int size = line_length.size ();
            for (unsigned int i = before + 1; i < size; i++)
                line_start [i] += delta;
        }
    }
    else
    {
        unsigned int b = 0;
        unsigned int e = c;

        if (before < lines_count) // Append line
        {
            line_length [before] += lengths [0];
            b = 1;
        }

        if (after < lines_count) // Prepend line
        {
            line_start [after] -= lengths [c - 1];
            line_length [after] += lengths [c - 1];
            e = c - 1;
        }

        if (end_line < start_line) end_line = start_line;

        ReplaceRange (line_length, start_line, end_line - start_line, lengths, b, e - b);
        ReplaceRange (line_start, start_line, end_line - start_line, starts, b, e - b);

        if (delta != 0)
        {
            unsigned int size = line_length.size ();
            for (unsigned int i = (before < lines_count ? before + 1 : 0) + e - b; i < size; i++)
                line_start [i] += delta;
        }
    }

    lines_count = line_length.size ();

    unsigned int c_new_end_line = GetLine (start + new_count);
    ASSERT (c_new_end_line < lines_count);
    unsigned int c_new_end_position = min (start + new_count - line_start [c_new_end_line], line_length [c_new_end_line]);

    FireOnChange (c_start_line, c_start_position, c_old_end_line, c_old_end_position, c_new_end_line, c_new_end_position);
}

#pragma endregion
