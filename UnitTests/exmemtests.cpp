#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Utils/exmem.h"
#include <memory>
#include <string>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ExMemTests {
	TEST_CLASS(Iterators)
	{
		TEST_METHOD(uninitialized)
		{
			auto deleter=[](auto ptr)
			{
				free(ptr);
			};
			constexpr size_t n=50;
			std::unique_ptr<std::string[],decltype(deleter)> mem(static_cast<std::string*>(malloc(50*sizeof(std::string))),deleter);
			exlib::uninitialized_iterator<std::string> begin(mem.get());
			exlib::iterator<std::string> b(mem.get());
		}
	};
}