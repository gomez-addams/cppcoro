///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Lewis Baker
// Licenced under MIT license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCORO_ASYNC_SCOPE_HPP_INCLUDED
#define CPPCORO_ASYNC_SCOPE_HPP_INCLUDED

#include <cppcoro/on_scope_exit.hpp>

#include <atomic>
#if __has_include(<coroutine>)
#include <coroutine>
#ifndef SP_DEFINED_EXPERIMENTAL_COROUTINE_HANDLE
#define SP_DEFINED_EXPERIMENTAL_COROUTINE_HANDLE
namespace std::experimental {
template <typename T = void>
using coroutine_handle = std::coroutine_handle<T>;
}
#endif
#ifndef SP_DEFINED_EXPERIMENTAL_SUSPEND_NEVER
#define SP_DEFINED_EXPERIMENTAL_SUSPEND_NEVER
namespace std::experimental {
using suspend_never = std::suspend_never;
}
#endif
#elif __has_include(<experimental/coroutine>)
#include <experimental/coroutine>
#endif
#include <type_traits>
#include <cassert>

namespace cppcoro
{
	class async_scope
	{
	public:

		async_scope() noexcept
			: m_count(1u)
		{}

		~async_scope()
		{
			// scope must be co_awaited before it destructs.
			assert(m_continuation);
		}

		template<typename AWAITABLE>
		void spawn(AWAITABLE&& awaitable)
		{
			[](async_scope* scope, std::decay_t<AWAITABLE> awaitable) -> oneway_task
			{
				scope->on_work_started();
				auto decrementOnCompletion = on_scope_exit([scope] { scope->on_work_finished(); });
				co_await std::move(awaitable);
			}(this, std::forward<AWAITABLE>(awaitable));
		}

		[[nodiscard]] auto join() noexcept
		{
			class awaiter
			{
				async_scope* m_scope;
			public:
				awaiter(async_scope* scope) noexcept : m_scope(scope) {}

				bool await_ready() noexcept
				{
					return m_scope->m_count.load(std::memory_order_acquire) == 0;
				}

				bool await_suspend(std::experimental::coroutine_handle<> continuation) noexcept
				{
					m_scope->m_continuation = continuation;
					return m_scope->m_count.fetch_sub(1u, std::memory_order_acq_rel) > 1u;
				}

				void await_resume() noexcept
				{}
			};

			return awaiter{ this };
		}

	private:

		void on_work_finished() noexcept
		{
			if (m_count.fetch_sub(1u, std::memory_order_acq_rel) == 1)
			{
				m_continuation.resume();
			}
		}

		void on_work_started() noexcept
		{
			assert(m_count.load(std::memory_order_relaxed) != 0);
			m_count.fetch_add(1, std::memory_order_relaxed);
		}

		struct oneway_task
		{
			struct promise_type
			{
				std::experimental::suspend_never initial_suspend() noexcept { return {}; }
				std::experimental::suspend_never final_suspend() noexcept { return {}; }
				void unhandled_exception() noexcept { std::terminate(); }
				oneway_task get_return_object() noexcept { return {}; }
				void return_void() noexcept {}
			};
		};

		std::atomic<size_t> m_count;
		std::experimental::coroutine_handle<> m_continuation;

	};
}

#endif
