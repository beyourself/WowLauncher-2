
#include "Process.hpp"
#include "WindowsError.hpp"
#include <stdexcept>

namespace launcher {
	Process::Process() {
		PROCESS_INFORMATION pi = { 0 };
		this->processInfo = pi;
	}

	Process::Process(const std::wstring &name) {
		PROCESS_INFORMATION pi = { 0 };
		this->processInfo = pi;

		this->open(name);
	}

	Process::Process(const std::wstring &name, const std::wstring &cmdLine) {
		PROCESS_INFORMATION pi = { 0 };
		this->processInfo = pi;

		this->open(name, cmdLine);
	}

	Process::~Process() {
		this->close();
	}

	void Process::open(const std::wstring &name) {
		this->open(name, L"");
	}

	void Process::open(const std::wstring &name, const std::wstring &cmdLine) {
		const wchar_t *lpAppName = nullptr;
		wchar_t* lpCmdLine = nullptr;

		STARTUPINFO si = { 0 };
		si.cb = sizeof(STARTUPINFO);

		// prepare application name
		lpAppName = name.c_str();

		// prepare command line
		if (cmdLine != L"") {
			lpCmdLine = const_cast<wchar_t*>(cmdLine.c_str());
		} else {
			lpCmdLine = nullptr;
		}
		
		// prepare creation flags
		DWORD dwCreateFlags = CREATE_SUSPENDED;

		// invoke process
		PROCESS_INFORMATION pi = { 0 };
		BOOL result = ::CreateProcess (
			lpAppName, lpCmdLine,
			NULL, NULL, FALSE,
			dwCreateFlags, NULL, NULL,
			&si, &pi
		);

		if (result == FALSE) {
			throw WindowsError(::GetLastError());
		}

		this->processInfo = pi;
	}

	void Process::start() {
		if (::ResumeThread(this->processInfo.hThread) == FALSE) {
			throw WindowsError(::GetLastError());
		} 
	}

	void Process::terminate() {
		if (::TerminateProcess(this->processInfo.hProcess, -1) == FALSE) {
			throw WindowsError(::GetLastError());
		} 
	}

	void Process::close() {
		::CloseHandle(this->processInfo.hProcess);
		::CloseHandle(this->processInfo.hThread);

		PROCESS_INFORMATION pi = { 0 };
		this->processInfo = pi;
	}

	void Process::wait() {
		if (::WaitForSingleObject(this->processInfo.hProcess, INFINITE) == WAIT_FAILED) {
			throw WindowsError(::GetLastError());
		}
	}

	int Process::getExitCode() const {
		DWORD exitCode = 0;

		if (!::GetExitCodeProcess(this->processInfo.hProcess, &exitCode)) {
			throw WindowsError(::GetLastError());
		} else {
			return static_cast<int>(exitCode);
		}
	}
}
