#pragma once
#include <type_traits>
#include <tuple>
#include <vector>

#define DECLARE_DELEGATE(DelegateName, ...) typedef Delegate<void, __VA_ARGS__> DelegateName

#define DECLARE_DELEGATE_Ret(Ret, DelegateName, ...) typedef Delegate<Ret, __VA_ARGS__> DelegateName

#define DECLARE_MULTICAST_DELEGATE(DelegateName, ...) typedef MulticasDelegate<__VA_ARGS__> DelegateName


namespace TMP
{
	template<typename... Args>
	using void_t = void;

	template<typename T, typename U = void>
	struct is_operator
	{
		static constexpr bool value = false;
	};
	template<typename T>
	struct is_operator<T, void_t<decltype(std::declval<T>()())>>
	{
		static constexpr bool value = true;
	};
	template<typename T>
	constexpr bool is_operator_v = is_operator<T>::value;
}

template<typename Ret, typename... Args>
class IDelegate
{
public:
	virtual ~IDelegate(){}

	virtual Ret Execute(Args... args) = 0;
};

template<bool bConst, class C, class Ret, typename... Args>
struct MemberFuncPtr;
template<class C, class Ret, typename... Params, typename... Payload>
struct MemberFuncPtr<false, C, Ret( Params...), Payload...>
{
	using type = Ret(C::*)(Params..., Payload...);
};
template<class C, class Ret, typename... Params, typename... Payload>
struct MemberFuncPtr<true, C, Ret(Params...), Payload...>
{
	using type = Ret(C::*)(Params..., Payload...) const;
};

template<bool bConst, class C, class Ret, typename... Params>
class MemberDelegate;
template<bool bConst, class C, class Ret, typename... Params, typename... Payload>
class MemberDelegate<bConst, C, Ret(Params...), Payload...> : IDelegate<Ret, Params...>
{
public:
	using FuncPtr = MemberFuncPtr<bConst, C, Ret, Params..., Payload...>::type;
	MemberDelegate(C* p, FuncPtr func, Payload... payload) : m_p(p), CallBack(func), m_payload(std::make_tuple(payload...)) {}

	virtual Ret Execute(Params... params) override
	{
		return Execute_Internal(std::forward<Params>(params)..., std::index_sequence_for<Payload...>());
	}

private:
	template<std::size_t... Indices>
	Ret Execute_Internal(Params... params, std::index_sequence<Indices...>)
	{
		return (m_p->*CallBack)(std::forward<Params>(params)..., std::get<Indices>(m_payload)...);
	}
	
	C* m_p;
	FuncPtr CallBack;
	std::tuple<Payload...> m_payload;
};

template<bool bConst, class Ret, typename... Params>
class StaticDelegate;
template<bool bConst, class Ret, typename... Params, typename... Payload>
class StaticDelegate<bConst, Ret(Params...), Payload...> : public IDelegate<Ret, Params...>
{
public:
	StaticDelegate(Ret(*func)(Params..., Payload...), Payload... payload) : CallBack(func), m_payload(std::make_tuple(payload...)){}

	virtual Ret Execute(Params... params) override
	{
		return Execute_Internal(std::forward<Params>(params)..., std::index_sequence_for<Payload...>());
	}

private:
	template<std::size_t... Indices>
	Ret Execute_Internal(Params... params, std::index_sequence<Indices...>)
	{
		return CallBack(std::forward<Params>(params)..., std::get<Indices>(m_payload)...);
	}

	Ret(*CallBack)(Params..., Payload...);
	std::tuple<Payload...> m_payload;
};

template<typename TLambda, class Ret, typename... Params>
class LambdaDelegate;
template<typename TLambda, class Ret, typename... Params, typename... Payload>
class LambdaDelegate<TLambda, Ret(Params...), Payload...> : public IDelegate<Ret, Params...>
{
public:
	LambdaDelegate(TLambda Lambda, Payload... payload) : m_Lambda(Lambda), m_payload(std::make_tuple(payload...)) {}

	virtual Ret Execute(Params... params) override
	{
		return Execute_Internal(std::forward<Params>(params)..., std::index_sequence_for<Payload...>());
	}
private:
	template<std::size_t... Indices>
	Ret Execute_Internal(Params... params, std::index_sequence<Indices...>)
	{
		return (Ret)(m_Lambda(std::forward<Params>(params)..., std::get<Indices>(m_payload)...));
	}

	TLambda m_Lambda;
	std::tuple<Payload...> m_payload;
};

template<class Ret, typename... Params>
class Delegate
{
public:
	Delegate(){}
	~Delegate(){}
	
	template<class C, typename... Payload>
	void BindMember(C* pObject, Ret(C::* FuncPtr)(Params..., Payload...), Payload...payload)
	{
		CreateMember(pObject, FuncPtr, std::forward<Payload>(payload)...);
	}
	
	template<class C, typename... Payload>
	void BindMember(C* pObject, Ret(C::* FuncPtr)(Params..., Payload...) const, Payload...payload)
	{
		CreateMember(pObject, FuncPtr, std::forward<Payload>(payload)...);
	}
	
	template<typename... Payload>
	void BindStatic(Ret(*FuncPtr)(Params..., Payload...), Payload...payload)
	{
		CreateStatic(FuncPtr, std::forward<Payload>(payload)...);
	}

	template<typename TLambda, typename... Payload>
	void BindLambda(TLambda Lambda, Payload... payload)
	{
		CreateLambda(Lambda, std::forward<Payload>(payload)...);
	}

	bool IsBound()
	{
		if (p)
		{
			return true;
		}
		return false;
	}

	Ret Execute(Params... params)
	{
		p->Execute(std::forward<Params>(params)...);
	}

	Ret ExecuteIfBound(Params... params)
	{
		if (IsBound())
		{
			return Execute(std::forward<Params>(params)...);
		}
	}

private:
	template<class C, typename... Payload>
	void CreateMember(C* pObject, Ret(C::*FuncPtr)(Params..., Payload...), Payload... payload)
	{
		p = new MemberDelegate<false, C, Ret(Params...), Payload...>(pObject, FuncPtr, std::forward<Payload>(payload)...);
	}

	template<class C, typename... Payload>
	void CreateMember(C* pObject, Ret(C::*FuncPtr)(Params..., Payload...) const, Payload... payload)
	{
		p = new MemberDelegate<true, C, Ret(Params...), Payload...>(pObject, FuncPtr, std::forward<Payload>(payload)...);
	}

	template<typename... Payload>
	void CreateStatic(Ret(*FuncPtr)(Params..., Payload...), Payload... payload)
	{
		p = new StaticDelegate<false, Ret(Params...), Payload...>(FuncPtr, std::forward<Payload>(payload)...);
	}

	template<typename TLambda, typename... Payload>
	void CreateLambda(TLambda Lambda, Payload... payload)
	{
		p = new LambdaDelegate<TLambda, Ret(Params...), Payload...>(Lambda, std::forward<Payload>(payload)...);
	}

	IDelegate<Ret, Params...>* p;
};

//多播代理,顺序执行,多播代理没有返回值,都是void
template<typename... ParamsType>
class MulticasDelegate
{
public:
	MulticasDelegate() {}
	~MulticasDelegate() {}

	bool IsNotEmpty()
	{
		if (DelegateList.size() > 0)
		{
			return true;
		}
		return false;
	}

	//使用Delegate的绑定函数进行单个代理的绑定

	//绑定类的成员函数
	template<class C, typename... Payload>
	inline void AddMember(C* pObject, void(C::* FuncPtr)(ParamsType..., Payload...), Payload...payload)
	{
		Delegate<void, ParamsType...> delegate;
		delegate.BindMember(pObject, FuncPtr, payload...);
		DelegateList.emplace_back(delegate);
	}
	//绑定类的静态函数和全局函数
	template<typename... Payload>
	inline void AddStatic(void(*FuncPtr)(ParamsType..., Payload...), Payload...payload)
	{
		Delegate<void, ParamsType...> delegate;
		delegate.BindStatic(FuncPtr, payload...);
		DelegateList.emplace_back(delegate);
	}

	//绑定Lambda表达式
	template<class TLambda, typename... Payload>
	inline void AddLambda(TLambda Lambda, Payload... payload)
	{
		Delegate<void, ParamsType...> delegate;
		delegate.BindLambda(Lambda, payload...);
		DelegateList.emplace_back(delegate);
	}

	//广播,顺序执行
	inline void Broadcast(ParamsType... args)
	{
		for (auto myDelegate : DelegateList)
		{
			myDelegate.ExecuteIfBound(args...);
		}
	}
private:
	std::vector<Delegate<void, ParamsType...>> DelegateList; //代理数组
};