#GCE Refactoring
##Current Status
Build Successfully with `cmake` and `make`.

##File Organization

All source files are refactored to be lower-cased.

content | detail
--- | ---
[algorithm](algorithm) | algorithm related files 
[util](util) | utilities including graph io preprocessing and other utils
[useless](useless) | not-used files
[CMakeLists.txt](CMakeLists.txt) | cmake config file

##Issues
- [range.hpp](util/range.hpp), not so useful now in c++14, so `typeof` is changed into `decltype`,\
 which is supported in modern C++

Refer to [typeid-versus-typeof-in-cpp](http://stackoverflow.com/questions/1986418/typeid-versus-typeof-in-c).

First:

> typeof is a compile time construct and returns the type as defined at compile time
  typeid is a runtime construct and hence gives information about the runtime type of the value.

Second:

> up vote
  114
  down vote
  accepted
  C++ language has no such thing as typeof. You must be looking at some compiler-specific extension. If you are talking about GCC's typeof, then a similar feature is present in C++11 through the keywords decltype and auto. Again, C++ has no such typeof keyword.
  
>  typeid is a C++ language operator which returns type identification information at run time. It basically returns a type_info object, which is equality-comparable with other type_info objects.
  
>  Note, that the only defined property of the returned type_info object has is its being equality- and non-equality-comparable, i.e. type_info objects describing different types shall compare non-equal, while type_info objects describing the same type have to compare equal. Everything else is implementation-defined. Methods that return various "names" are not guaranteed to return anything human-readable, and even not guaranteed to return anything at all.
  
>  Note also, that the above probably implies (although the standard doesn't seem to mention it explicitly) that consecutive applications of typeid to the same type might return different type_info objects (which, of course, still have to compare equal).