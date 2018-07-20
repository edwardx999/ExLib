#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Utils/exmath.h"
#include <vector>
#include <string>
#include <algorithm>
using namespace exlib;
using namespace std;
namespace Microsoft {
	namespace VisualStudio {
		namespace CppUnitTestFramework {
			static std::wstring ToString(const vector<unsigned int>& obj)
			{
				std::wstring ret;
				for(auto val:obj)
				{
					ret.append(std::to_wstring(val));
					ret.append(L" ");
				}
				return ret;
			}
			static std::wstring ToString(uint64_t in)
			{
				return std::to_wstring(in);
			}
			static std::wstring ToString(int64_t in)
			{
				return std::to_wstring(in);
			}
		}
	}
}

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ExMathTests {
	TEST_CLASS(UnitTest1)
	{
		TEST_METHOD(fatten1)
		{
			std::vector<unsigned int> in={1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,8,0,10,112};
			std::vector<unsigned int> ex={1,1,1,2,3,4,5,6,1, 1,1,1,1,2,3,0,0,0,0,0};
			auto res=exlib::fattened_profile(in,2,[](auto a,auto b)
			{
				return a<b;
			});
			Assert::AreEqual(ex,res);
		}
		TEST_METHOD(fatten1a)
		{
			std::vector<unsigned int> in={1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,8,0,10,112};
			std::vector<unsigned int> ex={1,1,1,2,3,4,5,6,1, 1,1,1,1,2,3,0,0,0,0,0};
			auto res=exlib::fattened_profile(in,2);
			Assert::AreEqual(ex,res);
		}
		TEST_METHOD(fatten2)
		{
			std::vector<unsigned int> in={1,2,3,4,5,6,7,8 ,9 ,10,1 ,2 ,3,4,5,6,8 ,0  ,10 ,112};
			std::vector<unsigned int> ex={3,4,5,6,7,8,9,10,10,10,10,10,5,6,8,8,10,112,112,112};
			auto res=exlib::fattened_profile(in,2,[](auto a,auto b)
			{
				return a>b;
			});
			Assert::AreEqual(ex,res);
		}
		TEST_METHOD(fatten3)
		{
			std::vector<unsigned int> in={1,2,3,4,5,6,7 ,8 ,9 ,10,1 ,2 ,3 ,4,5,6,8  ,0  ,10 ,112};
			std::vector<unsigned int> ex={4,5,6,7,8,9,10,10,10,10,10,10,10,8,8,8,112,112,112,112};
			auto res=exlib::fattened_profile(in,3,[](auto a,auto b)
			{
				return a>b;
			});
			Assert::AreEqual(ex,res);
		}
	};
}