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

template<typename T>
using Vector = std::vector<T>;

using String = std::string;
