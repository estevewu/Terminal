/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

#include "precomp.h"
#include "Row.hpp"
#include "CharRow.hpp"
#include "textBuffer.hpp"
#include "../types/inc/convert.hpp"

// Routine Description:
// - constructor
// Arguments:
// - rowId - the row index in the text buffer
// - rowWidth - the width of the row, cell elements
// - fillAttribute - the default text attribute
// - pParent - the text buffer that this row belongs to
// Return Value:
// - constructed object
ROW::ROW(const SHORT rowId, const short rowWidth, const TextAttribute fillAttribute, TextBuffer* const pParent) :
    _id{ rowId },
    _rowWidth{ gsl::narrow<size_t>(rowWidth) },
    _charRow{ gsl::narrow<size_t>(rowWidth), this },
    _attrRow{ gsl::narrow<UINT>(rowWidth), fillAttribute },
    _pParent{ pParent }
{
}

size_t ROW::size() const noexcept
{
    return _rowWidth;
}

const CharRow& ROW::GetCharRow() const
{
    return _charRow;
}

CharRow& ROW::GetCharRow()
{
    return const_cast<CharRow&>(static_cast<const ROW* const>(this)->GetCharRow());
}

const ATTR_ROW& ROW::GetAttrRow() const noexcept
{
    return _attrRow;
}

ATTR_ROW& ROW::GetAttrRow() noexcept
{
    return const_cast<ATTR_ROW&>(static_cast<const ROW* const>(this)->GetAttrRow());
}

SHORT ROW::GetId() const noexcept
{
    return _id;
}

void ROW::SetId(const SHORT id) noexcept
{
    _id = id;
}

// Routine Description:
// - Sets all properties of the ROW to default values
// Arguments:
// - Attr - The default attribute (color) to fill
// Return Value:
// - <none>
bool ROW::Reset(const TextAttribute Attr)
{
    _charRow.Reset();
    try
    {
        _attrRow.Reset(Attr);
    }
    catch (...)
    {
        LOG_CAUGHT_EXCEPTION();
        return false;
    }
    return true;
}

// Routine Description:
// - resizes ROW to new width
// Arguments:
// - width - the new width, in cells
// Return Value:
// - S_OK if successful, otherwise relevant error
[[nodiscard]]
HRESULT ROW::Resize(const size_t width)
{
    RETURN_IF_FAILED(_charRow.Resize(width));
    try
    {
        _attrRow.Resize(width);
    }
    CATCH_RETURN();

    _rowWidth = width;

    return S_OK;
}

// Routine Description:
// - clears char data in column in row
// Arguments:
// - column - 0-indexed column index
// Return Value:
// - <none>
void ROW::ClearColumn(const size_t column)
{
    THROW_HR_IF(E_INVALIDARG, column >= _charRow.size());
    _charRow.ClearCell(column);
}

// Routine Description:
// - gets the text of the row as it would be shown on the screen
// Return Value:
// - wstring containing text for the row
std::wstring ROW::GetText() const
{
    return _charRow.GetText();
}

// Routine Description:
// - gets the cell data for the row
// Return Value:
// - vector of cell data for row, one object per column
std::vector<OutputCell> ROW::AsCells() const
{
    return AsCells(0, size());
}

// Routine Description:
// - gets the cell data for the row
// Arguments:
// - startIndex - index to start fetching data from
// Return Value:
// - vector of cell data for row, one object per column
std::vector<OutputCell> ROW::AsCells(const size_t startIndex) const
{
    return AsCells(startIndex, size() - startIndex);
}

// Routine Description:
// - gets the cell data for the row
// Arguments:
// - startIndex - index to start fetching data from
// - count - the number of cells to grab
// Return Value:
// - vector of cell data for row, one object per column
std::vector<OutputCell> ROW::AsCells(const size_t startIndex, const size_t count) const
{
    std::vector<OutputCell> cells;
    cells.reserve(count);

    ATTR_ROW::const_iterator it = std::next(_attrRow.begin(), startIndex);
    for (size_t i = 0; i < count; ++i, ++it)
    {
        const auto index = startIndex + i;
        const auto glyph = _charRow.GlyphAt(index);
        cells.emplace_back(glyph, _charRow.DbcsAttrAt(index), *it);
    }
    return cells;
}

const OutputCell ROW::at(const size_t column) const
{
    const auto glyph = _charRow.GlyphAt(column);
    return { glyph, _charRow.DbcsAttrAt(column), _attrRow.GetAttrByColumn(column) };
}

UnicodeStorage& ROW::GetUnicodeStorage()
{
    return _pParent->GetUnicodeStorage();
}

const UnicodeStorage& ROW::GetUnicodeStorage() const
{
    return _pParent->GetUnicodeStorage();
}
