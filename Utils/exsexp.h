/*
Copyright 2018 Edward Xie

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef EXSEXP_H
#define EXSEXP_H

#include <utility>
#include <stdexcept>
#include <string>
#include <vector>
#include <memory>
#include <string_view>
#include <unordered_map>
namespace exlib {

	namespace sexp {

		class undeclared_variable:std::logic_error {
		public:
			using std::logic_error::logic_error;
		};

		template<typename InputMap>
		class Expression {
		public:
			typedef typename InputMap::key_type Input;
			typedef typename InputMap::mapped_type Output;
			virtual Output value(InputMap const&) const=0;
		};

		template<typename Input>
		[[noreturn]] void throw_undeclared_variable(Input const&)
		{
			throw undeclared_variable("Undeclared variable");
		}

		[[noreturn]] void throw_undeclared_variable(std::string const& name)
		{
			throw undeclared_variable(std::string("Undeclared variable: ").append(name));
		}

		[[noreturn]] void throw_undeclared_variable(std::wstring const& name)
		{
			std::string buffer("Undeclared variable: ");
			buffer.reserve(buffer.length()+name.size());
			for(auto wc:name)
			{
				uint16_t b=wc&0xFF00;
				if(b)
				{
					buffer.push_back(char(b>>8));

				}
				buffer.push_back(char(wc));
			}
			throw undeclared_variable(buffer);
		}

		template<typename InputMap>
		class Variable:Expression<InputMap> {
		public:
			using Expression::Input;
			using Expression::Output;
		private:
			Input _varname;
		public:
			template<typename... T>
			Variable(T&&... args):_varname(std::forward<T>(args)...)
			{}
			Output value(InputMap const& map) const override
			{
				auto p=map.find(_varname);
				if(p==map.end())
				{
					throw_undeclared_var(_varname);
				}
				return p->second;
			}
		};

		template<typename InputMap>
		class Value:public Expression<InputMap> {
		public:
			using Expression::Input;
			using Expression::Output;
		private:
			Output _val;
		public:
			template<typename... T>
			Value(T&&... args):_val(std::forward<T>(args)...)
			{}
			Output value(InputMap const& map) const override
			{
				return _val;
			}
		};

		template<typename InputMap>
		class Function:public Expression<InputMap> {
		protected:
			std::vector<std::unique_ptr<Expression<InputMap>>> _args;
		public:
			template<typename... Args>
			Function(Args&&... args):_args(std::forward<Args>(args)...)
			{}
		};

		template<typename SexpInputIterator>
		class parse_error:std::logic_error {
		private:
			SexpInputIterator _it;
		public:
			SexpInputIterator location() const
			{
				return _it;
			}
			parse_error(std::string const& error,SexpInputIterator loc):logic_error(error),_it(loc)
			{}
		};

		template<size_t Size,typename ErrorType,typename Container,typename... ErrorArgs>
		auto throw_if_wrong_size(Container&& cont,ErrorArgs const&... err_args)
		{
			if(cont.size()!=Size)
			{
				throw ErrorType(err_args...);
			}
			return std::forward<Container>(cont);
		}

		template<typename TypeFactory>
		auto parse_sexp(std::string_view sexp)
		{
			using Type=std::decay<decltype(TypeFactory::create(char const*,char const*))>::type;
			std::unique_ptr<Expression<std::map<std::string,Type>>> ret;
			return ret;
		}
		std::unique_ptr<Expression<std::unordered_map<std::string,double>>> parse_sexp_str_double(std::string_view sexp)
		{
			auto is_whitespace=[](char c)
			{
				return c=='\n'||c=='\r'||c=='\t'||c==' ';
			};
			using Map=std::unordered_map<std::string,double>;
			using Exp=Expression<Map>;
			using Func=Function<Map>;
			using ArgList=std::vector<std::unique_ptr<Exp>>;
			class Add:public Function<Map> {
			public:
				using Function::Function;
				double value(Map const& in) const override
				{
					double ret=0;
					for(auto const& arg:_args)
					{
						ret+=arg->value(in);
					}
					return ret;
				}
			};
			class Subtract:public Function<Map> {
			public:
				using Function::Function;
				double value(Map const& in) const override
				{
					if(_args.empty())
					{
						return 0;
					}
					auto it=_args.begin();
					double ret=(*it)->value(in);
					++it;
					for(;it!=_args.end();++it)
					{
						ret-=(*it)->value(in);
					}
					return ret;
				}
			};
			class Multiply:public Function<Map> {
			public:
				using Function::Function;
				double value(Map const& in) const override
				{
					double ret=1;
					for(auto const& arg:_args)
					{
						ret*=arg->value(in);
					}
					return ret;
				}
			};
			class Divide:public Function<Map> {
			public:
				using Function::Function;
				double value(Map const& in) const override
				{
					if(_args.empty())
					{
						return 1;
					}
					auto it=_args.begin();
					double ret=(*it)->value(in);
					++it;
					for(;it!=_args.end();++it)
					{
						ret/=(*it)->value(in);
					}
					return ret;
				}
			};
			class Equals:public Function<Map> {
			public:
				Equals(std::vector<std::unique_ptr<Exp>>&& args):
					Function(throw_if_wrong_size<2,std::invalid_argument>(std::move(args),"= requires 2 arugments"))
				{}
				double value(Map const& in) const override
				{
					if(_args[0]->value(in)==_args[1]->value(in))
					{
						return 1;
					}
					return 0;
				}
			};
			class Or:public Func {
			public:
				using Func::Function;
				double value(Map const& in) const override
				{
					for(auto const& a:_args)
					{
						if(a->value(in)!=0)
						{
							return 1;
						}
					}
					return 0;
				}
			};
			class And:public Func {
			public:
				using Func::Function;
				double value(Map const& in) const override
				{
					for(auto const& a:_args)
					{
						if(a->value(in)==0)
						{
							return 0;
						}
					}
					return 1;
				}
			};
			class Xor:public Func {
			public:
				using Func::Function;
				double value(Map const& in) const override
				{
					double value=0;
					for(auto const& a:_args)
					{
						if(a->value(in)==1)
						{
							value=value==0?1:0;
						}
					}
					return value;
				}
			};
			class Not:public Func {
			public:
				Not(ArgList&& args):Function(throw_if_wrong_size<1,std::invalid_argument>(std::move(args),"not only takes 1 argument"))
				{}
				double value(Map const& in) const override
				{
					if(_args[0]->value(in)==0)
					{
						return 1;
					}
					return 0;
				}
			};
			class If:public Func {
			public:
				If(ArgList&& args):Function(throw_if_wrong_size<3,std::invalid_argument>(std::move(args),"if requires 3 arguments"))
				{}
				double value(Map const& in) const override
				{
					if(_args[0]->value(in)==0)
					{
						return _args[2]->value(in);
					}
					return _args[1]->value(in);
				}
			};
			using Error=parse_error<char const*>;
			std::unique_ptr<Expression<Map>> ret;
			if(sexp.empty())
			{
				return ret;
			}
			char const* const first=sexp.data();
			if(*first=='(')
			{
				char const* last=&sexp.back();
				for(;;--last)
				{
					if(last==first)
					{
						throw Error("Unmatching parentheses",first);
					}
					if(*last==')')
					{
						break;
					}
					if(!is_whitespace(*last))
					{
						throw Error("Trailing characters",last);
					}
				}
				struct span {
					char const* begin;
					char const* end;
				};
				auto split_args=[is_whitespace,begin=first+1,limit=last-1]()
				{
					std::vector<span> args;

					auto inv=[=](auto c)
					{
						return !is_whitespace(c);
					};
					auto it=std::find_if(begin,limit,inv);
					if(it==limit) throw Error("Expected function args",begin);
					if(*it=='('||*it<'a'||*it>'z') throw Error("Expected function name",it);
					auto end=std::find_if(it+1,limit,inv);
					args.push_back(span({it,end}));
					while(true)
					{
						auto beg=std::find_if(begin,limit,inv);
						if(beg==limit)
						{
							break;
						}
						decltype(beg) end;
						if(*beg=='('){ 
							size_t num_parens=0;
							for(auto it=beg+1;beg<limit;++it)
							{
								if(*it==')');
							}
						}
						else
						{
							
						}
						if(end==limit)
						{
							break;
						}
					}
					return args;
				};
				auto args=split_args();
				if(args.empty())
				{
					throw Error("Expected function name and arguments",first);
				}
			}
		}

	}
}


#endif