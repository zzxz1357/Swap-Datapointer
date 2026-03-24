#include <ntifs.h>
#include <windef.h>
#include <cstdint>
#include <intrin.h>
#include <ntimage.h>
#include <ntddk.h>

#include "kernel/structures.h"
#include "skCrypter/skCrypter.h"

#include "impl/kex.h"
#include "impl/modules.h"
#include "impl/invoke.h"

_declspec(noinline) auto initialize_base() -> NTSTATUS
{
	const auto ntoskrnl_image = modules::get_ntos_base_address();
	if (!ntoskrnl_image) {
		safe_print(skCrypt("[debug] Failed to find ntoskrnl base address!\n"));
		return STATUS_UNSUCCESSFUL;
	}

	kex::init(ntoskrnl_image);
}

__declspec(noinline) auto initialize_hook() -> NTSTATUS
{
	const auto win32kbase_image = modules::get_kernel_module(skCrypt("win32kbase.sys"));
	if (!win32kbase_image)
	{
		safe_print(skCrypt("[-] Failed to find win32kbase.sys base address!\n"));
		return STATUS_NOT_FOUND;
	}

	const auto export_addr = modules::get_kernel_export(win32kbase_image, skCrypt("NtGdiPolyPolyDraw"));
	if (!export_addr)
	{
		safe_print(skCrypt("[-] Failed to resolve NtGdiPolyPolyDraw!\n"));
		return STATUS_PROCEDURE_NOT_FOUND;
	}

	const auto target_instruction = export_addr + 0x366;

	const auto target_qword_addr = modules::resolve_address(target_instruction, 3, 7);
	if (!target_qword_addr)
	{
		safe_print(skCrypt("[-] Failed to resolve target data pointer!\n"));
		return STATUS_INVALID_ADDRESS;
	}

	invoke::original_qword = reinterpret_cast<invoke::pfn_qword>(
		InterlockedExchangePointer(
			reinterpret_cast<PVOID*>(target_qword_addr),
			reinterpret_cast<PVOID>(&invoke::hooked_qword)
		)
		);

	safe_print(skCrypt("[+] Hook successfully installed on NtGdiPolyPolyDraw.\n"));
	return STATUS_SUCCESS;
}


extern "C" NTSTATUS driver_entry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	safe_print(skCrypt("[debug] driver_entry() called!\n"));

	NTSTATUS status = initialize_base();
	if (!NT_SUCCESS(status))
	{
		safe_print(skCrypt("[-] Failed to initialize base address! (0x%08X)\n"), status);
		return status;
	}

	status = initialize_hook();
	if (!NT_SUCCESS(status))
	{
		safe_print(skCrypt("[-] Failed to initialize hook! (0x%08X)\n"), status);
		return status;
	}

	return STATUS_SUCCESS;
}