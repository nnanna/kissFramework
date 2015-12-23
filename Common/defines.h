
#ifndef KS_DEFINES_H
#define KS_DEFINES_H

#define NAME_MAX		64

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

#define ks_max(a, b)	(a) > (b) ? (a) : (b)
#define ks_min(a, b)	(a) < (b) ? (a) : (b)


#if (defined _MSC_VER)
#define ks_thread_local		__declspec( thread )
#else
#define ks_thread_local		__thread
#endif // _MSC_VER


typedef size_t			ksType;
typedef unsigned int	ksU32;
typedef ksU32			u32;
typedef int				ks32;
typedef float			f32;
typedef const f32		constf32;

#if _DEBUG
	#define if_ks_dbg	if
#else
	#define if_ks_dbg()
#endif

namespace ks {

	template<typename T> struct strip_ref			{ typedef T type; };

	template<typename T> struct strip_ref < T& >	{ typedef T type; };
	template<typename T> struct strip_ref < T&& >	{ typedef T type; };

	template <typename T>
	typename strip_ref<T>::type&& move(T&& arg)		{ return static_cast<typename strip_ref<T>::type&&>(arg); }


	template<typename T> struct strip_qualifiers			{ typedef T Type; };
	template<typename T> struct strip_qualifiers<T&>		{ typedef T Type; };
	template<typename T> struct strip_qualifiers<const T>	{ typedef T Type; };
	template<typename T> struct strip_qualifiers<const T&>	{ typedef T Type; };
	template<typename T> struct strip_qualifiers<const T*>	{ typedef T* Type; };

}


#endif // KS_DEFINES_H