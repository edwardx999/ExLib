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
	TEST_CLASS(Fatten)
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
			std::vector<unsigned int> in={1,2,3,4,5,6,7 ,8 ,9 ,10,1 ,2 ,3 ,4,5,6 ,8  ,0  ,10 ,112};
			std::vector<unsigned int> ex={4,5,6,7,8,9,10,10,10,10,10,10,10,8,8,10,112,112,112,112};
			auto res=exlib::fattened_profile(in,3,[](auto a,auto b)
			{
				return a>b;
			});
			Assert::AreEqual(ex,res);
		}
	};

	TEST_CLASS(MaybeFixed)
	{
		TEST_METHOD(mfvar)
		{
			exlib::maybe_fixed<> mf(0.5f,1);
			Assert::AreEqual(10,mf(10,20));
			mf.index(0);
			Assert::AreEqual(5,mf(10,20));
		}
		TEST_METHOD(mffix)
		{
			exlib::maybe_fixed<> mf(100);
			Assert::AreEqual(100,mf(10,20));
			mf.fix_from(10,20);
			Assert::AreEqual(100,mf(10,20));
			mf.fix(19);
			Assert::AreEqual(19,mf(10,20));
		}
		TEST_METHOD(mffix2)
		{
			exlib::maybe_fixed<> mf(100);
			Assert::AreEqual(100,mf(10,20));
			mf.variant(10,2);
			mf.fix_from(10,20,30);
			Assert::AreEqual(300,mf(1110,20));
			mf.fix(19);
			Assert::AreEqual(19,mf(10,20));
		}
	};
}