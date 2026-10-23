//Copyright (c) 2026 CB604BL
#ifndef CEPO_MACROS_NODISCARD_HPP
#define CEPO_MACROS_NODISCARD_HPP

// C++11 does not have the [[nodiscard]] attribute; it was only added to the
// standard in C++17. Therefore, in C++11 we can only implement something similar.

// With GCC, the corresponding warning option must be enabled to see warnings
// when a return value is unused; Clang enables this by default. For other
// compilers, this macro has no effect. MSVC has _Check_return_, but using it
// would require including <sal.h>, which we do not want to do.

// Obviously, when using this macro, we can no longer append a semicolon after
// it to keep Tree-sitter from complaining. We will just have to live with it.

//Detect clang first
#if defined(__clang__)
	#define CEPO_NODISCARD [[clang::warn_unused_result]]
#elif defined(__GNUC__)
	#define CEPO_NODISCARD [[gnu::warn_unused_result]]
#else
	#define CEPO_NODISCARD
#endif

#endif //CEPO_MACROS_NODISCARD_HPP
