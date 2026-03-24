#pragma once

namespace invoke
{
	using pfn_qword = uint64_t(__fastcall*)(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4);
	inline pfn_qword original_qword = nullptr;

	__int64 __fastcall hooked_qword(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4)
	{
		if (!a1 || ExGetPreviousMode() != UserMode)
		{
			// !a1 || ExGetPreviousMode() != UserMode fail
			return 0;
		}

	}
}