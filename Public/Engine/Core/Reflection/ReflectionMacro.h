#pragma once


#define REFVARIABLE(...) [[ReflectedVariable, __VA_ARGS__]]

#define REFMETHOD(...) [[ReflectedMethod, __VA_ARGS__]]

#define REFCLASS(...) [[ReflectedClass, __VA_ARGS__]]

