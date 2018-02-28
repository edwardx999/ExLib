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
#ifndef MACROS_H
#define MACROS_H
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#ifdef _WINDOWS
#ifdef MACROS_EXPORTS
#define MACROS_API __declspec(dllexport)
#else
#define MACROS_API __declspec(dllimport)
#endif
#include <Windows.h>
#endif //_WINDOWS
namespace exlib {
#ifdef _WINDOWS
	typedef WORD VK_CODE;
#else
	typedef unsigned short VK_CODE;
#endif //WINDOWS
	enum vk_codes:VK_CODE {
		VK_A
	#ifdef _WINDOWS
		=0x41
	#endif
		,
		VK_B,
		VK_C,
		VK_D,
		VK_E,
		VK_F,
		VK_G,
		VK_H,
		VK_I,
		VK_J,
		VK_K,
		VK_L,
		VK_M,
		VK_N,
		VK_O,
		VK_P,
		VK_Q,
		VK_R,
		VK_S,
		VK_T,
		VK_U,
		VK_V,
		VK_W,
		VK_X,
		VK_Y,
		VK_Z,
		VK_0
	#ifdef _WINDOWS
		=0x30
	#endif
		,
		VK_1,
		VK_2,
		VK_3,
		VK_4,
		VK_5,
		VK_6,
		VK_7,
		VK_8,
		VK_9,
	};
	enum mouse_codes {
		ME_MOUSE1,
		ME_MOUSE2,
		ME_MOUSE3,
	};
	typedef std::vector<VK_CODE> Combo;
	/*
		Converts a char to a virtual key code
		If there is no code, 0 is returned
	*/
	constexpr VK_CODE MACROS_API char_to_vk(char const c) noexcept;
	/*
		Returns the "lowercase" version of any character
	*/
	constexpr char MACROS_API unshift(char const c) noexcept;
	/*
		Turns a string into a vector of key inputs
	*/
	std::vector<INPUT> MACROS_API string_to_inputs(std::string const& str);
	/*
		Turns a code combo (chord) into a vector of key inputs
	*/
	std::vector<INPUT> MACROS_API combo_to_inputs(Combo const&);
	/*
		Injects a vector of inputs into the System
	*/
	int inject_inputs(std::vector<INPUT>& inputs);
	/*
		Returns whether the character can be typed with one key press
	*/
	constexpr bool MACROS_API is_key(char const c) noexcept;
	/*
		Returns whether a window (e.g. L"IEFrame") is open
	*/
	bool MACROS_API is_window_open(wchar_t* const name) noexcept;
	/*
		Returns whether a program (e.g. L"iexplore.exe") is on
	*/
	bool MACROS_API is_program_on(wchar_t* const name);
	/*
		Releases a given key by key code
		Returns 0 on success
		Returns 1 on failure to send input
	*/
	int MACROS_API release_key(VK_CODE const code) noexcept;
	/*
		Releases a given key by character
		Returns 0 on success
		Returns 1 on failure to send iput
		Returns 2 on non-existent key
	*/
	int MACROS_API release_key(char const c) noexcept;
	/*
		Presses a given key by key code
		Returns 0 on success
		Returns 1 on failure to send input
	*/
	int MACROS_API press_key(VK_CODE const code) noexcept;
	/*
		Presses a given key by character
		Returns 0 on success
		Returns 1 on failure to send iput
		Returns 2 on non-existent key
	*/
	int MACROS_API press_key(char const c) noexcept;
	/*
		Taps a given key by key code
		Returns 0 on success
		Returns 1 on failure to send input
	*/
	int MACROS_API tap_key(VK_CODE const code) noexcept;
	/*
		Taps a given key by character
		Returns 0 on success
		Returns 1 on failure to send iput
		Returns 2 on non-existent key
	*/
	int MACROS_API tap_key(char const c) noexcept;
	/*
		Types out a string
		Returns 0 on success
		Returns 1 on failure
		Returns 2 on invalid character
	*/
	int MACROS_API type_string(std::string const& str) noexcept;
	/*
		Presses a key by key code while holding ctrl
		Returns 0 on success
		Returns 1 on failure to send input
	*/
	int MACROS_API ctrl_combo(VK_CODE const code) noexcept;
	/*
		Presses a key by key code while holding ctrl
		Returns 0 on success
		Returns 1 on failure to send input
		Returns 2 on non-existent key
	*/
	int MACROS_API ctrl_combo(char const c) noexcept;
	/*
		Taps the keys as a combo
		e.g. {VK_CONTROL,VK_ALT,VK_C} => Ctrl+Alt+c
	*/
	int MACROS_API combo(Combo const& combo);
	/*
		Moves the mouse to absolute x y
		Returns 0 on success
		Returns 1 on failure to send input
	*/
	int MACROS_API move_mouse(ULONG const x,ULONG const y) noexcept;
	/*
		Moves the mouse a relative x y
		Returns 0 on success
		Returns 1 on failure to send input
	*/
	int MACROS_API translate_mouse(LONG const x,LONG const y) noexcept;
	/*
		Taps a button on the mouse. Buttons are in ME_ definitions
		Returns 0 on success
		Returns 1 on failure to send input
	*/
	int MACROS_API tap_mouse_button(unsigned int const button) noexcept;
	/*
		Presses down a button on the mouse. Buttons are in ME_ definitions
		Returns 0 on success
		Returns 1 on failure to send input
	*/
	int MACROS_API press_mouse_button(unsigned int const button) noexcept;
	/*
		Releases a button on the mouse. Buttons are in ME_ definitions
		Returns 0 on success
		Returns 1 on failure to send input
	*/
	int MACROS_API release_mouse_button(unsigned int const button) noexcept;
	/*
		Returns the clipboard data as a string
		Throws exception on failure to access clipboard
	*/
	std::string MACROS_API get_clipboard_string();
	/*
		Puts a string on the clipboard
		Returns 0 on success
		Returns 1 on failure to set clipboard data
		Returns 2 on failure to open clipboard
		Returns 3 on failure to clear clipboard
	*/
	int MACROS_API set_clipboard_string(std::string const& str) noexcept;

#ifdef _WINDOWS
	/*
		Class used to run a program with parameters
	*/
	class MACROS_API Process {
	private:
		wchar_t*				prog_name;
		wchar_t*				params;
		PROCESS_INFORMATION*	pi;
	public:
		//Process();
		/*
			Creates a process to execute a program with no parameters
		*/
		explicit Process(std::wstring const& prog_name);
		/*
			Creates a process to execute a program with given parameters
		*/
		explicit Process(std::wstring const& prog_name,std::wstring const& params);

		Process(Process const& other);
		Process(Process&& other);

		Process& operator=(Process const& other);
		Process& operator=(Process&& other);
		~Process();

		bool operator==(Process const& other) const;
		/*
			Starts a program
			Returns 0 on success
			Returns 1 on failure
		*/
		int start(STARTUPINFO& si=STARTUPINFO{sizeof(STARTUPINFO)},DWORD flags=0) const;
		DWORD get_process_id() const;
		DWORD get_thread_id() const;
		void set_parameters(std::wstring const& params);
		wchar_t const* get_parameters() const;
		void set_program(std::wstring const& prog_name);
		wchar_t const* get_program() const;
		void wait_close(DWORD timeout=ULONG_MAX) const;
	};
#endif //WINDOWS
	class MACROS_API Command {
	protected:
		Command()=default;
	public:
		virtual ~Command()=default;
		/*
			Should return 0 on success
		*/
		virtual int execute()=0;
	};
	class MACROS_API MultiCommand:public Command {
	protected:
		std::vector<INPUT> inputs;
		MultiCommand()=default;
	public:
		virtual ~MultiCommand()=default;
		int execute();
	};
	class MACROS_API DoubleCommand:public Command {
	protected:
		INPUT inputs[2];
		DoubleCommand()=default;
	public:
		virtual ~DoubleCommand()=default;
		int execute();
	};
	class MACROS_API SingleCommand:public Command {
	protected:
		INPUT input;
		SingleCommand()=default;
	public:
		virtual ~SingleCommand()=default;
		int execute();
	};

	class MACROS_API TypeCommand:public MultiCommand {
	public:
		TypeCommand(std::string const&);
		~TypeCommand()=default;
	};
	class MACROS_API ComboCommand:public MultiCommand {
	public:
		ComboCommand(Combo const&);
		~ComboCommand()=default;
	};
	class MACROS_API PressKeyCommand:public SingleCommand {
	public:
		PressKeyCommand(VK_CODE code);
		PressKeyCommand(char key);
		~PressKeyCommand()=default;
	};
	class MACROS_API ReleaseKeyCommand:public SingleCommand {
	public:
		ReleaseKeyCommand(VK_CODE code);
		ReleaseKeyCommand(char key);
		~ReleaseKeyCommand()=default;
	};
	class MACROS_API TapKeyCommand:public DoubleCommand {
	public:
		TapKeyCommand(VK_CODE code);
		TapKeyCommand(char key);
		~TapKeyCommand()=default;
	};
	class MACROS_API TapMouseCommand:public DoubleCommand {
	public:
		TapMouseCommand(unsigned int button);
		~TapMouseCommand()=default;
	};
	class MACROS_API PressMouseCommand:public SingleCommand {
	public:
		PressMouseCommand(unsigned int button);
		~PressMouseCommand()=default;
	};
	class MACROS_API ReleaseMouseCommand:public SingleCommand {
	public:
		ReleaseMouseCommand(unsigned int button);
		~ReleaseMouseCommand()=default;
	};
	class MACROS_API MoveMouseCommand:public SingleCommand {
	public:
		MoveMouseCommand(ULONG x,ULONG y);
		~MoveMouseCommand()=default;
	};
	class MACROS_API TranslateMouseCommand:public SingleCommand {
	public:
		TranslateMouseCommand(LONG dx,LONG dy);
		~TranslateMouseCommand()=default;
	};
	class MACROS_API SleepCommand:public Command {
	private:
		DWORD time;
	public:
		SleepCommand(DWORD const time);
		~SleepCommand()=default;
		int execute();
	};

	class CommandList:public std::vector<std::unique_ptr<Command>> {
	public:
		template<typename CMD,typename... Args>
		inline void add_command(Args... args)
		{
			this->emplace_back(std::make_unique<CMD>(args...));
		}
		void MACROS_API loop_until_key_pressed(VK_CODE esc_code=VK_ESCAPE);
		void MACROS_API loop_for_time(unsigned long millis);
	};

	/*
		Exists for consistency with rest of library
		Use POINT p;GetCursorPos(&p); instead if on Windows
	*/
#ifdef _WINDOWS
	typedef POINT Point;
#else
	struct Point {
		int x,y;
	}
#endif _WINDOWS
	Point cursor_pos();

	std::ostream& operator<<(std::ostream& os,Point p);

#ifdef _WINDOWS
	inline POINT cursor_pos()
	{
		POINT pos;
		GetCursorPos(&pos);
		return pos;
	}
#endif _WINDOWS
	inline std::ostream& operator<<(std::ostream& os,Point p)
	{
		return os<<'('<<p.x<<','<<p.y<<')';
	}
#ifdef _WINDOWS
	inline int inject_inputs(std::vector<INPUT>& inputs)
	{
		return !(SendInput(inputs.size(),inputs.data(),sizeof(INPUT)));
	}
#endif //_WINDOWS
}
namespace mcomm=exlib;
#endif //MACROS_H