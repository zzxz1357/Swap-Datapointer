#pragma once

#ifdef _MSC_VER
#define _KEX_FORCEINLINE __forceinline
#else
#define _KEX_FORCEINLINE __attribute__((always_inline))
#endif

#ifndef KEX_DONT_INLINE
#define KEX_FORCEINLINE _KEX_FORCEINLINE
#else
#define KEX_FORCEINLINE inline
#endif

#define KEX_IMAGE_DIRECTORY_ENTRY_EXPORT 0

namespace kex
{
	namespace traits {
		template<class T> struct remove_reference { using type = T; };
		template<class T> struct remove_reference<T&> { using type = T; };
		template<class T> struct remove_reference<T&&> { using type = T; };
		template<class T> using remove_reference_t = typename remove_reference<T>::type;

		template<class T> struct remove_const { using type = T; };
		template<class T> struct remove_const<const T> { using type = T; };
		template<class T> using remove_const_t = typename remove_const<T>::type;
	}

	namespace cache {
		inline ULONG_PTR kernel_base = 0;
	}

	KEX_FORCEINLINE void init(ULONG_PTR base_address) {
		cache::kernel_base = base_address;
	}

	namespace literals {
		KEX_FORCEINLINE constexpr SIZE_T operator ""_KiB(SIZE_T num) { return num << 10; }
		KEX_FORCEINLINE constexpr SIZE_T operator ""_MiB(SIZE_T num) { return num << 20; }
		KEX_FORCEINLINE constexpr SIZE_T operator ""_GiB(SIZE_T num) { return num << 30; }
		KEX_FORCEINLINE constexpr SIZE_T operator ""_TiB(SIZE_T num) { return num << 40; }
	}
	using namespace literals;

	namespace hash {
		namespace detail {
			template <typename Size>
			struct fnv_constants;

			template <>
			struct fnv_constants<UINT32>
			{
				constexpr static UINT32 default_offset_basis = 0x811C9DC5UL;
				constexpr static UINT32 prime = 0x01000193UL;
			};

			template <>
			struct fnv_constants<UINT64>
			{
				constexpr static UINT64 default_offset_basis = 0xCBF29CE484222325ULL;
				constexpr static UINT64 prime = 0x100000001B3ULL;
			};

			template <typename Char>
			struct char_traits;

			template <>
			struct char_traits<char>
			{
				KEX_FORCEINLINE static constexpr char to_lower(char c) { return c | ' '; };
				KEX_FORCEINLINE static constexpr char to_upper(char c) { return c & '_'; };
				KEX_FORCEINLINE static constexpr char flip_case(char c) { return c ^ ' '; };
				KEX_FORCEINLINE static constexpr bool is_caps(char c) { return (c & ' ') == ' '; }
			};

			template <>
			struct char_traits<wchar_t>
			{
				KEX_FORCEINLINE static constexpr wchar_t to_lower(wchar_t c) { return c | L' '; };
				KEX_FORCEINLINE static constexpr wchar_t to_upper(wchar_t c) { return c & L'_'; };
				KEX_FORCEINLINE static constexpr wchar_t flip_case(wchar_t c) { return c ^ L' '; };
				KEX_FORCEINLINE static constexpr bool is_caps(wchar_t c) { return (c & L' ') == L' '; }
			};
		}

		template <typename Char> KEX_FORCEINLINE constexpr Char to_lower(Char c) { return detail::char_traits<Char>::to_lower(c); }
		template <typename Char> KEX_FORCEINLINE constexpr Char to_upper(Char c) { return detail::char_traits<Char>::to_upper(c); }
		template <typename Char> KEX_FORCEINLINE constexpr Char flip_case(Char c) { return detail::char_traits<Char>::flip_case(c); }

		template <typename Type, typename Char, bool ToLower = false>
		KEX_FORCEINLINE constexpr Type hash_fnv1a(const Char* str)
		{
			Type val = detail::fnv_constants<Type>::default_offset_basis;

			for (; *str != static_cast<Char>(0); ++str) {
				Char c = *str;

				if constexpr (ToLower)
					c = to_lower<Char>(c);

				val ^= static_cast<Type>(c);
				val *= static_cast<Type>(detail::fnv_constants<Type>::prime);
			}

			return val;
		}

		template <typename Type, Type Value>
		struct force_cx
		{
			constexpr static auto value = Value;
		};

#define _KEX_HASH_RTS(str) (::kex::hash::hash_fnv1a<UINT64, ::kex::traits::remove_const_t<::kex::traits::remove_reference_t<decltype(*(str))>>, false>((str)))
#define _KEX_HASH_RTS_TOLOWER(str) (::kex::hash::hash_fnv1a<UINT64, ::kex::traits::remove_const_t<::kex::traits::remove_reference_t<decltype(*(str))>>, true>((str)))

#define _KEX_HASH_STR(str) (::kex::hash::force_cx<UINT64, ::kex::hash::hash_fnv1a<UINT64, ::kex::traits::remove_const_t<::kex::traits::remove_reference_t<decltype(*(str))>>, false>((str))>::value)
#define _KEX_HASH_STR_TOLOWER(str) (::kex::hash::force_cx<UINT64, ::kex::hash::hash_fnv1a<UINT64, ::kex::traits::remove_const_t<::kex::traits::remove_reference_t<decltype(*(str))>>, true>((str))>::value)

#ifndef KEX_USE_TOLOWER
#define KEX_HASH_RTS(str) _KEX_HASH_RTS(str)
#define KEX_HASH_STR(str) _KEX_HASH_STR(str)
#else
#define KEX_HASH_RTS(str) _KEX_HASH_RTS_TOLOWER(str)
#define KEX_HASH_STR(str) _KEX_HASH_STR_TOLOWER(str)
#endif
	}

	namespace detail {
#pragma pack(push, 1)
		typedef struct _IMAGE_DOS_HEADER {
			UINT16 e_magic;
			UINT16 e_cblp;
			UINT16 e_cp;
			UINT16 e_crlc;
			UINT16 e_cparhdr;
			UINT16 e_minalloc;
			UINT16 e_maxalloc;
			UINT16 e_ss;
			UINT16 e_sp;
			UINT16 e_csum;
			UINT16 e_ip;
			UINT16 e_cs;
			UINT16 e_lfarlc;
			UINT16 e_ovno;
			UINT16 e_res[4];
			UINT16 e_oemid;
			UINT16 e_oeminfo;
			UINT16 e_res2[10];
			INT32  e_lfanew;
		} IMAGE_DOS_HEADER, * PIMAGE_DOS_HEADER;

		typedef struct _IMAGE_FILE_HEADER {
			UINT16 Machine;
			UINT16 NumberOfSections;
			UINT32 TimeDateStamp;
			UINT32 PointerToSymbolTable;
			UINT32 NumberOfSymbols;
			UINT16 SizeOfOptionalHeader;
			UINT16 Characteristics;
		} IMAGE_FILE_HEADER, * PIMAGE_FILE_HEADER;

		typedef struct _IMAGE_DATA_DIRECTORY {
			UINT32 VirtualAddress;
			UINT32 Size;
		} IMAGE_DATA_DIRECTORY, * PIMAGE_DATA_DIRECTORY;

		typedef struct _IMAGE_OPTIONAL_HEADER64 {
			UINT16 Magic;
			UINT8  MajorLinkerVersion;
			UINT8  MinorLinkerVersion;
			UINT32 SizeOfCode;
			UINT32 SizeOfInitializedData;
			UINT32 SizeOfUninitializedData;
			UINT32 AddressOfEntryPoint;
			UINT32 BaseOfCode;
			UINT64 ImageBase;
			UINT32 SectionAlignment;
			UINT32 FileAlignment;
			UINT16 MajorOperatingSystemVersion;
			UINT16 MinorOperatingSystemVersion;
			UINT16 MajorImageVersion;
			UINT16 MinorImageVersion;
			UINT16 MajorSubsystemVersion;
			UINT16 MinorSubsystemVersion;
			UINT32 Win32VersionValue;
			UINT32 SizeOfImage;
			UINT32 SizeOfHeaders;
			UINT32 CheckSum;
			UINT16 Subsystem;
			UINT16 DllCharacteristics;
			UINT64 SizeOfStackReserve;
			UINT64 SizeOfStackCommit;
			UINT64 SizeOfHeapReserve;
			UINT64 SizeOfHeapCommit;
			UINT32 LoaderFlags;
			UINT32 NumberOfRvaAndSizes;
			IMAGE_DATA_DIRECTORY DataDirectory[16];
		} IMAGE_OPTIONAL_HEADER64, * PIMAGE_OPTIONAL_HEADER64;

		typedef struct _IMAGE_NT_HEADERS64 {
			UINT32 Signature;
			IMAGE_FILE_HEADER FileHeader;
			IMAGE_OPTIONAL_HEADER64 OptionalHeader;
		} IMAGE_NT_HEADERS64, * PIMAGE_NT_HEADERS64;

		typedef struct _IMAGE_EXPORT_DIRECTORY {
			UINT32 Characteristics;
			UINT32 TimeDateStamp;
			UINT16 MajorVersion;
			UINT16 MinorVersion;
			UINT32 Name;
			UINT32 Base;
			UINT32 NumberOfFunctions;
			UINT32 NumberOfNames;
			UINT32 AddressOfFunctions;
			UINT32 AddressOfNames;
			UINT32 AddressOfNameOrdinals;
		} IMAGE_EXPORT_DIRECTORY, * PIMAGE_EXPORT_DIRECTORY;
#pragma pack(pop)
	}

	template <UINT64 ExportHash>
	KEX_FORCEINLINE ULONG_PTR find_kernel_export()
	{
		if (!cache::kernel_base) {
			// kex::init(base_address) must be called first!
			return 0;
		}

		const auto dos_header = (detail::PIMAGE_DOS_HEADER)cache::kernel_base;
		const auto nt_headers = (detail::PIMAGE_NT_HEADERS64)(cache::kernel_base + dos_header->e_lfanew);
		const auto export_directory = (detail::PIMAGE_EXPORT_DIRECTORY)(cache::kernel_base +
			nt_headers->OptionalHeader.DataDirectory[KEX_IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

		const auto address_of_functions = (UINT32*)(cache::kernel_base + export_directory->AddressOfFunctions);
		const auto address_of_names = (UINT32*)(cache::kernel_base + export_directory->AddressOfNames);
		const auto address_of_name_ordinals = (UINT16*)(cache::kernel_base + export_directory->AddressOfNameOrdinals);

		for (UINT32 i = 0; i < export_directory->NumberOfNames; ++i)
		{
			const auto export_entry_name = (char*)(cache::kernel_base + address_of_names[i]);
			const auto export_entry_hash = KEX_HASH_RTS(export_entry_name);

			if (export_entry_hash == ExportHash)
			{
				return cache::kernel_base + address_of_functions[address_of_name_ordinals[i]];
			}
		}

		return 0;
	}

	template <UINT64 ExportHash>
	KEX_FORCEINLINE ULONG_PTR find_kernel_export_cached()
	{
		static ULONG_PTR address = 0;
		if (!address)
			address = find_kernel_export<ExportHash>();

		return address;
	}
}

#define kex_import(name) ((decltype(&name))(::kex::find_kernel_export_cached<KEX_HASH_STR(#name)>()))