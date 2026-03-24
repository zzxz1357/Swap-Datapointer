#pragma once

#include <ntstrsafe.h> // RtlStringCchVPrintfA
#include <stdarg.h>    // va_list

inline void safe_print(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	CHAR buffer[512];

	NTSTATUS status = RtlStringCchVPrintfA(buffer, RTL_NUMBER_OF(buffer), fmt, args);

	if (NT_SUCCESS(status))
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%s", buffer);
	}
	else
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[safe_print] String format failed! (0x%08X)\n", status);
	}

	va_end(args);
}

#define to_lower_i(Char) ((Char >= 'A' && Char <= 'Z') ? (Char + 32) : Char)
#define to_lower_c(Char) ((Char >= (char*)'A' && Char <= (char*)'Z') ? (Char + 32) : Char)

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR  FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

typedef enum _SYSTEM_INFORMATION_CLASS
{
	SystemBasicInformation,
	SystemProcessorInformation,
	SystemPerformanceInformation,
	SystemTimeOfDayInformation,
	SystemPathInformation,
	SystemProcessInformation,
	SystemCallCountInformation,
	SystemDeviceInformation,
	SystemProcessorPerformanceInformation,
	SystemFlagsInformation,
	SystemCallTimeInformation,
	SystemModuleInformation,
	SystemLocksInformation,
	SystemStackTraceInformation,
	SystemPagedPoolInformation,
	SystemNonPagedPoolInformation,
	SystemHandleInformation,
	SystemObjectInformation,
	SystemPageFileInformation,
	SystemVdmInstemulInformation,
	SystemVdmBopInformation,
	SystemFileCacheInformation,
	SystemPoolTagInformation,
	SystemInterruptInformation,
	SystemDpcBehaviorInformation,
	SystemFullMemoryInformation,
	SystemLoadGdiDriverInformation,
	SystemUnloadGdiDriverInformation,
	SystemTimeAdjustmentInformation,
	SystemSummaryMemoryInformation,
	SystemNextEventIdInformation,
	SystemEventIdsInformation,
	SystemCrashDumpInformation,
	SystemExceptionInformation,
	SystemCrashDumpStateInformation,
	SystemKernelDebuggerInformation,
	SystemContextSwitchInformation,
	SystemRegistryQuotaInformation,
	SystemExtendServiceTableInformation,
	SystemPrioritySeperation,
	SystemPlugPlayBusInformation,
	SystemDockInformation,
	SystemProcessorSpeedInformation,
	SystemCurrentTimeZoneInformation,
	SystemLookasideInformation,
	SystemBigPoolInformation = 0x42
} SYSTEM_INFORMATION_CLASS, * PSYSTEM_INFORMATION_CLASS;

extern "C"
{
	NTSYSAPI
		PIMAGE_NT_HEADERS
		NTAPI
		RtlImageNtHeader(
			IN PVOID   ModuleAddress);

	NTSTATUS
		ZwProtectVirtualMemory(
			IN HANDLE ProcessHandle,
			IN OUT PVOID* BaseAddress,
			IN OUT SIZE_T* NumberOfBytesToProtect,
			IN ULONG NewAccessProtection,
			OUT PULONG OldAccessProtection);

	NTKERNELAPI
		PVOID
		PsGetProcessSectionBaseAddress(
			PEPROCESS Process
		);

	NTSYSCALLAPI
		NTSTATUS
		NTAPI
		ZwQuerySystemInformation(
			ULONG InfoClass,
			PVOID Buffer,
			ULONG Length,
			PULONG ReturnLength
		);

	NTSYSCALLAPI
		NTSTATUS NTAPI ExRaiseHardError(
			NTSTATUS ErrorStatus,
			ULONG NumberOfParameters,
			ULONG UnicodeStringParameterMask,
			PULONG_PTR Parameters,
			ULONG ValidResponseOptions,
			PULONG Response
		);

	NTSYSCALLAPI
		NTKERNELAPI
		PVOID
		NTAPI
		PsGetProcessWow64Process(_In_ PEPROCESS Process);

	NTSYSCALLAPI
		NTSTATUS NTAPI MmCopyVirtualMemory
		(
			PEPROCESS SourceProcess,
			PVOID SourceAddress,
			PEPROCESS TargetProcess,
			PVOID TargetAddress,
			SIZE_T BufferSize,
			KPROCESSOR_MODE PreviousMode,
			PSIZE_T ReturnSize
		);

	NTKERNELAPI PVOID NTAPI
		PsGetCurrentThreadWin32Thread(
			VOID
		);

	NTKERNELAPI
		PPEB
		NTAPI
		PsGetProcessPeb(
			IN PEPROCESS Process);

	NTSYSAPI
		NTSTATUS
		NTAPI
		ObReferenceObjectByName(
			_In_ PUNICODE_STRING ObjectName,
			_In_ ULONG Attributes,
			_In_opt_ PACCESS_STATE AccessState,
			_In_opt_ ACCESS_MASK DesiredAccess,
			_In_ POBJECT_TYPE ObjectType,
			_In_ KPROCESSOR_MODE AccessMode,
			_Inout_opt_ PVOID ParseContext,
			_Out_ PVOID* Object
		);

	ULONG
		NTAPI
		KeCapturePersistentThreadState(
			IN PCONTEXT Context,
			IN PKTHREAD Thread,
			IN ULONG BugCheckCode,
			IN ULONG BugCheckParameter1,
			IN ULONG BugCheckParameter2,
			IN ULONG BugCheckParameter3,
			IN ULONG BugCheckParameter4,
			OUT PVOID VirtualAddress
		);
}