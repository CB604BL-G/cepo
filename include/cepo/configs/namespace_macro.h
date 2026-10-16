//Copyright (c) 2026 CB604BL
#ifndef CEPO_CONFIGS_NAMESPACE_MACRO_H
#define CEPO_CONFIGS_NAMESPACE_MACRO_H

//Trailing semicolons on NAMESPACE_START/END are for tree-sitter, not the
//compiler. tree-sitter does not expand macros, so without a terminator it
//cannot tell where the macro call ends and keeps mis-highlighting the file.
//The semicolon is an empty-declaration in namespace scope (legal since C++11)
//and has no effect on compilation.

//Do NOT remove the semicolons at the call sites. Do NOT remove this comment.
#define CEPO_NAMESPACE_START namespace cepo {
#define CEPO_NAMESPACE_END }

#endif //CEPO_CONFIGS_NAMESPACE_MACRO_H
