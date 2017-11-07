/*
Copyright(C) 2017 Edward Xie

This program is free software:you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation,either version 3 of the License,or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not,see <https://www.gnu.org/licenses/>.
*/
#include "stdafx.h"
#include "macros.h"
#include <Psapi.h>
namespace macro_commands {
#define pkc_init(code) input.type=INPUT_KEYBOARD;input.ki={0,static_cast<WORD>(MapVirtualKey(code,MAPVK_VK_TO_VSC)),KEYEVENTF_SCANCODE,0,0};
#define rkc_init(code) input.type=INPUT_KEYBOARD;input.ki={0,static_cast<WORD>(MapVirtualKey(code,MAPVK_VK_TO_VSC)),KEYEVENTF_SCANCODE|KEYEVENTF_KEYUP,0,0};
#define tkc_init(code)\
		inputs[0].type=INPUT_KEYBOARD;\
		inputs[1].type=INPUT_KEYBOARD;\
		inputs[0].ki={0,static_cast<WORD>(MapVirtualKey(code,MAPVK_VK_TO_VSC)),KEYEVENTF_SCANCODE,0,0};\
		inputs[1].ki={0,inputs[0].ki.wScan,KEYEVENTF_SCANCODE|KEYEVENTF_KEYUP,0,0};
#define tmc_init(buttom)\
		inputs[0].type={INPUT_MOUSE};\
		inputs[1].type={INPUT_MOUSE};\
		inputs[0].mi={0,0,0,downmouse_codes[button],0,0};\
		inputs[1].mi={0,0,0,upmouse_codes[button],0,0};
#define pmc_init(button)\
		input={INPUT_MOUSE};\
		input.mi={0,0,0,downmouse_codes[button],0,0};
#define rmc_init(button)\
		input={INPUT_MOUSE};\
		input.mi={0,0,0,upmouse_codes[button],0,0};
#define mmc_init(x,y)\
		input.type=INPUT_MOUSE;\
		input.mi={static_cast<LONG>(x*screen_conv.x),static_cast<LONG>(y*screen_conv.y),0,MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE,0,0};
#define trmc_init(x,y)\
		input.type=INPUT_MOUSE;\
		input.mi={x,y,0,MOUSEEVENTF_MOVE,0,0};
	DWORD upmouse_codes[3]={MOUSEEVENTF_LEFTUP,MOUSEEVENTF_RIGHTUP,MOUSEEVENTF_MIDDLEUP};
	DWORD downmouse_codes[3]={MOUSEEVENTF_LEFTDOWN,MOUSEEVENTF_RIGHTDOWN,MOUSEEVENTF_MIDDLEDOWN};

	constexpr VK_CODE char_to_vk(char const c) noexcept {
		if('a'<=c&&c<='z')
		{
			return c-0x20;//VK_A is 0x41
		}
		if('0'<=c&&c<='9')
		{
			return c;
		}
		switch(c)
		{
			case '`': return VK_OEM_3;
			case '-': return VK_OEM_MINUS;
			case '=': return VK_OEM_PLUS;
			case '\t': return VK_TAB;
			case '[': return VK_OEM_4;
			case ']': return VK_OEM_6;
			case '\\': return VK_OEM_5;
			case ';': return VK_OEM_1;
			case '\'': return VK_OEM_7;
			case ',': return VK_OEM_COMMA;
			case '.': return VK_OEM_PERIOD;
			case '/': return VK_OEM_2;
			case '\b': return VK_BACK;
			case '\r': return VK_RETURN;
			case ' ': return VK_SPACE;
		}
		return 0;
	}

	constexpr char unshift(char const c) noexcept {
		if('A'<=c&&c<='Z')
		{
			return c+0x20;
		}
		switch(c)
		{
			case '!': return '1';
			case '@': return '2';
			case '#': return '3';
			case '$': return '4';
			case '%': return '5';
			case '^': return '6';
			case '&': return '7';
			case '*': return '8';
			case '(': return '9';
			case ')': return '0';
			case '_': return '-';
			case '+': return '=';
			case '~': return '`';
			case '{': return '[';
			case '}': return ']';
			case '|': return '\\';
			case ':': return ';';
			case '\"': return '\'';
			case '<': return ',';
			case '>': return '.';
			case '?': return '/';
		}
		return c;
	}

	constexpr bool is_key(char const c) noexcept {
		if(('a'<=c&&c<='z')||
			('0'<=c&&c<='9'))
		{
			return true;
		}
		switch(c)
		{
			case '`':
			case '[':
			case ']':
			case '\\':
			case ';':
			case '\'':
			case ',':
			case '.':
			case '/':
			case '\t':
			case '\b':
			case '\r':
			case ' ': return true;
		}
		return false;
	}

	bool is_window_open(wchar_t* const name) noexcept {
		return FindWindow(name,NULL)!=NULL;
	}

#define ipo_buffer_size 500
#define name_buffer_size MAX_PATH+1
	bool is_program_on(wchar_t* const name) {
		std::vector<DWORD> pi_buffer;
		pi_buffer.resize(ipo_buffer_size);
		DWORD num_proc;
		while(true)
		{
			DWORD num_bytes=pi_buffer.size()*sizeof(DWORD);
			EnumProcesses(pi_buffer.data(),num_bytes,&num_proc);
			if(num_proc!=num_bytes)
			{
				break;
			}
			pi_buffer.resize(2*pi_buffer.size());
		}
		num_proc/=sizeof(DWORD);
		for(unsigned int i=0;i<num_proc;++i)
		{
			HANDLE proc=OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,pi_buffer[i]);
			wchar_t name_buffer[name_buffer_size];
			GetProcessImageFileName(proc,name_buffer,name_buffer_size);
			unsigned int last_slash=wcslen(name_buffer);
			while(last_slash-->0)
			{
				if(name_buffer[last_slash]=='\\') break;
			}
			++last_slash;
			bool matches=!wcscmp(name_buffer+last_slash,name);
			CloseHandle(proc);
			if(matches)
			{
				return true;
			}
		}
		return false;
	}
	int release_key(VK_CODE const code)  noexcept {
		INPUT input;
		rkc_init(code);
		return !SendInput(1,&input,sizeof(INPUT));
	}
	int release_key(char const c) noexcept {
		if(is_key(c))
		{
			return release_key(char_to_vk(c));
		}
		return 2;
	}
	int press_key(VK_CODE const code) noexcept {
		INPUT input;
		pkc_init(code);
		return !SendInput(1,&input,sizeof(INPUT));
	}
	int press_key(char const c) noexcept {
		if(is_key(c))
		{
			return press_key(char_to_vk(c));
		}
		return 2;
	}
	int tap_key(VK_CODE const code) noexcept {
		INPUT inputs[2];
		tkc_init(code);
		return !SendInput(2,inputs,sizeof(INPUT));
	}
	int tap_key(char const c) noexcept {
		if(is_key(c))
		{
			return tap_key(char_to_vk(c));
		}
		return 2;
	}

	VK_CODE shift_scancode=MapVirtualKey(VK_SHIFT,MAPVK_VK_TO_VSC);

	std::vector<INPUT> string_to_inputs(std::string const& str) {
		std::vector<INPUT> inputs;
		inputs.reserve(3*str.length());
		for(unsigned int i=0;i<str.length();++i)
		{
			if(is_key(str[i]))
			{
				VK_CODE scancode=MapVirtualKey(char_to_vk(str[i]),MAPVK_VK_TO_VSC);
				inputs.push_back({INPUT_KEYBOARD});
				inputs.back().ki={0,scancode,KEYEVENTF_SCANCODE,0,0};
				inputs.push_back({INPUT_KEYBOARD});
				inputs.back().ki={0,scancode,KEYEVENTF_SCANCODE|KEYEVENTF_KEYUP,0,0};
			}
			else
			{
				char unshifted=unshift(str[i]);
				if(unshifted==str[i])
				{
					throw std::exception("Untypeable character");
				}
				VK_CODE scancode=MapVirtualKey(char_to_vk(unshifted),MAPVK_VK_TO_VSC);
				inputs.push_back({INPUT_KEYBOARD});
				inputs.back().ki={0,shift_scancode,KEYEVENTF_SCANCODE,0,0};
				inputs.push_back({INPUT_KEYBOARD});
				inputs.back().ki={0,scancode,KEYEVENTF_SCANCODE,0,0};
				inputs.push_back({INPUT_KEYBOARD});
				inputs.back().ki={0,scancode,KEYEVENTF_SCANCODE|KEYEVENTF_KEYUP,0,0};
				inputs.push_back({INPUT_KEYBOARD});
				inputs.back().ki={0,shift_scancode,KEYEVENTF_SCANCODE|KEYEVENTF_KEYUP,0,0};
			}
		}
		return inputs;
	}

	int type_string(std::string const& str) noexcept {
		try
		{
			auto inputs=string_to_inputs(str);
			return inject_inputs(inputs);
		}
		catch(std::exception&)
		{
			return 2;
		}
	}

	VK_CODE ctrl_scancode=MapVirtualKey(VK_CONTROL,MAPVK_VK_TO_VSC);

	int ctrl_combo(VK_CODE const code) noexcept {
		INPUT inputs[4];
		inputs[0].type=INPUT_KEYBOARD;
		inputs[0].ki={0,ctrl_scancode,KEYEVENTF_SCANCODE,0,0};
		inputs[1].type=INPUT_KEYBOARD;
		inputs[1].ki={0,static_cast<WORD>(MapVirtualKey(code,MAPVK_VK_TO_VSC)),KEYEVENTF_SCANCODE,0,0};
		inputs[2].type=INPUT_KEYBOARD;
		inputs[2].ki={0,inputs[1].ki.wScan,KEYEVENTF_SCANCODE|KEYEVENTF_KEYUP,0,0};
		inputs[3].type=INPUT_KEYBOARD;
		inputs[3].ki={0,ctrl_scancode,KEYEVENTF_SCANCODE|KEYEVENTF_KEYUP,0,0};
		return !SendInput(4,inputs,sizeof(INPUT));
	}

	int ctrl_combo(char const c) noexcept {
		if(is_key(c))
		{
			return ctrl_combo(char_to_vk(c));
		}
		return 2;
	}

	std::vector<INPUT> combo_to_inputs(Combo const& combo) {
		std::vector<INPUT> inputs(combo.size()*2);
		unsigned int ci,ii;
		for(ci=0,ii=0;ii<combo.size();++ci,++ii)
		{
			inputs[ii].type=INPUT_KEYBOARD;
			inputs[ii].ki={0,static_cast<WORD>(MapVirtualKey(combo[ci],MAPVK_VK_TO_VSC)),KEYEVENTF_SCANCODE,0,0};
		}
		for(--ci;ii<inputs.size();--ci,++ii)
		{
			inputs[ii].type=INPUT_KEYBOARD;
			inputs[ii].ki={0,inputs[ci].ki.wScan,KEYEVENTF_SCANCODE|KEYEVENTF_KEYUP,0,0};
		}
		return inputs;
	}

	int combo(Combo const& combo) {
		return inject_inputs(combo_to_inputs(combo));
	}

	struct dim {
		float x,y;
	};
	dim get_screen_conv() noexcept {
		SetProcessDPIAware();
		return {65536.0f/GetSystemMetrics(SM_CXSCREEN),65536.0f/GetSystemMetrics(SM_CYSCREEN)};
	}
	dim const screen_conv=get_screen_conv();
	int move_mouse(ULONG const x,ULONG const y) noexcept {
		INPUT input;
		mmc_init(x+1,y+1);
		return !SendInput(1,&input,sizeof(INPUT));
	}

	int translate_mouse(LONG const x,LONG const y) noexcept {
		INPUT input;
		trmc_init(x,y);
		return !(SendInput(1,&input,sizeof(INPUT)));
	}

	int tap_mouse_button(unsigned int const button) noexcept {
		INPUT inputs[2];
		tmc_init(button);
		return !SendInput(2,inputs,sizeof(INPUT));
	}
	int press_mouse_button(unsigned int const button) noexcept {
		INPUT input;
		pmc_init(button);
		return !(SendInput(1,&input,sizeof(INPUT)));
	}
	int release_mouse_button(unsigned int const button) noexcept {
		INPUT input;
		rmc_init(button);
		return !SendInput(1,&input,sizeof(INPUT));
	}

	std::string get_clipboard_string() {
		if(!OpenClipboard(GetActiveWindow()))
		{
			throw std::exception();
		}
		char* data;
		if(data=(char*)GetClipboardData(CF_TEXT))
		{
			CloseClipboard();
			return std::string(data);
		}
		CloseClipboard();
		throw std::exception();
	}

	int set_clipboard_string(std::string const& str) noexcept {
		if(!OpenClipboard(GetActiveWindow()))
		{
			return 2;
		}
		if(!EmptyClipboard())
		{
			return 3;
		}
		size_t datasize=str.length()+1;
		HGLOBAL gdata=GlobalAlloc(GMEM_FIXED,str.length()+1);
		memcpy_s(gdata,datasize,str.data(),datasize);
		if(SetClipboardData(CF_TEXT,gdata))
		{
			CloseClipboard();
			return 0;
		}
		CloseClipboard();
		GlobalFree(gdata);
		return 1;
	}

	//Process::Process():prog_name(nullptr),params(nullptr),pi(new PROCESS_INFORMATION) {}
	Process::Process(std::wstring const& prog_name):
		Process(prog_name,std::wstring(L"")) {}
	Process::Process(std::wstring const& prog_name,std::wstring const& params) {
		this->prog_name=new wchar_t[prog_name.length()+1];
		wcscpy(this->prog_name,prog_name.c_str());
		if(!params.empty()&&params[0]!=' ')
		{
			this->params=new wchar_t[params.length()+2];
			this->params[0]=L' ';
			wcscpy(this->params+1,params.c_str());
		}
		else
		{
			this->params=new wchar_t[params.length()+1];
			wcscpy(this->params,params.c_str());
		}
		pi=new PROCESS_INFORMATION{};
	}
	Process::~Process() {
		wait_close();
		delete[] prog_name;
		delete[] params;
		delete pi;
	}
	Process::Process(Process const& other) {
		params=new wchar_t[wcslen(other.params)+1];
		wcscpy(params,other.params);
		prog_name=new wchar_t[wcslen(other.prog_name)+1];
		wcscpy(prog_name,other.params);
		pi=new PROCESS_INFORMATION{};
	}
	Process::Process(Process&& other) {
		params=other.params;
		prog_name=other.prog_name;
		pi=other.pi;
		other.params=nullptr;
		other.prog_name=nullptr;
		other.pi=nullptr;
	}
	Process& Process::operator=(Process const& other) {
		if(this!=&other)
		{
			wait_close();
			*pi={};
			delete[] prog_name;
			delete[] params;
			params=new wchar_t[wcslen(other.params)+1];
			wcscpy(params,other.params);
			prog_name=new wchar_t[wcslen(other.prog_name)+1];
			wcscpy(prog_name,other.prog_name);
		}
		return *this;
	}
	Process& Process::operator=(Process&& other) {
		wait_close();
		delete[] params;
		delete[] prog_name;
		delete pi;
		params=other.params;
		prog_name=other.prog_name;
		pi=other.pi;
		other.params=nullptr;
		other.prog_name=nullptr;
		other.pi=nullptr;
		return *this;
	}
	DWORD Process::get_process_id() const {
		return pi->dwProcessId;
	}
	DWORD Process::get_thread_id() const {
		return pi->dwThreadId;
	}
	int Process::start(STARTUPINFO& si,DWORD flags) const {
		return !CreateProcess(
			prog_name,
			params,
			NULL,
			NULL,
			FALSE,
			flags,
			NULL,
			NULL,
			&si,
			pi
		);
	}
	bool Process::operator==(Process const& other) const {
		return
			prog_name==other.prog_name&&
			params==other.params&&
			pi==other.pi;
	}

	void Process::wait_close(DWORD timeout) const {
		if(pi->hProcess)
		{
			WaitForSingleObject(pi->hProcess,timeout);
			CloseHandle(pi->hProcess);
			CloseHandle(pi->hThread);
		}
	}
	void Process::set_parameters(std::wstring const& params) {
		delete[] this->params;
		this->params=new wchar_t[params.length()+1];
		wcscpy(this->params,params.c_str());
	}
	wchar_t const* Process::get_parameters() const {
		return params;
	}
	void Process::set_program(std::wstring const& prog_name) {
		delete[] this->prog_name;
		this->prog_name=new wchar_t[prog_name.length()+1];
		wcscpy(this->prog_name,prog_name.c_str());
	}
	wchar_t const* Process::get_program() const {
		return prog_name;
	}

	int MultiCommand::execute() {
		return inject_inputs(inputs);
	}

	int DoubleCommand::execute() {
		return !SendInput(2,inputs,sizeof(INPUT));
	}
	int SingleCommand::execute() {
		return !SendInput(1,&input,sizeof(INPUT));
	}

	TypeCommand::TypeCommand(std::string const& str) {
		inputs=string_to_inputs(str);
	}

	ComboCommand::ComboCommand(Combo const& codes) {
		inputs=combo_to_inputs(codes);
	}

	PressKeyCommand::PressKeyCommand(VK_CODE code) {
		pkc_init(code);
	}

	PressKeyCommand::PressKeyCommand(char key) {
		if(is_key(key))
		{
			pkc_init(char_to_vk(key));
		}
		throw std::exception("Invalid char");
	}

	ReleaseKeyCommand::ReleaseKeyCommand(VK_CODE code) {
		rkc_init(code);
	}
	ReleaseKeyCommand::ReleaseKeyCommand(char key) {
		if(is_key(key))
		{
			rkc_init(char_to_vk(key));
		}
		throw std::exception("Invalid char");
	}

	TapKeyCommand::TapKeyCommand(VK_CODE code) {
		tkc_init(code);
	}
	TapKeyCommand::TapKeyCommand(char key) {
		if(is_key(key))
		{
			tkc_init(char_to_vk(key));
		}
		throw std::exception("Invalid char");
	}

	TapMouseCommand::TapMouseCommand(unsigned int button) {
		tmc_init(button);
	}

	PressMouseCommand::PressMouseCommand(unsigned int button) {
		pmc_init(button);
	}

	ReleaseMouseCommand::ReleaseMouseCommand(unsigned int button) {
		rmc_init(button);
	}

	MoveMouseCommand::MoveMouseCommand(ULONG x,ULONG y) {
		mmc_init(x,y);
	}

	TranslateMouseCommand::TranslateMouseCommand(LONG x,LONG y) {
		trmc_init(x,y);
	}

	SleepCommand::SleepCommand(DWORD const time):time(time) {}

	int SleepCommand::execute() {
		Sleep(time);
		return 0;
	}
	void CommandList::loop_until_key_pressed(VK_CODE esc_code) {
		while(true)
		{
			for(unsigned int i=0;i<this->size();++i)
			{
				do
				{
					if(GetAsyncKeyState(esc_code))
					{
						return;
					}
				} while((*this)[i]->execute());
			}
		}
	}

#undef pkc_init
#undef rkc_init
#undef tkc_init
#undef pmc_init
#undef rmc_init
#undef tmc_init
#undef mmc_int
#undef trmc_init
}

