#pragma once
#ifndef EXSHUNT_H
#define EXSHUNT_H
#include <variant>
#include <string>
#include <functional>
#include <stdexcept>
#include <string_view>
#include <algorithm>
#include <vector>
namespace exlib {

	namespace shunting_yard_parsers {

		enum class op {
			invalid,
			ternary,_ternary_q,_ternary_colon,
			lor,
			land,
			bor,
			xor,
			band,
			eq,neq,
			lt,le,gt,ge,
			bls,brs,
			plus,minus,
			times,divide,modulo,
			power,
			unary_minus,bnot,lnot,
		};

		constexpr int precedence(op o)
		{
			switch(o)
			{
			case op::ternary:
				return 0;
			case op::lor:
				return 1;
			case op::land:
				return 2;
			case op::bor:
				return 3;
			case op::xor :
				return 4;
			case op::band:
				return 5;
			case op::eq:case op::neq:
				return 6;
			case op::lt:case op::le:case op::gt:case op::ge:
				return 7;
			case op::bls:case op::brs:
				return 8;
			case op::plus:case op::minus:
				return 9;
			case op::times:case op::divide:case op::modulo:
				return 10;
			case op::power:
				return 11;
			case op::unary_minus:case op::bnot:case op::lnot:
				return 12;
			}
		}

		using variable=std::string;

		class number {
		public:
			enum {
				int_tag,
				float_tag
			};
		private:
			int _disc;
			union {
				int i;
				float f;
			}_u;
		public:
			number(int n) noexcept:_disc{int_tag}
			{
				_u.i=n;
			}
			number(float n) noexcept:_disc{float_tag}
			{
				_u.f=n;
			}

			number(number const& n) noexcept:_disc{n._disc},_u{n._u}
			{}

			number& operator=(int n) noexcept
			{
				_disc=int_tag;
				_u.i=n;
				return *this;
			}
			number& operator=(float n) noexcept
			{
				_disc=float_tag;
				_u.f=n;
				return *this;
			}
			number& operator=(number const& n) noexcept
			{
				_disc=n._disc;
				_u=n._u;
				return *this;
			}

			int type() const noexcept
			{
				return _disc;
			}

			bool is_int() const noexcept
			{
				return !_disc;
			}

			bool is_float() const noexcept
			{
				return _disc;
			}

			decltype(_u) raw_data() const noexcept
			{
				return _u;
			}

			explicit operator bool() const noexcept
			{
				if(_disc)
				{
					return bool(_u.f);
				}
				return bool(_u.i);
			}

			operator int() const noexcept
			{
				if(_disc)
				{
					return int(_u.f);
				}
				return _u.i;
			}
			operator float() const noexcept
			{
				if(_disc)
				{
					return _u.f;
				}
				return float(_u.i);
			}

#define COMP_OP(op)\
			bool operator op(number other) const noexcept\
			{\
				if(_disc==int_tag)\
				{\
					if(other._disc==int_tag) return _u.i op other._u.i;\
					return _u.i op other._u.f;\
				}\
				if(other._disc==int_tag) return _u.f op other._u.i;\
				return _u.f op other._u.f;\
			}
			COMP_OP(<)
				COMP_OP(>)
				COMP_OP(<=)
				COMP_OP(>=)
				COMP_OP(==)
				COMP_OP(!=)
#undef COMP_OP
#define ARITH_OP(op)\
			friend number operator op(number a,number b) noexcept\
			{\
				if(a._disc||b._disc)\
				{\
					return float(a) op float(b);\
				}\
				return int(a) op int(b);\
			}
				ARITH_OP(+)
				ARITH_OP(-)
				ARITH_OP(/)
				ARITH_OP(*)
#undef ARITH_OP

				friend number operator %(number a,number b) noexcept
			{
				if(a._disc||b._disc)
				{
					return fmodf(a,b);
				}
				return int(a)%int(b);
			}

			number operator-() const noexcept
			{
				if(is_int())
				{
					return -int(*this);
				}
				return -float(*this);
			}
		};

		template<typename N>
		constexpr N ipow(N n,int b)
		{
			if(b==0)
			{
				return 1;
			}
			if(b==1)
			{
				return n;
			}
			N acc=1;
			do
			{
				if(b&1)
				{
					acc*=n;
					--b;
				}
				else
				{
					n*=n;
					b>>=1;
				}
			} while(b>=2);
			return n*acc;
		}

		number pow(number a,number b) noexcept
		{
			if(a.is_float())
			{
				if(b.is_int())
				{
					if(int(b)<0)
					{
						return 1.0f/ipow(float(a),b);
					}
					return ipow(float(a),b);
				}
				return fmodf(a,b);
			}
			if(b.is_int())
			{
				return ipow(int(a),b);
			}
			return powf(a,b);
		}

		template<typename Visitor>
		decltype(auto) visit(Visitor&& vis,number n)
		{
			if(n._disc)
			{
				return vis(n.raw_data().i);
			}
			return vis(n.raw_data().f);
		}

		enum class func_code {
			invalid,
			max,
			min,
			int_cast,
			float_cast
		};

		struct function {
			std::size_t arity;
			func_code op_code;
		};

		struct _start_paren {};
		struct _end_paren {};
		struct _comma {};
		struct _empty {};

		using reverse_polish_item=std::variant<variable,op,function,int,float,_start_paren,_end_paren,_comma,_empty>;

		bool is_real_operator(reverse_polish_item const& item)
		{
			return item.index()==1;
		}

		class token_parser {
		public:
		};

		std::vector<reverse_polish_item> c_mathexpr_parse(char const* begin,char const* end)
		{
			std::vector<reverse_polish_item> operator_stack;
			std::vector<reverse_polish_item> out;
			std::basic_string<unsigned char> comma_ability{0};
			std::basic_string<unsigned char> arg_count{0};
			bool after_token=false;
			auto parser=[&,end](char const* begin) -> std::pair<char const*,reverse_polish_item>
			{
				while(true)
				{
					if(std::isspace(*begin))
					{
						++begin;
						if(begin==end)
						{
							return {end,_empty{}};
						}
					}
					else
					{
						break;
					}
				}
				auto is_digit=[](auto c)
				{
					return c>='0'&&c<='9';
				};
				auto is_letter=[](auto c)
				{
					return (c>='a'&&c<='z')||(c>='A'&&c<='Z');
				};
				auto is_after_token_op=[](auto c)
				{
					switch(c)
					{
					case '?':
						return op::_ternary_q;
					case ':':
						return op::_ternary_colon;
					case '|':
						return op::bor;
					case '&':
						return op::land;
					case '^':
						return op::xor;
					case '=':
						return op::eq;
					case '!':
						return op::neq;
					case '>':
						return op::gt;
					case '<':
						return op::lt;
					case '+':
						return op::plus;
					case '-':
						return op::minus;
					case '*':
						return op::times;
					case '/':
						return op::divide;
					case '%':
						return op::modulo;
					}
					throw std::invalid_argument{"Unknown op"};
				};
				auto is_after_token_second_op=[](op first,auto c)
				{
					switch(first)
					{
					case op::bor:
						return c=='|'?op::lor:op::bor;
					case op::band:
						return c=='&'?op::land:op::band;
					case op::eq:
					case op::neq:
						return c=='='?first:throw std::invalid_argument{"Unknown op"};;
					case op::gt:
					case op::lt:
						return c=='='?static_cast<op>(int(first)+1):first;
					case op::times:
						return c=='*'?op::power:op::times;
					}
					return first;
				};
				auto is_unary_op=[](auto c)
				{
					switch(c)
					{
					case '!':
						return op::lnot;
					case '~':
						return op::bnot;
					}
					return op::invalid;
				};
				auto is_func=[](std::string_view str)
				{
					using namespace std::literals;
					if(str=="max"sv)
					{
						return func_code::max;
					}
					if(str=="min"sv)
					{
						return func_code::min;
					}
					if(str=="int"sv)
					{
						return func_code::int_cast;
					}
					if(str=="float"sv)
					{
						return func_code::float_cast;
					}
					throw std::invalid_argument{"Unknown function"};
				};
				bool negative=*begin=='-';
				{
					if(after_token)
					{
						if(*begin==')')
						{
							return {++begin,_end_paren{}};
						}
						after_token=false;
						if(*begin==',')
						{
							if(comma_ability.back())
							{
								return {++begin,_comma{}};
							}
							else
							{
								throw std::invalid_argument{"Invalid comma"};
							}
						}
						auto const first_token=is_after_token_op(*begin);
						++begin;
						if(begin==end)
						{
							throw std::invalid_argument{"Trailing op"};
						}
						auto const second_token=is_after_token_second_op(first_token,*begin);
						if(second_token!=first_token)
						{
							++begin;
						}
						return {begin,second_token};
					}
					else
					{
						if(is_digit(*begin))
						{
							after_token=true;
							int integer=*begin-'0';
							++begin;
							while(true)
							{
								if(begin==end)
								{
									if(negative) integer=-integer;
									return {begin,integer};
								}
								if(is_digit(*begin))
								{
									integer*=10;
									integer+=*begin-'0';
								}
								else if(*begin=='.')
								{
									break;
								}
								else
								{
									if(negative) integer=-integer;
									return {begin,integer};
								}
								++begin;
							}
							++begin;
							float integral_part=integer;
							float decimal_part=0;
							float places=0;
							while(true)
							{
								if(begin==end||!is_digit(*begin))
								{
									auto ret=integral_part+decimal_part/std::pow(10.0f,places);
									if(negative) ret=-ret;
									return {begin,ret};
								}
								decimal_part*=10;
								decimal_part+=*begin-'0';
								places+=1;
								++begin;
							}
						}
						if(*begin=='(')
						{
							return {++begin,_start_paren{}};
						}
						if(*begin==')')
						{
							return {++begin,_end_paren{}};
						}
						auto const uop=is_unary_op(*begin);
						if(uop!=op::invalid)
						{
							return {++begin,uop};
						}
						else
						{
							std::string token;
							token.push_back(*begin);
							++begin;
							while(true)
							{
								if(begin==end)
								{
									after_token=true;
									return {begin,std::move(token)};
								}
								auto const c=*begin;
								if(c=='(')
								{
									return {++begin,function{size_t(-1),is_func(token)}};
								}
								if(!is_digit(c)&&!is_letter(c))
								{
									after_token=true;
									return {begin,std::move(token)};
								}
								token.push_back(c);
								++begin;
							}
						}
					}
				}
			};
			while(begin!=end)
			{
				auto [next,token]=parser(begin);

				std::visit([&](auto const& elem)
					{
						using T=std::decay_t<decltype(elem)>;
						if constexpr(std::is_same_v<T,int>||std::is_same_v<T,float>||std::is_same_v<T,variable>)
						{
							if(arg_count.back()<1) arg_count.back()=1;
							out.push_back(std::move(token));
						}
						else if constexpr(std::is_same_v<T,function>)
						{
							comma_ability.push_back(true);
							arg_count.push_back(0);
							operator_stack.push_back(std::move(token));
						}
						else if constexpr(std::is_same_v<T,op>)
						{
							if(elem==op::_ternary_colon)
							{
								while(!operator_stack.empty())
								{
									auto& oper=operator_stack.back();
									if(std::visit([&](auto const& item) -> bool
										{
											if constexpr(std::is_same_v<std::decay_t<decltype(item)>,op>)
											{
												if(item==op::_ternary_q)
												{
													return true;
												}
												return false;
											}
											throw std::invalid_argument{"Invalid ternary colon"};
										},oper))
									{
										out.push_back(std::move(oper));
										break;
									}
								}
								out.push_back(op::ternary);
								return;
							}
							while(!operator_stack.empty())
							{
								auto& oper=operator_stack.back();
								if(std::visit([&](auto const& item)
									{
										if constexpr(std::is_same_v<std::decay_t<decltype(item)>,op>)
										{
											if(precedence(item)>=precedence(elem))
											{
												return false;
											}
										}
										return true;
									},oper))
								{
									break;
								}
								out.push_back(std::move(oper));
								operator_stack.pop_back();
							}
							operator_stack.push_back(std::move(token));
						}
						else if constexpr(std::is_same_v<T,_start_paren>)
						{
							comma_ability.push_back(false);
							arg_count.push_back(0);
							operator_stack.push_back(std::move(token));
						}
						else if constexpr(std::is_same_v<T,_end_paren>)
						{
							after_token=true;
							while(true)
							{
								if(operator_stack.empty())
								{
									throw std::invalid_argument{"Extra end parentheses"};
								}
								auto oper=std::move(operator_stack.back());
								operator_stack.pop_back();
								auto const index=oper.index();
								if(index!=1)
								{
									auto ac=arg_count.back();
									if(std::visit([ac](auto& item)
										{
											if constexpr(std::is_same_v<std::decay_t<decltype(item)>,function>)
											{
												function& f=item;
												switch(f.op_code)
												{
												case func_code::max:
												case func_code::min:
													if(ac<2)
													{
														throw std::invalid_argument{"Too few arguments to min/max"};
													}
													break;
												case func_code::int_cast:
												case func_code::float_cast:
													if(ac>1)
													{
														throw std::invalid_argument{"Too many arguments to cast"};
													}
												}
												f.arity=ac;
												return true;
											}
											return false;
										},oper))
									{
										out.push_back(std::move(oper));
									}
									arg_count.pop_back();
									break;
								}
								out.push_back(std::move(oper));
							}
							comma_ability.pop_back();
						}
						else if constexpr(std::is_same_v<T,_comma>)
						{
							after_token=false;
							while(!operator_stack.empty())
							{
								auto& oper=operator_stack.back();
								auto const index=oper.index();
								if(index!=1)
								{
									break;
								}
								out.push_back(std::move(oper));
								operator_stack.pop_back();
							}
							++arg_count.back();
						}
						else if constexpr(std::is_same_v<T,_empty>)
						{}
						else
						{
							static_assert(!std::is_same_v<T,T>,"Missing types");
						}
					},token);
				begin=next;
			}
			while(!operator_stack.empty())
			{
				auto& oper=operator_stack.back();
				if(oper.index()!=1)
				{
					throw std::invalid_argument{"Unclosed parentheses"};
				}
				out.push_back(std::move(oper));
				operator_stack.pop_back();
			}
			return out;
		}

		template<typename Map>
		number evaluate(std::vector<reverse_polish_item> const& rp,Map const& environment)
		{
			std::vector<number> stack;
			for(auto const& item:rp)
			{
				std::visit([&](auto const& item)
					{
						using T=std::decay_t<decltype(item)>;
						if constexpr(std::is_same_v<T,variable>)
						{
							if(auto it=environment.find(item);it!=environment.end())
							{
								stack.push_back(it->second);
							}
							else
							{
								throw std::invalid_argument{"Unbound variable"};
							}
						}
						else if constexpr(std::is_same_v<T,op>)
						{
							auto const s=stack.size();
							switch(item)
							{
							case op::ternary:
							{
								auto& a=stack[s-3];
								auto& b=stack[s-2];
								auto& c=stack[s-1];
								if(a)
								{
									a=b;
								}
								else
								{
									a=c;
								}
								stack.erase(stack.end()-2,stack.end());
								break;
							}
							case op::lor:
							{
								auto& a=stack[s-2];auto& b=stack[s-1];
								a=bool(a)||bool(b);
								stack.pop_back();
								break;
							}
							case op::land:
							{
								auto& a=stack[s-2];auto& b=stack[s-1];
								a=bool(a)&&bool(b);
								stack.pop_back();
								break;
							}
							case op::bor:
							{
								auto& a=stack[s-2];auto& b=stack[s-1];
								a=int(a)|int(b);
								stack.pop_back();
								break;
							}
							case op::xor :
							{
								auto& a=stack[s-2];auto& b=stack[s-1];
								a=int(a)^int(b);
								stack.pop_back();
								break;
							}
							case op::band:
							{
								auto& a=stack[s-2];auto& b=stack[s-1];
								a=int(a)&int(b);
								stack.pop_back();
								break;
							}
							case op::eq:
							{
								auto& a=stack[s-2];auto& b=stack[s-1];
								a=(a==b);
								stack.pop_back();
								break;
							}
							case op::neq:
							{
								auto& a=stack[s-2];auto& b=stack[s-1];
								a=(a!=b);
								stack.pop_back();
								break;
							}
							case op::lt:
							{
								auto& a=stack[s-2];auto& b=stack[s-1];
								a=(a<b);
								stack.pop_back();
								break;
							}
							case op::le:
							{
								auto& a=stack[s-2];auto& b=stack[s-1];
								a=(a<=b);
								stack.pop_back();
								break;
							}
							case op::gt:
							{
								auto& a=stack[s-2];auto& b=stack[s-1];
								a=(a>=b);
								stack.pop_back();
								break;
							}
							case op::ge:
							{
								auto& a=stack[s-2];auto& b=stack[s-1];
								a=(a>=b);
								stack.pop_back();
								break;
							}
							case op::bls:
							{
								auto& a=stack[s-2];auto& b=stack[s-1];
								a=(int(a)<<int(b));
								stack.pop_back();
								break;
							}
							case op::brs:
							{
								auto& a=stack[s-2];auto& b=stack[s-1];
								a=(int(a)>>int(b));
								stack.pop_back();
								break;
							}
							case op::plus:
							{
								auto& a=stack[s-2];auto& b=stack[s-1];
								a=a+b;
								stack.pop_back();
								break;
							}
							case op::minus:
							{
								auto& a=stack[s-2];auto& b=stack[s-1];
								a=a-b;
								stack.pop_back();
								break;
							}
							case op::times:
							{
								auto& a=stack[s-2];auto& b=stack[s-1];
								a=a*b;
								stack.pop_back();
								break;
							}
							case op::divide:
							{
								auto& a=stack[s-2];auto& b=stack[s-1];
								a=a/b;
								stack.pop_back();
								break;
							}
							case op::modulo:
							{
								auto& a=stack[s-2];auto& b=stack[s-1];
								a=a%b;
								stack.pop_back();
								break;
							}
							case op::power:
							{
								auto& a=stack[s-2];auto& b=stack[s-1];
								a=pow(a,b);
								stack.pop_back();
								break;
							}
							case op::unary_minus:
							{
								stack.back()=-stack.back();
								break;
							}
							case op::bnot:
							{
								stack.back()=~int(stack.back());
								break;
							}
							case op::lnot:
							{
								stack.back()=!bool(stack.back());
								break;
							}
							}
						}
						else if constexpr(std::is_same_v<T,function>)
						{
							function const& f=item;
							switch(f.op_code)
							{
							case func_code::max:
							{
								auto const begin=stack.end()-f.arity;
								auto mx=std::max_element(begin,stack.end());
								*begin=*mx;
								stack.erase(begin+1,stack.end());
									break;
							}
							case func_code::min:
							{
								auto const begin=stack.end()-f.arity;
								auto mx=std::min_element(begin,stack.end());
								*begin=*mx;
								stack.erase(begin+1,stack.end());
								break;
							}
							case func_code::int_cast:
							{
								if(f.arity==0)
								{
									stack.push_back(0.0f);
								}
								else
								{
									stack.back()=float(stack.back());
								}
								break;
							}
							case func_code::float_cast:
							{
								if(f.arity==0)
								{
									stack.push_back(0);
								}
								else
								{
									stack.back()=int(stack.back());
								}
								break;
							}
							}
						}
						else if constexpr(std::is_same_v<T,int>||std::is_same_v<T,float>)
						{
							stack.push_back(item);
						}
					},item);
			}
			return stack[0];
		}
	}

	template<typename InputIter,typename OutputIter,typename TokenParser>
	void shunting_yard_parse(InputIter begin,InputIter end,OutputIter out,TokenParser parser)
	{
		while(begin!=end)
		{
			auto end_token=parser(begin,end);
		}
	}

}
#endif