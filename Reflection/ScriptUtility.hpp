//////////////////////////////////////////////////////////////////////////
///
///	@author		Nnanna Kama
///	@date		29/10/2013
///	@brief		Header implementation of Templated methods.
///	@remarks	NEVER INCLUDE THIS IN A HEADER FILE, DON'T BE CRAZY!
///
///
//////////////////////////////////////////////////////////////////////////

#ifndef SCRIPT_UTILITY_HPP
#define SCRIPT_UTILITY_HPP

#include "ScriptUtility.h"
#include <Reflection\TypeUID.h>
#include <Reflection\DataProperty.hpp>
#include <algorithm>


#define MAX_EXPORT_METHODS		200			// can generally be any number high enough but lower than default iOS recursive template limit


namespace ks {
	
//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

template<typename T, typename KEY>
void sortPtrMapIntoArrayByOffsets( Hash_map<KEY, T*>& pMap, Array<T*>& pArray )
{
	pArray.clear();

	for ( auto itr = pMap.begin(); itr != pMap.end(); ++itr )
	{
		pArray.push_back( itr->second );
	}

	static auto offset_sorter = [](const T* a, const T* b) -> bool
	{ 
		return a->sort_index() < b->sort_index();
	};

	std::sort( pArray.begin(), pArray.end(), offset_sorter );
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
ksU32 getKey( const char* pText )
{
	return CRC32(pText);
}

//////////////////////////////////////////////////////////////////////////
// MethodAccessorWrapper
//////////////////////////////////////////////////////////////////////////

template<class T, typename PARAM>
void MethodAccessorWrapper<T, PARAM>::Set( void* pClassInstance, DataProperty& data ) const
{
	auto object( static_cast<T*>(pClassInstance) );
	if (mSetMode == access_none)
	{
		KS_ASSERT(mGetMode == access_ref);
		PARAM& val	= (object->*refGetter)();
		data = val;
	}
	else if(mSetMode == access_DataProperty)
	{
		(object->*stackSetter)(data);
	}
	else
	{
		PARAM val;
		data = val;
		if(mSetMode == access_const_ref)
			(object->*constSetter)( val );
		else
			(object->*setter2)( val );
	}
}

//template<class T, typename PARAM>
//void MethodAccessorWrapper<T, PARAM>::Set( void* pClassInstance, const DataProperty& pVal ) const
//{
//	auto object( static_cast<T*>(pClassInstance) );
//	switch (mSetMode)
//	{
//	case access_none:
//		{
//			KS_ASSERT(mGetMode == access_ref);
//			PARAM& val	= (object->*refGetter)();
//			pVal.getValue( val );
//		}
//		break;
//
//	case access_DataProperty:
//		{
//			KS_ASSERT(!"unsupported!");
//		}
//		break;
//
//	case access_const_ref:
//		(object->*constSetter)( pVal.getValue<PARAM>() );
//		break;
//
//	case access_default:
//		(object->*setter2)( pVal.getValue<PARAM>() );
//		break;
//
//	default:
//		break;
//	}
//
//}

template<class T, typename PARAM>
void MethodAccessorWrapper<T, PARAM>::Get( void* pClassInstance, DataProperty& data ) const
{
	auto object( static_cast<T*>(pClassInstance) );
	switch (mGetMode)
	{
	case access_const_ref:
		{
			const PARAM& val	= (object->*constRefGetter)();
			PARAM* val_ref		= const_cast<PARAM*>( &val );
			data = val_ref;
		}
		break;

	case access_const:
		{
			PARAM val = (object->*constGetter)();
			data = val;
		}
		break;

	case access_ref:
		{
			PARAM& val	= (object->*refGetter)();
			data = val;
		}
		break;

	case access_default:
		{
			PARAM val = (object->*getter)();
			data = val;
		}
		break;

	case access_DataProperty:
		{
			KS_ASSERT(!"unsupported!");
		}
		break;

	case access_none:
	default:
		break;
	}
}

//template<class T, typename PARAM>
//void MethodAccessorWrapper<T, PARAM>::Get( void* pClassInstance, DataProperty& pResult ) const
//{
//	auto object( static_cast<T*>(pClassInstance) );
//	switch (mGetMode)
//	{
//	case access_const_ref:
//		pResult = (object->*constRefGetter)();
//		break;
//
//	case access_const:
//		pResult = (object->*constGetter)();
//		break;
//
//	case access_ref:
//		pResult = (object->*refGetter)();
//		break;
//
//	case access_default:
//		pResult = (object->*getter)();
//		break;
//
//	case access_DataProperty:
//		{
//			KS_ASSERT(!"unsupported!");
//		}
//		break;
//
//	case access_none:
//	default:
//		break;
//	}
//}


template<typename T> struct pointer_type			{ typedef T Type; };
template<typename T> struct pointer_type<T&>		{ typedef T* Type; };
template<typename T> struct pointer_type<const T>	{ typedef T* Type; };
template<typename T> struct pointer_type<const T&>	{ typedef T* Type; };
template<typename T> struct pointer_type<const T*>	{ typedef T* Type; };

template<typename T> struct to_pointer
{
	typedef typename pointer_type<T>::Type		ArgType;

	to_pointer( T& pObj ) : Pointer(&pObj)		{}
	ArgType Pointer;
};
template<typename T> struct to_pointer<T*>
{
	typedef typename pointer_type<T>::Type		ArgType;

	to_pointer( T* pObj ) : Pointer(pObj)		{}
	ArgType Pointer;
};


//////////////////////////////////////////////////////////////////////////
// MethodAccessorWrapper
//////////////////////////////////////////////////////////////////////////
template<typename T>
const IStringifier* getDefaultIfNull(const IStringifier* pPrinter)		{ return pPrinter /*? pPrinter : &GenericStringifier<T>::GetDefault()*/; }

template<class T, typename PARAM>
MethodAccessorWrapper<T, PARAM>::MethodAccessorWrapper( ksU32 pIdx, ConstRefGetter pGetter, ConstRefdSetter pSetter, const char* pName, const IStringifier* pPrinter ) :
	attrib_index(pIdx),
	constRefGetter(pGetter),
	constSetter(pSetter),
	printer(getDefaultIfNull<PARAM>(pPrinter)),
	name(pName),
	mSetMode( access_const_ref ),
	mGetMode(access_const_ref)
{}

template<class T, typename PARAM>
MethodAccessorWrapper<T, PARAM>::MethodAccessorWrapper( ksU32 pIdx, ConstGetter pGetter, Setter pSetter, const char* pName, const IStringifier* pPrinter ) :
	attrib_index(pIdx),
	constGetter(pGetter),
	setter2(pSetter),
	printer(getDefaultIfNull<PARAM>(pPrinter)),
	name(pName),
	mSetMode(access_default),
	mGetMode(access_const)
{}

template<class T, typename PARAM>
MethodAccessorWrapper<T, PARAM>::MethodAccessorWrapper( ksU32 pIdx, Getter pGetter, Setter pSetter, const char* pName, const IStringifier* pPrinter ) :
	attrib_index(pIdx),
	getter(pGetter),
	setter2(pSetter),
	printer(getDefaultIfNull<PARAM>(pPrinter)),
	name(pName),
	mSetMode(access_default),
	mGetMode(access_default)
{}

template<class T, typename PARAM>
MethodAccessorWrapper<T, PARAM>::MethodAccessorWrapper( ksU32 pIdx, RefGetter pGetter, const char* pName, const IStringifier* pPrinter ) :
	attrib_index(pIdx),
	refGetter(pGetter),
	setter2(NULL),
	printer(getDefaultIfNull<PARAM>(pPrinter)),
	name(pName),
	mSetMode(access_none),
	mGetMode(access_ref)
{}

template<class T, typename PARAM>
MethodAccessorWrapper<T, PARAM>::MethodAccessorWrapper( ksU32 pIdx, RefGetter pGetter, StackSet pSetter, const char* pName, const IStringifier* pPrinter ) :
	attrib_index(pIdx),
	refGetter(pGetter),
	stackSetter(pSetter),
	printer(getDefaultIfNull<PARAM>(pPrinter)),
	name(pName),
	mSetMode(access_DataProperty),
	mGetMode(access_ref)
{}

template<class T, typename PARAM>
void MethodAccessorWrapper<T, PARAM>::ToString( const void* pClassInstance, std::string& buff ) const
{
	if (printer)
	{
		switch (mGetMode)
		{
		case access_const_ref:
		case access_ref:
			{
				PARAM val = (mGetMode == access_const_ref) ? (PARAM&)((T*)(pClassInstance)->*constRefGetter)() : ((T*)(pClassInstance)->*refGetter)();
				//printer->ToString( buff, name, to_pointer<PARAM>(val).Pointer, pClassInstance );
				buff += DataProperty(ks::ref(val)).ToString();
			}
			break;

		case access_const:
		case access_default:
			{
				PARAM val = (mGetMode == access_const) ? ((T*)(pClassInstance)->*constGetter)() : ((T*)(pClassInstance)->*getter)();
				//printer->ToString( buff, name, to_pointer<PARAM>(val).Pointer, pClassInstance );
				buff += DataProperty(ks::ref(val)).ToString();
			}
			break;

		case access_DataProperty:
			{
				KS_ASSERT(!"unsupported!");
			}
			break;

		case access_none:
		default:
			break;
		}

	}
}


//////////////////////////////////////////////////////////////////////////
// ConstRefAccessorWrapper
//////////////////////////////////////////////////////////////////////////
template<class T, typename PARAM>
ConstRefAccessorWrapper<T, PARAM>::ConstRefAccessorWrapper( ksU32 pIdx, CRGetter pGetter, CRSetter pSetter, const char* pName, const IStringifier* pPrinter ) :
	attrib_index(pIdx),
	constRefGetter(pGetter),
	constSetter(pSetter),
	printer(getDefaultIfNull<PARAM>(pPrinter)),
	name(pName)
{}


template<class T, typename PARAM>
void ConstRefAccessorWrapper<T, PARAM>::ToString( const void* pClassInstance, std::string& buff ) const
{
	if (printer)
	{
		auto& val = (PARAM&)((T*)(pClassInstance)->*constRefGetter)();
		printer->ToString( buff, name, to_pointer<PARAM>(val).Pointer, pClassInstance );
	}
}


template<class T, typename PARAM>
void ConstRefAccessorWrapper<T, PARAM>::Set( void* pClassInstance, DataProperty& data ) const
{
	PARAM val;
	data = val;
	(static_cast<T*>(pClassInstance)->*constSetter)( val );
}

template<class T, typename PARAM>
void ConstRefAccessorWrapper<T, PARAM>::Set( void* pClassInstance, const DataProperty& pVal ) const
{
	(static_cast<T*>(pClassInstance)->*constSetter)( pVal.getValue<PARAM>() );
}

template<class T, typename PARAM>
void ConstRefAccessorWrapper<T, PARAM>::Get( void* pClassInstance, DataProperty& data ) const
{
	const PARAM& val	= (static_cast<T*>(pClassInstance)->*constRefGetter)();
	PARAM* val_ref		= const_cast<PARAM*>( &val );
	data = val_ref;
}

//template<class T, typename PARAM>
//void ConstRefAccessorWrapper<T, PARAM>::Get( void* pClassInstance, DataProperty& pResult ) const
//{
//	pResult = (static_cast<T*>(pClassInstance)->*constRefGetter)();
//}




//////////////////////////////////////////////////////////////////////////
// ScriptObjectHandler
//////////////////////////////////////////////////////////////////////////
template<ksU32 IDX, class T, class DEST>
bool add_script_method( Hash_map<ksU32, IFuncInvoker*>& source_array, DEST& dest_array, const ksU32 base_count = 0 )
{
	const auto inv		= source_array.find(IDX) == source_array.end() ? NULL : source_array[ IDX ];
	const bool found	= inv != NULL;
	if ( found )
	{
		dest_array[ base_count+IDX ].name	= inv->GetScriptName();
		dest_array[ base_count+IDX ].mFunc	= &T::template script_op<IDX>;
	}
	return found;
}

template<ksU32 FROM, ksU32 TO>
struct loop_add_script_method
{
	template<class T, class DEST>
	void operator()( Hash_map<ksU32, IFuncInvoker*>& source_array, DEST& dest_array, const ksU32 end_at, const ksU32 base_count = 0 )
	{
		if(FROM < end_at)
		{
			if ( add_script_method<FROM>( source_array, dest_array, base_count ) )
			{
				loop_add_script_method<FROM + 1, TO>()( source_array, dest_array, end_at, base_count );
			}
		}
	}
};

template<ksU32 TO>
struct loop_add_script_method<TO,TO>
{
	template<class T, class DEST>
	void operator()( Hash_map<ksU32, IFuncInvoker*>& source_array, DEST& dest_array, const ksU32 end_at, const ksU32 base_count = 0 )
	{
		KS_ASSERT( (end_at < MAX_EXPORT_METHODS) && "MAX_EXPORT_METHODS might need to be increased." );
	}
};

//
//template<typename T>
//SScriptMethod<T>* AutoBuildScriptObjectMethods()
//{
//	static SScriptMethod<T>* thods(NULL);
//
//	auto customInvokers				= T::GetInvokers();
//	const ksU32 num_meta_methods	= 4;
//	const ksU32 num_methods			= T::GetNumInvokers() + num_meta_methods;
//
//	if(thods == NULL)
//	{
//		thods						= new SScriptMethod<T>[ num_methods + 1 ];
//
//		thods[ 0 ].name				= "__index";		thods[ 0 ].mFunc			= &T::script_get;
//		thods[ 1 ].name				= "__newindex";		thods[ 1 ].mFunc			= &T::script_set;
//		thods[ 2 ].name				= "__tostring";		thods[ 2 ].mFunc			= &T::script_tostring;
//		thods[ 3 ].name				= "__gc";			thods[ 3 ].mFunc			= &T::script_op__gc;
//		thods[ num_methods ].name	= NULL;				thods[ num_methods ].mFunc	= NULL;
//		
//		loop_add_script_method<0, MAX_EXPORT_METHODS>()( customInvokers, thods, num_methods, num_meta_methods );
//	}
//
//	return thods;
//}



//////////////////////////////////////////////////////////////////////////
// AutoScriptObject
//////////////////////////////////////////////////////////////////////////
template<typename T>
Hash_map<ksU32, IAccessorWrapper*>				AutoScriptObject<T>::sAccessors;

template<typename T>
Array<IAccessorWrapper*>						AutoScriptObject<T>::sSortedAccessors;

template<typename T>
int AutoScriptObject<T>::sInstanceCount			= 0;

template<typename T>
ksU32 AutoScriptObject<T>::sAccessorCount		= 0;

template<typename T>
ksU32 AutoScriptObject<T>::sNumMethods			= 0;

template<typename T>
Hash_map<ksU32, IFuncInvoker*>				AutoScriptObject<T>::sFuncInvokers;

template<typename T>
const char* AutoScriptObject<T>::scriptClassName	= ClassUID<T>::getAutoTypeName( "AutoScriptObject_" );

template<typename T>
int AutoScriptObject<T>::scriptMethods		= NULL;


template<typename T>
AutoScriptObject<T>::AutoScriptObject() : mIndentLevel(0)
{
	++sInstanceCount;
}

template<typename T>
AutoScriptObject<T>::AutoScriptObject(const AutoScriptObject<T>& pCopy) : mIndentLevel(pCopy.mIndentLevel)
{
	++sInstanceCount;
}


template<typename T>
AutoScriptObject<T>::~AutoScriptObject()
{
	--sInstanceCount;
	KS_ASSERT(sInstanceCount >= 0 );
	if (sInstanceCount == 0)
	{
		sAccessors.clear();
		sSortedAccessors.clear();
	}
}


template<typename T>
T* AutoScriptObject<T>::newScriptScoped( DataProperty& data )
{
	T* obj = new T();			// if your class has no default constructor & errors here; use DISABLE_SCRIPTSIDE_INSTANTIATION in your header, or write a constructor :)
	obj->setObject( data );
	return obj;
}


template<typename T>
Hash_map<ksU32, IFuncInvoker*>& AutoScriptObject<T>::GetInvokers()
{
	if(sNumMethods == 0)
	{
		T::InitScriptBindings();
		sortPtrMapIntoArrayByOffsets( sAccessors, sSortedAccessors );
	}
	return sFuncInvokers;
}


template<typename T> template<ksU32 ID>
int AutoScriptObject<T>::script_op( DataProperty& data )
{
	auto invoker = sFuncInvokers.find( ID );
	return ( invoker != sFuncInvokers.end() ) ? (*invoker->second)( static_cast<T*>(this), data ) : NULL;
}


//template<typename T>
//int AutoScriptObject<T>::script_op__gc( DataProperty &data )
//{
//	if (sInstanceCount > 0 && mGarbageCollected == 1 )
//	{
//		T* me = static_cast<T*>(this);
//		delete me;
//	}
//	return 0;
//}


template<typename T>
int AutoScriptObject<T>::script_get( DataProperty &data )
{
	auto accessor = sAccessors.find( getKey(data) );

	if ( accessor != sAccessors.end() )
	{
		auto& attrib	= accessor->second;
		attrib->Get( static_cast<T*>(this), data );
	}
	else
	{
		data = nullptr;
	}

	return 1;
}

template<typename T>
int AutoScriptObject<T>::script_set( DataProperty &data )
{
	if( sAccessors.empty() )
	{
		T::InitScriptBindings();
		sortPtrMapIntoArrayByOffsets( sAccessors, sSortedAccessors );
	}

	auto accessor = sAccessors.find( getKey(data) );

	if ( accessor != sAccessors.end() )
	{
		auto& attrib	= accessor->second;
		attrib->Set( static_cast<T*>(this), data );
	}

	return 0;
}



template<typename T>
int AutoScriptObject<T>::script_tostring( DataProperty& data )
{
	std::string buff;

	getString( buff );

	data = buff;
	return 1;
}

template<typename T>
void AutoScriptObject<T>::getString( std::string& buff ) const
{
	static const bool sEnclosingBraces	= false;
	static const int mIndentLevel		= 1;

	auto* derived_me	= static_cast<const T*>( this );

	if(sEnclosingBraces)
	{
		buff	+= "\n";
		for (int i = 0; i < int(mIndentLevel - 1); ++i)
			buff += "\t";

		buff	+= "{";
	}

	for ( auto itr = sSortedAccessors.begin(); itr != sSortedAccessors.end(); ++itr )
	{
		buff += "\n";

		for (ksU32 i = 0; i < mIndentLevel; ++i)
			buff += "\t";

		(*itr)->ToString( derived_me, buff );
	}

	if(sEnclosingBraces)
	{
		buff	+= "\n";
		for (int i = 0; i < int(mIndentLevel - 1); ++i)
			buff += "\t";

		buff	+= "},";
	}
}

}	// namespace ks

#endif