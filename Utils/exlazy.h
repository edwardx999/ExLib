/*
Copyright 2019 Edward Xie

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef EXLAZY_H
#define EXLAZY_H
#include "exretype.h"
#include <map>
#include <utility>
#include <mutex>
#include <atomic>
#include <shared_mutex>
#define _EXLAZY_HAS_CPP_20 _EXRETYPE_HAS_CPP_20
#define _EXLAZY_HAS_CPP_17 _EXRETYPE_HAS_CPP_17
#define _EXLAZY_HAS_CPP_14 _EXRETYPE_HAS_CPP_14
namespace exlib {

	namespace lazy_detail {
		template<typename Sig>
		struct get_sig;

		template<typename Ret,typename... Args>
		struct get_sig<Ret(Args...)> {
			using result_type=Ret;
			using input_type=std::tuple<Args...>;
			using default_map=std::map<input_type,result_type>;
		};
		template<typename Func>
		using lazy_base_func=typename std::remove_cv<typename std::remove_reference<decltype(exlib::wrap(std::declval<Func const&>()))>::type>::type;
	}
	template<typename Sig,typename Func,typename MemoMap=typename lazy_detail::get_sig<Sig>::default_map>
	class memoized;

	template<typename Ret,typename... Args,typename Func,typename MemoMap>
	class memoized<Ret(Args...),Func,MemoMap>:lazy_detail::lazy_base_func<Func> {
		mutable MemoMap map;
		using Base=lazy_detail::lazy_base_func<Func>;
	public:
		template<typename F>
		memoized(F&& func):Base(exlib::wrap(std::forward<F>(func)))
		{}
		template<typename... T>
		Ret const& operator()(T&& ... args) const
		{
			auto tuple=std::make_tuple(args);
			auto pos=map.find(tuple);
			if(pos==map.end())
			{
				auto result=Base::operator()(std::forward<T>(args)...);
				auto position=map.emplace({std::move(tuple),std::move(result)});
				return position.first->second;
			}
			else
			{
				return pos->second;
			}
		}
	};


#if _EXLAZY_HAS_CPP_17
	template<typename Sig,typename MemoMap,typename F>
	memoized(F&&)->memoized<Sig,std::remove_cv_t<std::remove_reference_t<F>>,MemoMap>;

	template<typename MemoMap,typename Ret,typename...Args>
	memoized(Ret(&)(Args...))->memoized<Ret(Args...),Ret(*)(Args...),MemoMap>;

	template<typename MemoMap,typename Ret,typename...Args>
	memoized(Ret(*)(Args...))->memoized<Ret(Args...),Ret(*)(Args...),MemoMap>;
#endif

	namespace lazy_detail {
		template<typename F,typename MemoMap>
		struct make_memoized;

		template<typename Ret,typename... Args,typename MemoMap>
		struct make_memoized<Ret(Args...),MemoMap> {
			template<typename Func>
			static auto get(Func&& f) ->memoized<Ret(Args...),typename std::remove_cv<typename std::remove_reference<Func>::type>::type,MemoMap>
			{
				return {std::forward<Func>(f)};
			}
		};
	}

	template<typename Signature,typename MemoMap=typename lazy_detail::get_sig<Signature>::default_map,typename Func>
	auto make_memoized(Func&& f) -> decltype(lazy_detail::make_memoized<Signature,MemoMap>::get(std::forward<Func>(f)))
	{
		return lazy_detail::make_memoized<Signature,MemoMap>::get(std::forward<Func>(f));
	}

	template<typename Sig,typename Func,typename MemoMap=typename lazy_detail::get_sig<Sig>::default_map>
	class concurrent_memoized;

	template<typename Ret,typename... Args,typename Func,typename MemoMap>
	class concurrent_memoized<Ret(Args...),Func,MemoMap>:lazy_detail::lazy_base_func<Func> {
		mutable MemoMap _map;
		mutable std::shared_mutex _mtx;
		using Base=lazy_detail::lazy_base_func<Func>;
	public:
		template<typename F>
		concurrent_memoized(F&& func):Base(exlib::wrap(std::forward<F>(func)))
		{}
		template<typename... T>
		Ret const& operator()(T&& ... args) const
		{
			auto tuple=std::make_tuple(args);
			std::shared_lock<std::mutex> lock(_mtx);
			auto pos=_map.find(tuple);
			if(pos==_map.end())
			{
				lock.unlock_shared();
				{
					std::unique_lock<std::mutex> ulock(_mtx);
					auto pos=_map.find(tuple);
					if(pos==_map.end())
					{
						auto result=Base::operator()(std::forward<T>(args)...);
						auto position=_map.emplace({std::move(tuple),std::move(result)});
						return position.first->second;
					}
				}
			}
			else
			{
				return pos->second;
			}
		}
	};

	template<typename Func>
	class lazy:lazy_detail::lazy_base_func<Func> {
		using Base=lazy_detail::lazy_base_func<Func>;
	public:
		using result_type=decltype(std::declval<Base const&>()());
		using reference=result_type&;
		using const_reference=result_type const&;
	private:
		using storage_base=wrap_reference_t<result_type>;
		using storage_type=typename std::aligned_storage<sizeof(storage_base),alignof(storage_base)>::type;
		bool mutable _initialized=false;
		storage_type mutable _storage;
		void do_init() const
		{
			if(!_initialized)
			{
				new (get_pointer()) result_type(Base::operator()());
				_initialized=true;
			}
		}
		result_type* get_pointer() const
		{
#if _EXLAZY_HAS_CPP_17
			return std::launder(reinterpret_cast<result_type*>(&_storage));
#else
			return reinterpret_cast<result_type*>(&_storage);
#endif
		}
	public:
		template<typename F>
		lazy(F&& f):Base(exlib::wrap(std::forward<F>(f)))
		{}
		operator reference ()
		{
			do_init();
			return *get_pointer();
		}
		operator const_reference() const
		{
			do_init();
			return *get_pointer();
		}
		~lazy() noexcept
		{
			if(_initialized)
			{
				get_pointer()->~result_type();
			}
		}
	};

#if _EXLAZY_HAS_CPP_17
	template<typename F>
	lazy(F&& f)->lazy<std::decay_t<F>>;
#endif

	template<typename F>
	lazy<typename std::decay<F>::type> make_lazy(F&& f)
	{
		return {std::forward<F>(f)};
	}

	template<typename Func>
	class concurrent_lazy:lazy_detail::lazy_base_func<Func> {
		using Base=lazy_detail::lazy_base_func<Func>;
	public:
		using result_type=decltype(std::declval<Base const&>()());
		using reference=result_type&;
		using const_reference=result_type const&;
	private:
		using storage_base=wrap_reference_t<result_type>;
		using storage_type=typename std::aligned_storage<sizeof(storage_base),alignof(storage_base)>::type;
		std::atomic<bool> mutable _initialized=false;
		std::mutex mutable _mtx;
		storage_type mutable _storage;
		void do_init() const
		{
			if(!_initialized.load(std::memory_order_acquire))
			{
				std::lock_guard<std::mutex> lock(_mtx);
				if(!_initialized.load(std::memory_order_relaxed))
				{
					new (get_pointer()) result_type(Base::operator()());
					_initialized.store(true,std::memory_order_release);
				}
			}
		}
		result_type* get_pointer() const
		{
#if _EXLAZY_HAS_CPP_17
			return std::launder(reinterpret_cast<result_type*>(&_storage));
#else
			return reinterpret_cast<result_type*>(&_storage);
#endif
		}
	public:
		template<typename F>
		concurrent_lazy(F&& f):Base(exlib::wrap(std::forward<F>(f)))
		{}
		operator reference ()
		{
			do_init();
			return *get_pointer();
		}
		operator const_reference () const
		{
			do_init();
			return *get_pointer();
		}
		~concurrent_lazy() noexcept
		{
			if(_initialized.load(std::memory_order_relaxed))
			{
				get_pointer()->~result_type();
			}
		}
	};

#if _EXLAZY_HAS_CPP_17
	template<typename F>
	concurrent_lazy(F&& f)->concurrent_lazy<std::decay_t<F>>;
#endif

	template<typename F>
	concurrent_lazy<typename std::decay<F>::type> make_concurrent_lazy(F&& f)
	{
		return {std::forward<F>(f)};
	}

	template<typename Func>
	class lazy_forward:lazy_detail::lazy_base_func<Func> {
		using Base=lazy_detail::lazy_base_func<Func>;
	public:
		template<typename F>
		lazy_forward(F&& f):Base(exlib::wrap(std::forward<F>(f))){}
		auto operator()() const -> decltype(Base::operator()())
		{
			return Base::operator()();
		}
	};

#if _EXLAZY_HAS_CPP_17
	template<typename F>
	lazy_forward(F&& f)->lazy_forward<std::decay_t<F>>;
#endif

	template<typename F>
	lazy_forward<typename std::decay<F>::type> make_lazy_forward(F&& f)
	{
		return {std::forward<F>(f)};
	}

	template<typename Func>
	class lazy_construct {
		Func _func;
	public:
		template<typename F>
		lazy_construct(F&& func) noexcept(noexcept(Func{std::forward<F>(func))):_func{std::forward<F>(func)} {}
		template<typename T>
		operator T() const noexcept(noexcept(_func()))
		{
			return _func();
		}
		template<typename T>
		operator T() noexcept(noexcept(_func()))
		{
			return _func();
		}
	};

	template<typename Func>
	lazy_construct<typename std::decay<Func>::type> make_lazy_construct(Func&& func)
	{
		return lazy_construct<typename std::decay<Func>::type>(std::forward<Func>(func));
	}

#if _EXLAZY_HAS_CPP_17
	template<typename F>
	lazy_construct(F&&) -> lazy_construct<std::decay_t<F>>;
#endif
}
#endif