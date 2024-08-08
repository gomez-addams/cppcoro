///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Lewis Baker
// Licenced under MIT license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCORO_INLINE_SCHEDULER_HPP_INCLUDED
#define CPPCORO_INLINE_SCHEDULER_HPP_INCLUDED

#if __has_include(<coroutine>)
#include <coroutine>
#ifndef SP_DEFINED_EXPERIMENTAL_SUSPEND_NEVER
#define SP_DEFINED_EXPERIMENTAL_SUSPEND_NEVER
namespace std::experimental {
using suspend_never = std::suspend_never;
}
#endif
#elif __has_include(<experimental/coroutine>)
#include <experimental/coroutine>
#endif

namespace cppcoro
{
	class inline_scheduler
	{
	public:

		inline_scheduler() noexcept = default;

		std::experimental::suspend_never schedule() const noexcept
		{
			return {};
		}
	};
}

#endif
