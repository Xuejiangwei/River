#pragma once

#ifdef _DEBUG
#define GP_ASSERT(expression) assert(expression)
#else
#define GP_ASSERT(expression)
#endif

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

template<class _Kty, class _Ty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>>
using HashMap = std::unordered_map<_Kty, _Ty, _Hasher, _Keyeq>;

template <class _Kty, class _Ty, class _Pr = std::less<_Kty>>
using LessRBTreeMap = std::map<_Kty, _Ty, _Pr>;

template <class _Kty, class _Ty, class _Pr = std::greater<_Kty>>
using GreaterRBTreeMap = std::map<_Kty, _Ty, _Pr>;

namespace River
{
	template <class _Ty>
	struct remove_reference {
		using type = _Ty;
		using _Const_thru_ref_type = const _Ty;
	};

	template <class _Ty>
	using remove_reference_t = typename remove_reference<_Ty>::type;

	template <class _Ty>
	constexpr remove_reference_t<_Ty>&& Move(_Ty&& _Arg) noexcept
	{
		return static_cast<remove_reference_t<_Ty>&&>(_Arg);
	}
}