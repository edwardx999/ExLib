#ifndef EXLIB_MATRIX_H
#define EXLIB_MATRIX_H
#include <initializer_list>
#include <memory>
#include <amp.h>
namespace exlib {

	//ALL OPERATIONS ARE UNCHECKED
	template<typename T=float>
	class Matrix {
		size_t _rows;
		size_t _cols;
		std::unique_ptr<T[]> _data;
		T* _end;
	public:
		typedef T value_type;
		typedef T& reference;
		typedef T const& const_reference;

		class iterator {
			T* data;
		public:
			typedef std::random_access_iterator_tag iterator_category;
			typedef T value_type;
			typedef T* pointer;
			typedef T const* const_pointer;
			typedef T& reference;
			typedef T const& const_reference;
			typedef std::ptrdiff_t difference_type;

			iterator(T* data):data(data)
			{}
			iterator operator++(int)
			{
				iterator copy(data++);
				return copy;
			}
			iterator& operator++()
			{
				++data;
				return *this;
			}
			iterator operator--(int)
			{
				iterator copy(data--);
				return copy;
			}
			iterator& operator--()
			{
				--data;
				return *this;
			}
			reference operator*() const
			{
				return *data;
			}
			reference operator[](size_t n) const
			{
				return *(data+n);
			}

		#define comp(op)\
			bool operator##op##(iterator other) const\
			{\
				return data##op##other.data;\
			}
			comp(==)
				comp(>)
				comp(<)
				comp(>=)
				comp(<=)
				comp(!=)
			#undef comp
		};
		class const_iterator {
			T const* data;
		public:
			typedef std::random_access_iterator_tag iterator_category;
			typedef T value_type;
			typedef T* pointer;
			typedef T const* const_pointer;
			typedef T& reference;
			typedef T const& const_reference;
			typedef std::ptrdiff_t difference_type;

			const_iterator(iterator i):data(&*i)
			{}
			const_iterator(T const* data):data(data)
			{}
			const_iterator operator++(int)
			{
				iterator copy(data++);
				return copy;
			}
			const_iterator& operator++()
			{
				++data;
				return *this;
			}
			const_iterator operator--(int)
			{
				iterator copy(data--);
				return copy;
			}
			const_iterator& operator--()
			{
				--data;
				return *this;
			}
			const_reference operator*() const
			{
				return *data;
			}
			const_reference operator[](size_t n) const
			{
				return *(data+n);
			}

		#define comp(op)\
			bool operator##op##(const_iterator other) const\
			{\
				return data##op##other.data;\
			}
			comp(==)
				comp(>)
				comp(<)
				comp(>=)
				comp(<=)
				comp(!=)
			#undef comp
		};
		class row_iterator {
			T* _data;
			size_t _cols;
		public:
			row_iterator(T* data,size_t cols):_data(data),_cols(cols)
			{}
			row_iterator& operator++()
			{
				_data+=_cols;
				return *this;
			}
			row_iterator operator++(int)
			{
				row_iterator copy(_data,_cols);
				_data+=cols;
				return copy;
			}
			row_iterator& operator--()
			{
				_data-=_cols;
				return *this;
			}
			row_iterator operator--(int)
			{
				row_iterator copy(_data,_cols);
				_data-=_cols;
				return copy;
			}
			size_t size() const
			{
				return _cols;
			}
			iterator operator*()
			{
				return iterator(_data);
			}
			iterator operator[](size_t n)
			{
				return iterator(_data+n*_cols);
			}
			iterator begin()
			{
				return iterator(_data);
			}
			iterator end()
			{
				return iterator(_data+_cols);
			}
		#define comp(op)\
			bool operator##op##(row_iterator const& other) const\
			{\
				return _data##op##other._data;\
			}
			comp(==)
				comp(>)
				comp(<)
				comp(>=)
				comp(<=)
				comp(!=)
			#undef comp
		};
		class const_row_iterator {
			T const* _data;
			size_t _cols;
		public:
			const_row_iterator(T const* data,size_t cols):_data(data),_cols(cols)
			{}
			const_row_iterator& operator++()
			{
				_data+=_cols;
				return *this;
			}
			const_row_iterator operator++(int)
			{
				const_row_iterator copy(_data,_cols);
				_data+=cols;
				return copy;
			}
			const_row_iterator& operator--()
			{
				_data-=_cols;
				return *this;
			}
			const_row_iterator operator--(int)
			{
				const_row_iterator copy(_data,_cols);
				_data-=_cols;
				return copy;
			}
			size_t size() const
			{
				return _cols;
			}
			const_iterator operator*()
			{
				return const_iterator(_data);
			}
			const_iterator operator[](size_t n)
			{
				return const_iterator(_data+n*_cols);
			}
			const_iterator begin()
			{
				return const_iterator(_data);
			}
			const_iterator end()
			{
				return const_iterator(_data+_cols);
			}
		#define comp(op)\
			bool operator##op##(const_row_iterator const& other) const\
			{\
				return _data##op##other._data;\
			}
			comp(==)
				comp(>)
				comp(<)
				comp(>=)
				comp(<=)
				comp(!=)
			#undef comp
		};

		class vertical_iterator {
			T* _data;
			size_t _cols;
		public:
			vertical_iterator(T* data,size_t cols):_data(data),_cols(cols)
			{}
			vertical_iterator& operator++()
			{
				_data+=_cols;
				return *this;
			}
			vertical_iterator operator++(int)
			{
				vertical_iterator copy(_data,_cols);
				_data-=cols;
				return copy;
			}
			vertical_iterator& operator--()
			{
				_data-=_cols;
				return *this;
			}
			vertical_iterator operator--(int)
			{
				vertical_iterator copy(_data,_cols);
				_data-=_cols;
				return copy;
			}
			T& operator*()
			{
				return *_data;
			}
			T& operator[](size_t n)
			{
				return *(_data+n*_cols);
			}
		#define comp(op)\
			bool operator##op##(vertical_iterator const& other) const\
			{\
				return _data##op##_other.data;\
			}
			comp(==)
				comp(>)
				comp(<)
				comp(>=)
				comp(<=)
				comp(!=)
			#undef comp
		};
		class const_vertical_iterator {
			T const* _data;
			size_t _cols;
		public:
			const_vertical_iterator(T const* data,size_t cols):_data(data),_cols(cols)
			{}
			const_vertical_iterator& operator++()
			{
				_data+=_cols;
				return *this;
			}
			const_vertical_iterator operator++(int)
			{
				const_vertical_iterator copy(_data,_cols);
				_data-=cols;
				return copy;
			}
			const_vertical_iterator& operator--()
			{
				_data-=_cols;
				return *this;
			}
			const_vertical_iterator operator--(int)
			{
				const_vertical_iterator copy(_data,_cols);
				_data-=_cols;
				return copy;
			}
			T const& operator*()
			{
				return *_data;
			}
			T const& operator[](size_t n)
			{
				return *(_data+n*_cols);
			}
		#define comp(op)\
			bool operator##op##(const_vertical_iterator const& other) const\
			{\
				return _data##op##_other.data;\
			}
			comp(==)
				comp(>)
				comp(<)
				comp(>=)
				comp(<=)
				comp(!=)
			#undef comp
		};

		class column_iterator {
			T* _data;
			size_t _cols;
			size_t _rows;
		public:
			column_iterator(T* data,size_t _rows,size_t cols):_data(data),_rows(rows),_cols(cols)
			{}
			column_iterator& operator++()
			{
				_data++;
				return *this;
			}
			column_iterator operator++(int)
			{
				column_iterator copy(_data,_rows);
				_data++;
				return copy;
			}
			column_iterator& operator--()
			{
				_data--;
				return *this;
			}
			column_iterator operator--(int)
			{
				column_iterator copy(_data,_rows);
				_data--;
				return copy;
			}
			vertical_iterator begin()
			{
				return vertical_iterator(_data,_cols);
			}
			vertical_iterator end()
			{
				return vertical_iterator(_data+_rows*_cols,_cols);
			}
		};
		class const_column_iterator {
			T const* _data;
			size_t _cols;
			size_t _rows;
		public:
			const_column_iterator(T const* data,size_t _rows,size_t cols):_data(data),_rows(rows),_cols(cols)
			{}
			const_column_iterator& operator++()
			{
				_data++;
				return *this;
			}
			const_column_iterator operator++(int)
			{
				const_column_iterator copy(_data,_rows);
				_data++;
				return copy;
			}
			const_column_iterator& operator--()
			{
				_data--;
				return *this;
			}
			const_column_iterator operator--(int)
			{
				const_column_iterator copy(_data,_rows);
				_data--;
				return copy;
			}
			const_vertical_iterator begin()
			{
				return const_vertical_iterator(_data,_cols);
			}
			const_vertical_iterator end()
			{
				return const_vertical_iterator(_data+_rows*_cols,_cols);
			}
		};

		reference operator()(size_t r,size_t c)
		{
			return _data[r*_cols+c];
		}
		const_reference operator()(size_t r,size_t c) const
		{
			return _data[r*_cols+c];
		}
		void assign(size_t rows,size_t cols)
		{
			_rows=rows;
			_cols=cols;
			auto size=rows*cols;
			_data=std::make_unique<T[]>(size);
			_end=_data.get()+size;
		}
		Matrix()
		{
			assign(0,0);
		}
		Matrix(size_t rows,size_t cols)
		{
			assign(rows,cols);
		}
		Matrix(std::initializer_list<std::initializer_list<T>> init)
		{
			size_t rows=init.size(),c=0;
			for(auto const& r:init)
			{
				auto width=r.size();
				if(width>c)
				{
					c=width;
				}
			}
			assign(rows,c);
			T* rit=_data.get();
			for(auto const& r:init)
			{
				T* it=rit;
				rit+=_cols;
				for(auto const& e:r)
				{
					*(it++)=e;
				}
				for(;it!=rit;++it)
				{
					*it=0;
				}
			}
		}
		Matrix(Matrix const& other)
		{
			_rows=other._rows;
			_cols=other._cols;
			auto const size=_rows*_cols;
			_data=std::make_unique<T[]>(size);
			_end=_data.get()+size;
			//T const* const od=other._data.get();
			memcpy(_data.get(),other._data.get(),size*sizeof(T));
		}
		Matrix(Matrix&&)=default;
		Matrix& operator=(Matrix const&)
		{
			_rows=other._rows;
			_cols=other._cols;
			auto const size=_rows*_cols;
			_data=std::make_unique<T[]>(size);
			_end=_data.get()+size;
			memcpy(_data.get(),other._data.get(),size*sizeof(T));
		}
		Matrix& operator=(Matrix&&)=default;
		~Matrix()=default;
		T* data()
		{
			return _data.get();
		}
		T const* data() const
		{
			return _data.get();
		}

		iterator begin()
		{
			return iterator(_data.get());
		}
		iterator end()
		{
			return iterator(_end);
		}
		const_iterator cbegin() const
		{
			return const_iterator(_data.get());
		}
		const_iterator cend() const
		{
			return const_iterator(_end);
		}
		row_iterator row_begin()
		{
			return row_iterator(_data.get(),_cols);
		}
		row_iterator row_end()
		{
			return row_iterator(_end,_cols);
		}
		const_row_iterator crow_begin() const
		{
			return const_row_iterator(_data.get(),_cols);
		}
		const_row_iterator crow_end() const
		{
			return const_row_iterator(_end,_cols);
		}
		column_iterator column_begin()
		{
			return column_iterator(_data.get(),_rows,_cols);
		}
		column_iterator column_end()
		{
			return column_iterator(_data.get()+_cols,_rows,_cols);
		}
		const_column_iterator ccolumn_begin() const
		{
			return const_column_iterator(_data.get(),_rows,_cols);
		}
		const_column_iterator ccolumn_end() const
		{
			return const_column_iterator(_data.get()+_cols,_rows,_cols);
		}

		size_t rows() const
		{
			return _rows;
		}
		void rerow(size_t rows)
		{
			size_t new_size=rows*_cols;
			auto m=std::make_unique<T[]>(new_size);
			auto mend=m.get()+new_size;
			size_t old_size=end-_data.get();
			auto end=m.get()+std::min(old_size,new_size);
			T* it=_data.get(),*mit=m.get();
			for(;mit!=end;++it,++mit)
			{
				*mit=*it;
			}
			for(;mit!=mend;++mit)
			{
				*mit=0;
			}
		}
		size_t cols() const
		{
			return _cols;
		}
		void recol(size_t cols);
		size_t size() const
		{
			return end-_data.get();
		}
		void resize(size_t rows,size_t cols);
		void fill_row(row_iterator row,T val);
		void fill_col(column_iterator col,T val);
		void fill(T val)
		{
			T* const limit=_end;
			for(auto it=_data.get();it!=limit;++it)
			{
				*it=val;
			}
		}
		Matrix operator*(Matrix const& other) const
		{
			Matrix ret(rows(),other.cols());
			ret.fill(0);
			size_t const M=rows();
			size_t const N=cols();
			size_t const P=other.cols();
			T const* const dt=this->data();
			T* const rt=ret.data();
			T const* const ot=other.data();
		#pragma loop(ivdep)
			for(size_t i=0;i<M;++i)
			{
				size_t const row_t=i*N;
				size_t const row_r=i*P;
				for(size_t k=0;k<N;++k)
				{
					size_t const row_o=k*P;
					T const r=dt[row_t+k];
					for(size_t j=0;j<P;++j)
					{
						size_t const idx=row_r+j;
						rt[idx]=rt[idx]+r*ot[row_o+j];
					}
				}
			}
			return ret;
		}
		Matrix accelerated_times(Matrix const& other) const
		{
			/*Matrix ret(rows(),other.cols());
			ret.fill(0);
			using namespace concurrency;
			array_view<T,2> out(ret.rows(),ret.cols(),ret.data());
			array_view<T const,2> a(rows(),cols(),data());
			array_view<T const,2> b(other.rows(),other.cols(),other.data());
			parallel_for_each(out.extent,
				[=,cols=static_cast<unsigned int>(ret.cols())](index<2> idx) restrict(amp)
			{
				unsigned int i=idx[0];
				unsigned int k=idx[1];
				T const r=a(i,k);
				for(unsigned int j=0;j<cols;++j)
				{
					out(i,j)+=r*b(k,j);
				}
			});
			out.synchronize();
			return ret;*/
			Matrix ret(rows(),other.cols());
			ret.fill(0);
			int const M=rows();
			int const N=cols();
			int const P=other.cols();
			T const* const dt=this->data();
			T* const rt=ret.data();
			T const* const ot=other.data();
		#pragma loop(hint_parallel(0))
		#pragma loop(ivdep)
			for(int i=0;i<M;++i)
			{
				int const row_t=i*N;
				int const row_r=i*P;
				for(int k=0;k<N;++k)
				{
					int const row_o=k*P;
					T const r=dt[row_t+k];
					for(int j=0;j<P;++j)
					{
						int const idx=row_r+j;
						rt[idx]=rt[idx]+r*ot[row_o+j];
					}
				}
			}
			return ret;
		}
		Matrix get_transpose() const
		{
			Matrix ret(cols(),rows());
			T* col=ret.data();
			size_t const c=ret.cols();
			auto const limit=crow_end();
			for(auto row=crow_begin();row<limit;++row)
			{
				T* re=col;
				auto const limit=row.end();
				for(auto it=row.begin();it<limit;++it,re+=c)
				{
					*re=*it;
				}
				++col;
			}
			return ret;
		}
		Matrix& transpose()
		{
			return (*this=get_transpose());
		}

		Matrix& operator*=(T val)
		{
			auto const limit=_end;
			for(auto it=_data.get();it<limit;++it)
			{
				*it=*it*val;
			}
			return *this;
		}
		Matrix operator*(T val) const
		{
			Matrix ret(*this);
			return ret*=val;
		}
	};
}
#endif