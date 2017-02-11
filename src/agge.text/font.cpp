#include <agge.text/font.h>

using namespace std;

namespace agge
{
	const glyph *font::get_glyph(wchar_t character) const
	{
		uint16_t index = get_glyph_index(character);

		if (index == 0xFFFF)
			return 0;

		glyphs_cache_t::const_iterator i = _glyphs.find(index);

		if (_glyphs.end() == i)
		{
			const glyph *g = load_glyph(index);

			i = _glyphs.insert(make_pair(index, g)).first;
		}
		return i->second;
	}
}