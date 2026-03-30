#include <ntifs.h>
#include <windef.h>
#include <cstdint>
#include <intrin.h>
#include <ntimage.h>
#include <ntddk.h>

#include "kernel/structures.h"
#include "skCrypter/skCrypter.h"

#include "impl/comm/interface.h"

#include "impl/kex.h"
#include "impl/modules.h"
#include "impl/invoke.h"



_declspec(noinline) auto initialize_base() -> NTSTATUS
{
	const auto ntoskrnl_image = modules::get_ntos_base_address();
	if (!ntoskrnl_image) {
		return STATUS_UNSUCCESSFUL;
	}

	kex::init(ntoskrnl_image);
}

__declspec(noinline) auto initialize_hook() -> NTSTATUS
{
	const auto win32kbase_image = modules::get_kernel_module(MODULE_NAME);
	if (!win32kbase_image)
	{
		return STATUS_NOT_FOUND;
	}

	const auto export_addr = modules::get_kernel_export(win32kbase_image, FUNCTION_NAME);
	if (!export_addr)
	{
		return STATUS_PROCEDURE_NOT_FOUND;
	}

	// call    cs:__imp_W32GetWin32kBaseApiSetTable
	const auto target_instruction = export_addr + 0x366;

	// .idata:00000000002C1158 ; __int64 __fastcall W32GetWin32kBaseApiSetTable(_QWORD, _QWORD)
	// .idata:00000000002C1158                 extrn __imp_W32GetWin32kBaseApiSetTable : qword
	// .idata : 00000000002C1158; CODE XREF : NtGdiDeleteObjectApp:loc_138CC↑p
	// .idata : 00000000002C1158; HANDLELOCK::vUnlockAndRelease(void) :loc_1A8EB↑p ...
	const auto target_qword_addr = modules::resolve_address(target_instruction, 3, 7);
	if (!target_qword_addr)
	{
		return STATUS_INVALID_ADDRESS;
	}

	invoke::original_function = reinterpret_cast<invoke::pfn_qword>(
		InterlockedExchangePointer(
			reinterpret_cast<PVOID*>(target_qword_addr),
			reinterpret_cast<PVOID>(&invoke::hooked_function)
		)
		);

	return STATUS_SUCCESS;
}

// Use Manual Mapper for driver loading like KdMapper.
extern "C" NTSTATUS driver_entry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	safe_print(skCrypt("[debug] driver_entry() called!\n"));

	NTSTATUS status = initialize_base();
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	status = initialize_hook();
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	safe_print(skCrypt("[debug] driver_entry() finished!\n"));

	return STATUS_SUCCESS;
}