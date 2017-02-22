//////////////////////////////////////////////////////////////////////////
///
///
///	@author		Nnanna Kama
///	@date		23/05/2013
///	@brief		Utility Macros & Templates
///	@remarks
///
///
//////////////////////////////////////////////////////////////////////////

#include <string>
#include <unordered_map>
#include <crc32.h>
#include <defines.h>
#include <Containers\Array.h>
#include <Reflection\DataProperty.h>
#include <Reflection\Delegate.h>

#ifndef SCRIPT_UTILITY_H
#define SCRIPT_UTILITY_H

class CScript;


#define Hash_map	std::unordered_map


namespace ks {

//////////////////////////////////////////////////////////////////////////
//	STRINGIFIERS
//////////////////////////////////////////////////////////////////////////

struct IStringifier
{
	virtual ~IStringifier()	{}
	virtual void ToString( std::string& buff, const char* pName, void* pVar, const void* pClassObject ) const		= 0;
};


class IAccessorWrapper
{
public:
	virtual ~IAccessorWrapper() {}
	inline bool operator== (const IAccessorWrapper& pRHS) const			{ return sort_index() == pRHS.sort_index(); }
	inline virtual ksU32 sort_index() const								{ return 0; }
	inline virtual void Set( void* pClassInstance, DataProperty& data ) const = 0;
	inline virtual void Get( void* pClassInstance, DataProperty& data ) const = 0;
	inline virtual void ToString( const void* pClassInstance, std::string& buff ) const = 0;
};


//////////////////////////////////////////////////////////////////////////
// MethodAccessorWrapper
//////////////////////////////////////////////////////////////////////////
template<class T, typename PARAM>
class MethodAccessorWrapper : public IAccessorWrapper
{
	typedef enum mawAccessMode
	{
		access_default,
		access_ref,
		access_const,
		access_const_ref,
		access_DataProperty,
		access_none
	}mawAccessMode;

	typedef void(T::*Setter)( PARAM pVal );
	typedef void(T::*ConstRefdSetter)( const PARAM& pVal );
	typedef PARAM (T::*Getter)();
	typedef PARAM (T::*ConstGetter)() const;
	typedef PARAM& (T::*RefGetter)();
	typedef const PARAM& (T::*ConstRefGetter)() const;
	typedef void (T::*StackSet)(DataProperty& data);
public:

	MethodAccessorWrapper( ksU32 pIdx, ConstRefGetter pGetter, ConstRefdSetter pSetter, const char* pName, const IStringifier* pPrinter = NULL );

	MethodAccessorWrapper( ksU32 pIdx, ConstGetter pGetter, Setter pSetter, const char* pName, const IStringifier* pPrinter = NULL );

	MethodAccessorWrapper( ksU32 pIdx, Getter pGetter, Setter pSetter, const char* pName, const IStringifier* pPrinter = NULL );

	MethodAccessorWrapper( ksU32 pIdx, RefGetter pGetter, const char* pName, const IStringifier* pPrinter = NULL );

	MethodAccessorWrapper( ksU32 pIdx, RefGetter pGetter, StackSet pSetter, const char* pName, const IStringifier* pPrinter = NULL );

	void Set( void* pClassInstance, DataProperty& data ) const override;
	//void Set( void* pClassInstance, const DataProperty& pVal ) const override;
	void Get( void* pClassInstance, DataProperty& data ) const override;
	void ToString( const void* pClassInstance, std::string& buff ) const override;

	ksU32 sort_index() const override										{ return attrib_index; }

private:
	const ksU32			attrib_index;
	union
	{
		ConstRefGetter	constRefGetter;
		ConstGetter		constGetter;
		RefGetter		refGetter;
		Getter			getter;
	};
	union
	{
		ConstRefdSetter	constSetter;
		StackSet		stackSetter;
		Setter			setter2;
	};

	const IStringifier*	printer;
	const char*			name;
	const mawAccessMode	mSetMode;
	const mawAccessMode	mGetMode;
};


//////////////////////////////////////////////////////////////////////////
// ConstRefAccessorWrapper
//////////////////////////////////////////////////////////////////////////
template<class T, typename PARAM>
class ConstRefAccessorWrapper : public IAccessorWrapper
{
	typedef void(T::*CRSetter)( const PARAM& pVal );
	typedef const PARAM& (T::*CRGetter)() const;
public:

	ConstRefAccessorWrapper( ksU32 pIdx, CRGetter pGetter, CRSetter pSetter, const char* pName, const IStringifier* pPrinter = NULL );

	void Set( void* pClassInstance, DataProperty& data ) const override;
	void Set( void* pClassInstance, const DataProperty& pVal ) const override;
	void Get( void* pClassInstance, DataProperty& data ) const override;
	void ToString( const void* pClassInstance, std::string& buff ) const override;

	ksU32 sort_index() const override										{ return attrib_index; }

private:
	const ksU32			attrib_index;
	CRGetter			constRefGetter;
	CRSetter			constSetter;
	const IStringifier*	printer;
	const char*			name;
};




//////////////////////////////////////////////////////////////////////////
// FUNCTION INTERFACE. pass in a method and it returns an methor return type

#define DEDUCE_TYPE(Class, Meth)					ks::strip_qualifiers<decltype(((Class*)nullptr)->Meth())>::Type


//////////////////////////////////////////////////////////////////////////
// AutoScriptObject
// For easier setting up of Script Objects
// Derived types only need implement T::InitScriptBindings
//////////////////////////////////////////////////////////////////////////

template<class T>
class AutoScriptObject
{
public:
	AutoScriptObject();

	AutoScriptObject(const AutoScriptObject& pCopy);

	~AutoScriptObject();

	static T* newScriptScoped( DataProperty& data );

	int script_get( DataProperty &data );

	int script_set( DataProperty &data );

	int script_tostring( DataProperty& data );

	void getString( std::string& buff ) const;


	template<ksU32 ID>
	int script_op( DataProperty& data );

	static Hash_map<ksU32, IFuncInvoker*>&			GetInvokers();
	static const ksU32				GetNumInvokers()					{ return sNumMethods; }
	static const char*				GetClassName()						{ return scriptClassName; }

private:
	static Array<IAccessorWrapper*>				sSortedAccessors;
	static int									sInstanceCount;

protected:
	static Hash_map<ksU32, IAccessorWrapper*>	sAccessors;
	static ksU32								sAccessorCount;
	static ksU32								sNumMethods;
	static Hash_map<ksU32, IFuncInvoker*>	sFuncInvokers;

	typedef T TDerived;

#define ADD_OBJECT_GET_SET_ACCESSOR(name, getter, setter)																				\
	typedef DEDUCE_TYPE(TDerived, getter) name##arg_t;																					\
	static ks::MethodAccessorWrapper<TDerived, name##arg_t> name##_acc(sAccessorCount++, &TDerived::getter, &TDerived::setter, #name );	\
	sAccessors[ CRC32(#name) ] = &name##_acc																							\

#define ADD_OBJECT_GET_SET_ACCESSOR2(name, getter, setter, printer)																					\
	typedef DEDUCE_TYPE(TDerived, getter) name##arg_t;																								\
	static ks::MethodAccessorWrapper<TDerived, name##arg_t> name##_acc(sAccessorCount++, &TDerived::getter, &TDerived::setter, #name, &printer );	\
	sAccessors[ CRC32(#name) ] = &name##_acc																										\

#define ADD_OBJECT_CONST_GETSET(name, getter, setter)																							\
	typedef DEDUCE_TYPE(TDerived, getter) name##arg_t;																							\
	static ks::ConstRefAccessorWrapper<TDerived, name##arg_t> name##_ac(sAccessorCount++, &TDerived::getter, &TDerived::setter, #name );		\
	sAccessors[ CRC32(#name) ] = &name##_ac																										\

#define ADD_OBJECT_GETREF_ACCESSOR(name, ref_getter)																		\
	typedef DEDUCE_TYPE(TDerived, ref_getter) name##arg_t;																	\
	static ks::MethodAccessorWrapper<TDerived, name##arg_t> name##_acc(sAccessorCount++, &TDerived::ref_getter, #name );	\
	sAccessors[ CRC32(#name) ] = &name##_acc																				\

#define ADD_OBJECT_METHOD(meth_name, meth)																	\
	if( scriptMethods == NULL )																				\
		sFuncInvokers[sNumMethods++] = ks::IFuncInvoker::Create<TDerived>( #meth_name, &TDerived::meth );	\

#define DISABLE_SCRIPTSIDE_INSTANTIATION( DCLASS )										\
	template<>																			\
	inline DCLASS* ks::AutoScriptObject<DCLASS>::newScriptScoped( DataProperty& data )	\
	{																					\
		KS_ASSERT("This object should not be created from script.");					\
		return NULL;																	\
	}																					\


public:
	static const char*			scriptClassName;
	static int					scriptMethods;
};


}	// namespace ks


#endif
