#include "global_thread_pool.h"

namespace exlib {
	namespace global_thread_pool {
		namespace detail {
			thread_pool& get_pool() {
				static thread_pool pool;
				return pool;
			}
		}
	}
}