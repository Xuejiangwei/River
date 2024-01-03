#pragma once

//#ifdef _DEBUG
//#define _ASSERT(expression) assert(expression)
//#else
//#define _ASSERT(expression)
//#endif

#ifdef RIVER_EXPORT
	#define RIVER_API __declspec(dllexport)
#else
	#define RIVER_API __declspec(dllimport)
#endif // RIVER_API
  

template<typename T>
using Share = std::shared_ptr<T>;

template<typename T, typename... _Types>
Share<T> MakeShare(_Types&&... _Args)
{
	return std::make_shared<T>(std::forward<_Types>(_Args)...);
}

template<typename T>
using Unique = std::unique_ptr<T>;

template<typename T, typename... _Types>
Unique<T> MakeUnique(_Types&&... _Args)
{
	return std::make_unique<T>(std::forward<_Types>(_Args)...);
}

using String = std::string;

template<typename T>
using V_Array = std::vector<T>;

template<typename T>
using List = std::list<T>;

template<typename T>
using Set = std::set<T>;

template<class _Kty, class _Ty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>>
using HashMap = std::unordered_map<_Kty, _Ty, _Hasher, _Keyeq>;

template<class _Kty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>>
using HashSet = std::unordered_set<_Kty, _Hasher, _Keyeq>;

template <class _Kty, class _Ty, class _Pr = std::less<_Kty>>
using LessRBTreeMap = std::map<_Kty, _Ty, _Pr>;

template <class _Kty, class _Ty, class _Pr = std::greater<_Kty>>
using GreaterRBTreeMap = std::map<_Kty, _Ty, _Pr>;

template<class Ty1, class Ty2>
using Pair = std::pair<Ty1, Ty2>;

template<typename T>
using Func = std::function<T>;

namespace River
{
	template <class _Ty>
		struct remove_reference {
		using type = _Ty;
		using _Const_thru_ref_type = const _Ty;
	};

	template <class _Ty>
	struct remove_reference<_Ty&> {
		using type = _Ty;
		using _Const_thru_ref_type = const _Ty&;
	};

	template <class _Ty>
	struct remove_reference<_Ty&&> {
		using type = _Ty;
		using _Const_thru_ref_type = const _Ty&&;
	};

	template <class _Ty>
	using remove_reference_t = typename remove_reference<_Ty>::type;

	template <class _Ty>
	constexpr remove_reference_t<_Ty>&& Move(_Ty&& _Arg) noexcept
	{
		return static_cast<remove_reference_t<_Ty>&&>(_Arg);
	}
}

using BitArray = std::vector<bool>;

using int8 = char;
using uint8 = unsigned char;
using int16 = short;
using uint16 = unsigned short;
using uint32 = unsigned int;
using int64 = long long;
using uint64 = unsigned long long;
using float64 = double;
