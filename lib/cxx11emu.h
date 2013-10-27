#ifndef NULLPTR_H
#define NULLPTR_H

#ifdef __cplusplus
#if __cplusplus < 201103L

// Source: SC22/WG21/N2431 = J16/07-0301
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2431.pdf

const                        // this is a const object...
class {
public:
	template<class T>          // convertible to any type
	operator T*() const      // of null non-member
	{ return 0; }            // pointer...
	template<class C, class T> // or any type of null
	operator T C::*() const  // member pointer...
	{ return 0; }
private:
	void operator&() const;    // whose address can't be taken
} nullptr = {};              // and whose name is nullptr

#endif // __cplusplus <
#endif // __cplusplus
#endif // NULLPTR_H
