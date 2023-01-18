#pragma once
#include <Windows.h>
#include <cstdint>

///////////////////////////////////////////////////////////
enum comm_code
{
	is_driver_load,
	read_mem_phys,
	write_mem_phys,
	process_base_address,
	get_async_key_state,
	mouse_input,
	change_protect_window,
	set_process_id,
	send_buffer,
	get_buffer,
	get_peb_address,
	get_thread_context,
	set_thread_context,
	get_guarded_region,
	read_kernel_memory
};

typedef struct operation_data
{
	/*communication*/
	comm_code comm_code;
	DWORD comm_key;
	/*communication*/

	/*status code*/
	int64_t status_code;
	/*status code*/

	/*is driver load*/
	ULONG load_code;
	/*is driver load*/

	/*read mem phys*/
	ULONG rmp_pid;
	PVOID rmp_address;
	PVOID rmp_buffer;
	SIZE_T rmp_size;
	/*read mem phys*/

	/*write mem phys*/
	ULONG wmp_pid;
	PVOID wmp_address;
	PVOID wmp_buffer;
	SIZE_T wmp_size;
	/*write mem phys*/

	/*process base address*/
	ULONG pba_pid;
	PVOID pba_out_address;
	/*process base address*/

	/*get async key state*/
	uint8_t gaks_vk_code;
	bool gaks_is_key;
	/*get async key state*/

	/*mouse input*/
	LONG mi_x;
	LONG mi_y;
	USHORT mi_button_flags;
	/*mouse input*/

	/*change protect window*/
	ULONG cpw_value;
	ULONGLONG cpw_window_handle;
	/*change protect window*/

	/*set process id*/
	ULONG spi_current_pid;
	ULONG spi_new_pid;
	/*set process id*/

	/*send buffer*/
	ULONG sb_index;
	ULONGLONG sb_in_buffer1;
	ULONGLONG sb_in_buffer2;
	/*send buffer*/

	/*get buffer*/
	ULONG gb_index;
	ULONGLONG gb_out_buffer1;
	ULONGLONG gb_out_buffer2;
	/*get buffer*/

	/*get peb address*/
	ULONG gpa_pid;
	PVOID gpa_address;
	int32_t gpa_iswow64;
	/*get peb address*/

	/*get thread context*/
	ULONGLONG gtc_out_put;
	ULONGLONG gtc_window_handle;
	/*get thread context*/

	/*set thread context*/
	ULONGLONG stc_window_handle;
	ULONGLONG stc_thread_context;
	/*set thread context*/

	/*get guarded region*/
	ULONGLONG ggr_guarded_region;
	/*get guarded region*/

	/*read kernel memory*/
	PVOID rkm_address;
	PVOID rkm_buffer;
	ULONG rkm_size;
	/*read kernel memory*/
};
///////////////////////////////////////////////////////////