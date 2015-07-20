#pragma once

#include "config.h"

#include <vector>

namespace agge
{
	template <typename RendererT>
	class scanline_adapter
	{
	public:
		typedef typename RendererT::cover_type cover_type;
		typedef std::vector<cover_type> covers_buffer_type;

	public:
		scanline_adapter(RendererT &renderer, covers_buffer_type &covers, size_t max_length);

		bool begin(int y);
		void add_cell(int x, cover_type cover);
		void add_span(int x, unsigned int length, cover_type cover);
		void commit(int next_x = 0);

	private:
		scanline_adapter(const scanline_adapter &other);
		const scanline_adapter &operator =(const scanline_adapter &rhs);

	private:
		RendererT &_renderer;
		cover_type *_cover, *_start_cover;
		int _x, _start_x;
	};



	template <typename RendererT>
	inline scanline_adapter<RendererT>::scanline_adapter(RendererT &renderer, covers_buffer_type &covers, size_t max_length)
		: _renderer(renderer), _x(0), _start_x(0)
	{
		max_length += 16;
		if (max_length > covers.size())
			covers.resize(max_length);
		_cover = _start_cover = &covers[3];
	}

	template <typename RendererT>
	inline bool scanline_adapter<RendererT>::begin(int y)
	{	return _renderer.set_y(y);	}

	template <typename RendererT>
	AGGE_INLINE void scanline_adapter<RendererT>::add_cell(int x, cover_type cover)
	{
		if (x != _x)
			commit(x);
		++_x;
		*_cover++ = cover;
	}

	template <typename RendererT>
	AGGE_INLINE void scanline_adapter<RendererT>::add_span(int x, unsigned int length, cover_type cover)
	{
		if (x != _x)
			commit(x);
		
		cover_type *p = _cover;
		
		_x += length;
		_cover += length;

		while (length--)
			*p++ = cover;
	}

	template <typename RendererT>
	AGGE_INLINE void scanline_adapter<RendererT>::commit(int next_x)
	{
		*reinterpret_cast<int *>(_cover) = 0;
		_renderer(_start_x, _x - _start_x, _start_cover);
		_start_x = _x = next_x;
		_cover = _start_cover;
	}
}
