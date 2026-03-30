#pragma once
typedef unsigned long long ull;
typedef ull uint64;
#define _QWORD uint64

namespace invoke
{
	// .idata:00000000002C1158 ; __int64 __fastcall W32GetWin32kBaseApiSetTable(_QWORD, _QWORD)
	using pfn_qword = uint64_t(__fastcall*)(_QWORD a1, _QWORD a2);
	inline pfn_qword original_function = nullptr;

	// a1, a2 will be passed in the desired flow if, 
	// when calling the NtGdiPolyPolyDraw function in user mode, a5 is set to 2.
	//if (a5 == 2)
	//{
	//	v24 = *(__int64 (**)(void))(*(_QWORD*)(W32GetWin32kBaseApiSetTable(a1, a2) + 24) + 2200LL);
	__int64 __fastcall hooked_function(_QWORD a1, _QWORD a2)
	{
		auto buffer = reinterpret_cast<invoke_data*>(a1);

		if (buffer->magic_number != MAGIC_NUMBER) {
			return original_function(a1, a2);
		}

		switch (buffer->code)
		{
			case invoke_read:
			{

			}
			case invoke_write:
			{

			}
		}
	}
}