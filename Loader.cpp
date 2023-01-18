#include "dispath/defines.h"

#include <iostream>
#include <string>

using namespace std;

DWORD driver_pid = 0;
ULONG build_number = 0;

int64_t last_error = 0;

BOOL(*OperationCallback)(ULONG iMode, ULONG magic, void* a1, FLONG flRed, FLONG flGreen, FLONG flBlue);

#define xor_status_success (0)
#define xor_status_unsuccessful (3221225473)

void attach_driver(DWORD pid)
{
	driver_pid = pid;
}

bool driver_setup()
{
	//build_number = *(ULONG*)((2147353184));

	//if (build_number < (19041))
	//	return false;

	auto win32u = GetModuleHandleW((L"win32u.dll"));
	if (!win32u)
	{
		win32u = LoadLibraryW((L"win32u.dll"));
		if (!win32u)
			return false;
	}

	auto addr = GetProcAddress(win32u, ("NtGdiEngCreatePalette"));
	if (!addr)
		return false;

	*(PVOID*)&OperationCallback = addr;

	return true;
}

bool send_message(operation_data* req)
{
	uint64_t buffer = (1337);

	constexpr ULONG firstCall = 1;

	auto veh = AddVectoredExceptionHandler(firstCall, [](PEXCEPTION_POINTERS exceptionHandler) -> LONG
		{
			auto context = exceptionHandler->ContextRecord;
			context->Rip += 8;

			return EXCEPTION_CONTINUE_EXECUTION;
		});

	if (!veh)
	{
		printf("[-] veh failed\n");
		return false;
	}

	OperationCallback(0x000004, 0x128, req, 0xF33FC0C1, 0x1, 0x1);

	if (!RemoveVectoredExceptionHandler(veh))
	{
		printf("[-] rem_veh failed\n");
		return false;
	}

	return true;
}

int64_t get_last_error()
{
	return last_error;
}

void set_last_error(int64_t error)
{
	last_error = error;
}

bool is_driver_load_ex()
{
	operation_data req = {};

	req.comm_code = comm_code::is_driver_load;
	req.comm_key = (65452);

	if (!send_message(&req))
		return false;

	set_last_error(req.status_code);

	return (req.load_code == (926343));
}

bool read_memory_phys_ex(PVOID base, PVOID buffer, DWORD size)
{
	operation_data req = {};

	req.comm_code = comm_code::read_mem_phys;
	req.comm_key = (65452);

	if (!driver_pid || !base || !size || (uint64_t)base > 0xFFFFFFFFFFFF)
		return false;

	req.rmp_pid = driver_pid;
	req.rmp_address = base;
	req.rmp_buffer = buffer;
	req.rmp_size = size;

	if (!send_message(&req))
		return false;

	set_last_error(req.status_code);

	return true;
}

template<typename T>
inline T ReadMemory(uint64_t address)
{
	T buffer{ };
	bool status = read_memory_phys_ex((PVOID)address, &buffer, sizeof(T));
	
	if (!status)
	{
		printf("[-] Failed to read memory.\n");
		return {};
	}

	return buffer;
}

bool write_memory_phys_ex(PVOID base, PVOID buffer, DWORD size)
{
	operation_data req = {};

	req.comm_code = comm_code::write_mem_phys;
	req.comm_key = (65452);

	if (!driver_pid || !base || !buffer || !size || (uint64_t)base > 0xFFFFFFFFFFFF)
		return false;

	req.wmp_pid = driver_pid;
	req.wmp_address = base;
	req.wmp_buffer = buffer;
	req.wmp_size = size;

	if (!send_message(&req))
		return false;

	set_last_error(req.status_code);

	return true;
}

template<typename T>
bool  Write(uintptr_t write_address, T value)
{
	auto pValue = value;

	write_memory_phys_ex((PVOID)write_address, (PVOID)&pValue, sizeof(value));
	return true;
}

void* process_base_address_ex()
{
	operation_data req = {};

	req.comm_code = comm_code::process_base_address;
	req.comm_key = (65452);

	req.pba_pid = driver_pid;

	send_message(&req);
	set_last_error(req.status_code);

	return req.pba_out_address;
}

bool send_buffer_fn(uint32_t index, ULONGLONG in_buffer1, ULONGLONG in_buffer2)
{
	operation_data req = {};

	req.comm_code = comm_code::send_buffer;
	req.comm_key = (65452);

	req.sb_index = index;
	req.sb_in_buffer1 = in_buffer1;
	req.sb_in_buffer2 = in_buffer2;

	if (!send_message(&req))
		return false;

	set_last_error(req.status_code);

	return true;
}

bool get_buffer_fn(uint32_t index, ULONGLONG* out_buffer1, ULONGLONG* out_buffer2)
{
	operation_data req = {};

	req.comm_code = comm_code::get_buffer;
	req.comm_key = (65452);

	req.gb_index = index;

	if (!send_message(&req))
		return false;

	set_last_error(req.status_code);

	if (out_buffer1 != 0)
		*out_buffer1 = req.gb_out_buffer1;

	if (out_buffer2 != 0)
		*out_buffer2 = req.gb_out_buffer2;

	return true;
}

bool set_process_id_ex(DWORD current_pid, DWORD new_pid)
{
	operation_data req = {};

	req.comm_code = comm_code::set_process_id;
	req.comm_key = (65452);

	req.spi_new_pid = new_pid;
	req.spi_current_pid = current_pid;

	if (!send_message(&req))
		return false;

	set_last_error(req.status_code);

	return true;
}

bool get_async_key_state_ex(uint8_t vk_code)
{
	operation_data req = {};

	req.comm_code = comm_code::get_async_key_state;
	req.comm_key = (65452);

	req.gaks_vk_code = vk_code;

	if (!send_message(&req))
		return false;

	set_last_error(req.status_code);

	return req.gaks_is_key;
}

bool mouse_event_ex(long x, long y, unsigned short button_flags)
{
	operation_data req = {};

	req.comm_code = comm_code::mouse_input;
	req.comm_key = (65452);

	req.mi_x = x;
	req.mi_y = y;
	req.mi_button_flags = button_flags;

	if (!send_message(&req))
		return false;

	set_last_error(req.status_code);

	return true;
}

bool change_protect_window_ex(HWND window, uint32_t flag)
{
	operation_data req = {};

	req.comm_code = comm_code::change_protect_window;
	req.comm_key = (65452);

	req.cpw_value = flag;
	req.cpw_window_handle = reinterpret_cast<uint64_t>(window);

	if (!send_message(&req))
		return false;

	set_last_error(req.status_code);

	return true;
}

PVOID get_peb_address_ex(int32_t* iswow64)
{
	operation_data req = {};

	req.comm_code = comm_code::get_peb_address;
	req.comm_key = (65452);

	req.gpa_pid = driver_pid;

	if (!send_message(&req))
		return 0;

	*iswow64 = req.gpa_iswow64;

	set_last_error(req.status_code);

	return req.gpa_address;
}

bool get_thread_context_ex(HWND window_handle, uint64_t* thread_context)
{
	operation_data req = {};

	req.comm_code = comm_code::get_thread_context;
	req.comm_key = (65452);

	req.gtc_window_handle = reinterpret_cast<uint64_t>(window_handle);
	req.gtc_out_put = 0;

	if (!send_message(&req))
		return false;

	set_last_error(req.status_code);

	*thread_context = req.gtc_out_put;

	return true;
}

bool set_thread_context_ex(HWND window_handle, uint64_t thread_context)
{
	operation_data req = {};

	req.comm_code = comm_code::set_thread_context;
	req.comm_key = (65452);

	req.stc_window_handle = reinterpret_cast<uint64_t>(window_handle);
	req.stc_thread_context = thread_context;

	if (!send_message(&req))
		return false;

	set_last_error(req.status_code);

	return true;
}

bool get_guarded_region_ex(uint64_t* guarded_region)
{
	operation_data req = {};

	req.comm_code = comm_code::get_guarded_region;
	req.comm_key = (65452);

	if (!send_message(&req))
		return false;

	set_last_error(req.status_code);

	if (guarded_region)
		*guarded_region = req.ggr_guarded_region;

	return true;
}

bool read_kernel_memory_ex(PVOID base, PVOID buffer, DWORD size)
{
	operation_data req = {};

	req.comm_code = comm_code::read_kernel_memory;
	req.comm_key = (65452);

	if (!base || !size || !buffer || base == 0)
		return false;

	req.rkm_address = base;
	req.rkm_buffer = buffer;
	req.rkm_size = size;

	if (!send_message(&req))
		return false;

	set_last_error(req.status_code);

	return true;
}

struct LIST_ENTRY_64 {
	std::uint64_t Flink;
	std::uint64_t Blink;
};

struct UNICODE_STRING_64 {
	unsigned short Length;
	unsigned short MaximumLength;
	std::uint64_t  Buffer;
};

struct PEB_64 {
	unsigned char _unused_1[4];
	std::uint64_t _unused_2[2];
	std::uint64_t Ldr;
};

struct PEB_LDR_DATA_64 {
	unsigned long Length;
	unsigned long Initialized;
	std::uint64_t SsHandle;
	LIST_ENTRY_64 InLoadOrderModuleList;
};

struct LDR_DATA_TABLE_ENTRY_64 {
	LIST_ENTRY_64 InLoadOrderLinks;
	LIST_ENTRY_64 InMemoryOrderLinks;
	LIST_ENTRY_64 InInitializationOrderLinks;
	std::uint64_t DllBase;
	std::uint64_t EntryPoint;
	union {
		unsigned long SizeOfImage;
		std::uint64_t _dummy;
	};
	UNICODE_STRING_64 FullDllName;
	UNICODE_STRING_64 BaseDllName;
};



uint64_t get_process_module_ex(const wchar_t* name)
{
	int32_t iswow64 = 0;
	PVOID process_peb = get_peb_address_ex(&iswow64);

	if (iswow64 == 0)
	{
		if (!process_peb)
			return 0;

		PEB_64 peb_ex{};
		if (!read_memory_phys_ex(process_peb, &peb_ex, sizeof(PEB_64)))
			return 0;

		PEB_LDR_DATA_64 ldr_list{};
		if (!read_memory_phys_ex((PVOID)(peb_ex.Ldr), &ldr_list, sizeof(PEB_LDR_DATA_64)))
			return 0;

		uint64_t first_link = ULONG64(ldr_list.InLoadOrderModuleList.Flink);
		uint64_t forward_link = first_link;

		do
		{
			LDR_DATA_TABLE_ENTRY_64 entry{};
			if (!read_memory_phys_ex((PVOID)forward_link, &entry, sizeof(LDR_DATA_TABLE_ENTRY_64)))
				continue;

			wstring buffer(entry.BaseDllName.Length, 0);
			read_memory_phys_ex((PVOID)entry.BaseDllName.Buffer, (PVOID)buffer.data(), entry.BaseDllName.Length);
			forward_link = ULONG64(entry.InLoadOrderLinks.Flink);

			if (!entry.DllBase)
				continue;

			if (wcscmp(buffer.c_str(), name) == 0)
				return entry.DllBase;

		} while (forward_link && forward_link != first_link);
	}

	return 0;
}





ULONG FindProcessIdByWindow(string WindowName)
{
	ULONG ProcessId = 0;

	HWND hwnd = FindWindowA(NULL, WindowName.c_str());
	GetWindowThreadProcessId(hwnd, &ProcessId);

	return ProcessId;
}

int main()
{
	setlocale(0, "");

	printf("[+] Initializing operation callback...\n");

	if (!driver_setup())
	{
		printf("[-] Failed to init operation callback.");
		Sleep(1200);
		return EXIT_FAILURE;
	}

	printf("[+] Success!\n\n");

	ULONG pID = FindProcessIdByWindow("Rust");

	if (!pID)
	{
		printf("[-] Process not found.");
		Sleep(1200);
		return EXIT_FAILURE;
	}

	attach_driver(pID);

	auto base_address = process_base_address_ex();

	if (!base_address)
	{
		printf("[-] Base not found.");
		Sleep(1200);
		return EXIT_FAILURE;
	}

	auto GameAssemblyBase = get_process_module_ex(L"GameAssembly.dll"); //read(+ oBaseNetworkable, DWORD64);	
	cout << ("[+] GameAssembly: ") << hex << GameAssemblyBase << endl;
	auto BaseNetworkable = ReadMemory<ULONG64>(GameAssemblyBase + 0x3646888);
	cout << ("[+] BaseNetworkable: ") << hex << BaseNetworkable << endl;

	bool status = is_driver_load_ex();

	printf("[+] Status: %d", status);

	cin.get();
	return EXIT_SUCCESS;
}