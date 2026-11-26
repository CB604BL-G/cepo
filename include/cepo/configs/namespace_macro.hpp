//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_CONFIGS_NAMESPACE_MACRO_HPP
#define CEPO_CONFIGS_NAMESPACE_MACRO_HPP

//Trailing semicolons on NAMESPACE_START/END are for tree-sitter, not the
//compiler. tree-sitter does not expand macros, so without a terminator it
//cannot tell where the macro call ends and keeps mis-highlighting the file.
//The semicolon is an empty-declaration in namespace scope (legal since C++11)
//and has no effect on compilation.

//Do NOT remove the semicolons at the call sites. Do NOT remove this comment.

#define CEPO_NAMESPACE_START namespace cepo {
#define CEPO_NAMESPACE_END }

#define CEPO_IMPL_NAMESPACE_START namespace cepo { namespace impl {
#define CEPO_IMPL_NAMESPACE_END } }

#define CEPO_NAMESPACE_START_OF_MODULE(module) namespace cepo { namespace module {
#define CEPO_NAMESPACE_END_OF_MODULE } }

#define CEPO_IMPL_NAMESPACE_START_OF_MODULE(module) namespace cepo { namespace module { namespace impl {
#define CEPO_IMPL_NAMESPACE_END_OF_MODULE } } }

#endif //CEPO_CONFIGS_NAMESPACE_MACRO_HPP
