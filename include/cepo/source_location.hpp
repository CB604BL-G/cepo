//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_SOURCE_LOCATION_HPP
#define CEPO_SOURCE_LOCATION_HPP

#include "cepo/configs/namespace_macro.h"

#if defined(__clang__) or defined(__GNUC__)
	#define CEPO_IMPL_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
	#define CEPO_IMPL_CURRENT_FUNCTION __FUNCSIG__
#else
	#define CEPO_IMPL_CURRENT_FUNCTION __func__
#endif

//How to use these two classes.

//Construct a location with one of the four macros at the bottom of this file,
//or copy and move an existing one. Construction, copy construction, move
//construction, copy assignment and move assignment are the only ways to keep
//the recorded file, line and function name meaningful.

//Passing the constructor arguments yourself is undefined behavior. Those
//parameters exist to receive whatever __FILE__, __LINE__ and the compiler
//helper expand to; nothing checks a hand written value and nothing promises
//that its meaning stays the same.

//anywhere_source_location may be written almost anywhere - namespace scope, a
//member initializer, a default argument, a function body, a lambda - as it
//only needs __FILE__ and __LINE__. function_source_location also needs the
//name of the enclosing function and therefore belongs inside a function body.
//Written elsewhere the behavior is undefined: MSVC may refuse to compile it,
//GCC may store an empty string, and clang stores a placeholder ("top level")
//together with a -Wpredefined-identifier-outside-function warning.
CEPO_NAMESPACE_START;

class anywhere_source_location
{
private:
	struct area_type
	{
		const char* file_name;
		unsigned line;
	};

	area_type area;
	
public:
	constexpr anywhere_source_location(
		const char* filename,
		unsigned fileline) noexcept:
		area({filename, fileline})
	{}

	~anywhere_source_location() = default;
	constexpr anywhere_source_location(const anywhere_source_location&) noexcept = default;
	constexpr anywhere_source_location(anywhere_source_location&&) noexcept = default;
	//Under C++11, the 'operator=' of this class is not marked 'constexpr'.
	//Compatibility with later C++ standards is not considered for now. The
	//same applies to the class below.
	anywhere_source_location& operator=(const anywhere_source_location&) noexcept = default;
	anywhere_source_location& operator=(anywhere_source_location&&) noexcept = default;

	constexpr const char* file_name() const noexcept
	{
		return area.file_name;
	}
	
	constexpr unsigned line() const noexcept
	{
		return area.line;
	}

	anywhere_source_location() = delete;
};

class function_source_location
{
private:
	struct area_type
	{
		const char* file_name;
		const char* function_name;
		unsigned line;
	};

	area_type area;
	
public:
	constexpr function_source_location(
		const char* filename,
		const char* funcname,
		unsigned fileline) noexcept:
		area({filename, funcname, fileline})
	{}

	~function_source_location() = default;
	constexpr function_source_location(const function_source_location&) noexcept = default;
	constexpr function_source_location(function_source_location&&) noexcept = default;
	function_source_location& operator=(const function_source_location&) noexcept = default;
	function_source_location& operator=(function_source_location&&) noexcept = default;

	constexpr const char* file_name() const noexcept
	{
		return area.file_name;
	}

	constexpr const char* function_name() const noexcept
	{
		return area.function_name;
	}
	
	constexpr unsigned line() const noexcept
	{
		return area.line;
	}

	function_source_location() = delete;
};

CEPO_NAMESPACE_END;

#define CEPO_CURRENT_ANYWHERE_SOURCE_LOCATION() \
	::cepo::anywhere_source_location{__FILE__, __LINE__}

#define CEPO_ANYWHERE_SOURCE_LOCATION(var_name) \
	::cepo::anywhere_source_location var_name{__FILE__, __LINE__}

#define CEPO_CURRENT_FUNCTION_SOURCE_LOCATION() \
	::cepo::function_source_location{__FILE__, CEPO_IMPL_CURRENT_FUNCTION, __LINE__}

#define CEPO_FUNCTION_SOURCE_LOCATION(var_name) \
	::cepo::function_source_location var_name{__FILE__, CEPO_IMPL_CURRENT_FUNCTION, __LINE__}

#endif //CEPO_SOURCE_LOCATION_HPP
