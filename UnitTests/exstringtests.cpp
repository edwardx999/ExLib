#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Utils/exstring.h"
#include <vector>
#include <string>
#include <algorithm>
using namespace exlib;
using namespace std;
namespace Microsoft {
	namespace VisualStudio {
		namespace CppUnitTestFramework {
			static std::wstring ToString(const vector<char*>& obj)
			{
				std::wstring ret;
				for(auto str:obj)
				{
					while(*str)
					{
						ret.push_back(*str);
						str++;
					}
					ret.push_back(' ');
				}
				return ret;
			}
			static std::wstring ToString(const vector<std::string>& obj)
			{
				std::wstring ret;
				for(auto const& str:obj)
				{
					auto s=str.data();
					while(*s)
					{
						ret.push_back(*s);
						s++;
					}
					ret.push_back(' ');
				}
				return ret;
			}
		}
	}
}
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests {
	TEST_CLASS(UnitTest1)
	{
	public:

		TEST_METHOD(StrnCmpNum0)
		{
			auto a="1";
			auto b="1";
			auto res=strncmp_num(a,a+strlen(a),b,b+strlen(b));
			Assert::IsTrue(res==0);
		}
		TEST_METHOD(StrnCmpNum1)
		{
			auto a="01";
			auto b="1";
			auto res=strncmp_num(a,a+strlen(a),b,b+strlen(b));
			Assert::IsTrue(res<0);
		}
		TEST_METHOD(StrnCmpNum2)
		{
			auto a="01";
			auto b="1";
			auto res=strncmp_num(a,a+strlen(a),b,b+strlen(b));
			Assert::IsTrue(res<0);
		}
		TEST_METHOD(StrnCmpWind1)
		{
			auto a="01.html";
			auto b="1.html";
			auto res=strncmp_wind(a,b);
			auto res2=strncmp_wind(b,a);
			Assert::IsTrue(res<0);
			Assert::IsTrue(res2>0);
		}
		TEST_METHOD(StrnCmpWind2)
		{
			auto a="9.html";
			auto b="10.html";
			auto res=strncmp_wind(a,b);
			auto res2=strncmp_wind(b,a);
			Assert::IsTrue(res<0);
			Assert::IsTrue(res2>0);
		}
		TEST_METHOD(StrnCmpWind3)
		{
			auto a="9";
			auto b="10";
			auto res=strncmp_wind(a,b);
			auto res2=strncmp_wind(b,a);
			Assert::IsTrue(res<0);
			Assert::IsTrue(res2>0);
		}
		TEST_METHOD(StrnCmpWind4)
		{
			auto a="010";
			auto b="10";
			auto res=strncmp_wind(a,b);
			auto res2=strncmp_wind(b,a);
			Assert::IsTrue(res<0);
			Assert::IsTrue(res2>0);
		}
		TEST_METHOD(StrnCmpWind5)
		{
			auto a="7";
			auto b="10";
			auto res=strncmp_wind(a,b);
			auto res2=strncmp_wind(b,a);
			Assert::IsTrue(res<0);
			Assert::IsTrue(res2>0);
		}
		TEST_METHOD(StrnCmpWind6)
		{
			auto a="7";
			auto b="106";
			auto res=strncmp_wind(a,b);
			auto res2=strncmp_wind(b,a);
			Assert::IsTrue(res<0);
			Assert::IsTrue(res2>0);
		}
		TEST_METHOD(StrnCmpWindList)
		{
			vector<char*> ret={"1","02","2","3","4","5","6","7","8","9","010","10","11","103","0104"};
			auto exp=ret;
			random_shuffle(ret.begin(),ret.end());
			sort(ret.begin(),ret.end(),[](auto a,auto b)
			{
				return strncmp_wind(a,b)<0;
			});
			Assert::AreEqual(exp,ret);
		}
		TEST_METHOD(StrnCmpWindList2)
		{
			vector<char*> ret=
			{"My Show - Episode 1 - Blash.mp4",
				"My Show - Episode 2 - SAD.mp4",
				"My Show - Episode 3 - Des.mp4",
				"My Show - Episode 4 - Kac.mp4",
				"My Show - Episode 5 - Wasdwa.mp4",
				"My Show - Episode 6 - Borea.mp4",
				"My Show - Episode 7 - Cardian.mp4",
				"My Show - Episode 8 - Mask.mp4",
				"My Show - Episode 9 - Bit.mp4",
				"My Show - Episode 11 - Ti.mp4",
				"My Show - Episode 12 - Tra.mp4",
				"My Show - Episode 13 - Car.mp4",
				"My Show - Episode 14 - Make.mp4",};
			auto exp=ret;
			random_shuffle(ret.begin(),ret.end());
			sort(ret.begin(),ret.end(),[](auto const& a,auto const& b)
			{
				return strncmp_wind(a,b)<0;
			});
			Assert::AreEqual(exp,ret);
		}
		TEST_METHOD(StrnCmpWind7)
		{
			auto a="My Show - Episode 1 - Blash.mp4";
			auto b="My Show - Episode 2 - Cram.mp4";
			auto res=strncmp_wind(a,b);
			auto res2=strncmp_wind(b,a);
			Assert::IsTrue(res<0);
			Assert::IsTrue(res2>0);
		}
	};
}