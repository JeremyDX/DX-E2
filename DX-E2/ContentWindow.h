#pragma once

#include <cstdint>

class ContentWindow
{
	public:
		ContentWindow();
		~ContentWindow();

	public:
		int32_t background_color;
		int16_t background_shader_id = -1;

	public:
		int32_t state_change_alias[4];
		int16_t state_vertex_offsets[4];
		int16_t state_vertex_sizes[4];
		char state_changes;

	public:
		void (*update)();
		void (*children)();
		void SetUpdateProc(int index);
		void SetChildUpdateProc(int index, int size, int disabled_bits);

	public:
		int menu_index;
		int menu_size;
		int disabled_menu_bits;
};
