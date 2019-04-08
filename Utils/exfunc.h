#pragma once
#ifndef EXFUNC_H
#define EXFUNC_H
#include <cstdint>
#include <utility>
#include <type_traits>
#include <memory>
#include <functional>
#include <array>
namespace exlib {
	template<typename Sig>
	class unique_function;

	namespace unique_func_det {

		template<typename T,bool trivial=std::is_nothrow_destructible<T>::value>
		struct deleter {
			constexpr static void do_delete(void* data)
			{
				static_cast<T*>(data)->~T();
			}
		};
		template<typename T>
		struct deleter<T,true> {
			constexpr static void do_delete(void* data)
			{}
		};

		constexpr size_t max_size_count = 4;
		constexpr size_t max_size = sizeof(std::size_t[max_size_count]);

		template<typename T,bool fits=sizeof(T)<=max_size>
		struct indirect_deleter {
			constexpr static void do_delete(void* data)
			{
				deleter<T>::do_delete(data);
			}
		};
		template<typename Func>
		struct indirect_deleter<Func,false> {
			constexpr static void do_delete(void* data)
			{
				auto const real_data=*static_cast<Func**>(data);
				deleter<Func>::do_delete(real_data);
				delete real_data;
			}
		};
		
		template<typename Func,typename Sig,bool fits=sizeof(Func)<=max_size>
		struct indirect_call;

		template<typename Func,typename Ret,typename... Args>
		struct indirect_call<Func,Ret(Args...),true> {
			static Ret call_me(void* obj,Args... args)
			{
				return (*static_cast<Func*>(obj))(std::forward<Args>(args)...);
			}
		};
		template<typename Func,typename Ret,typename... Args>
		struct indirect_call<Func,Ret(Args...),false> {
			static Ret call_me(void* obj,Args... args)
			{
				return (**static_cast<Func**>(obj))(std::forward<Args>(args)...);
			}
		};

		template<typename Func,bool fits=sizeof(Func)<=max_size>
		struct func_constructor {
			static void construct(void* location,Func func)
			{
				new (location) Func{std::move(func)};
			}
		};

		template<typename Func>
		struct func_constructor<Func,false> {
			template<typename Func>
			static void construct(void* location,Func func)
			{
				*static_cast<Func**>(location)=new Func{std::move(func)};
			}
		};
	}

	template<typename Ret,typename... Args>
	class unique_function<Ret(Args...)> {
		void(*_deleter)(void*);
		Ret(*_func)(void*,Args...);
		std::array<std::size_t,unique_func_det::max_size_count> _data;
		void cleanup()
		{
			if(_deleter)
			{
				_deleter(static_cast<void*>(_data.data()));
			}
		}
	public:
		unique_function() noexcept:_deleter{nullptr},_func{nullptr}{}

		template<typename Func>
		unique_function(Func func):
			_deleter{&unique_func_det::indirect_deleter<Func>::do_delete},
			_func{&unique_func_det::indirect_call<Func,Ret(Args...)>::call_me}
		{
			static_assert(alignof(Func)<=alignof(decltype(_data)),"Overaligned functions not supported");
			unique_func_det::func_constructor<Func>::construct(_data.data(),std::move(func));
		}

		unique_function(unique_function&& o) noexcept:_func{std::exchange(o._func,nullptr)},_deleter{std::exchange(o._deleter,nullptr)},_data{o._data}
		{
		}

		explicit operator bool() const noexcept
		{
			return _func;
		}

		Ret operator()(Args... args) const
		{
			if(_func)
			{
				return _func(static_cast<void*>(const_cast<size_t*>(_data.data())),std::forward<Args>(args)...);
			}
			throw std::bad_function_call{};
		}

		template<typename Func>
		unique_function& operator=(Func func)
		{
			unique_function uf{std::move(func)};
			uf.swap(*this);
			return *this;
		}

		void swap(unique_function& other) noexcept
		{
			std::swap(_deleter,other._deleter);
			std::swap(_func,other._func);
			std::swap(_data,other._data);
		}

		~unique_function()
		{
			cleanup();
		}
	};
}
#endif