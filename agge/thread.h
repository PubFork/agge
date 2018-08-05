#pragma once

#include <agge/types.h>

namespace agge
{
	class thread : noncopyable
	{
	public:
		typedef void (*thread_function_t)(void *argument);

	public:
		thread(thread_function_t thread_function, void *argument);
		~thread();

	private:
		void *_thread;
		const thread_function_t _thread_function;
		void * const _argument;
	};
}
