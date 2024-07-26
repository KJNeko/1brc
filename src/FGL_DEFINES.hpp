//
// Created by kj16609 on 3/1/24.
//

#pragma once

#define FGL_DELETE_DEFAULT_CTOR( ClassName ) ClassName() = delete;
#define FGL_DELETE_COPY_ASSIGN( ClassName ) ClassName& operator=( const ClassName& ) = delete;
#define FGL_DELETE_COPY_CTOR( ClassName ) ClassName( const ClassName& ) = delete;
#define FGL_DELETE_MOVE_ASSIGN( ClassName ) ClassName& operator=( ClassName&& ) = delete;
#define FGL_DELETE_MOVE_CTOR( ClassName ) ClassName( ClassName&& ) = delete;
#define FGL_DELETE_COPY( ClassName ) FGL_DELETE_COPY_CTOR( ClassName ) FGL_DELETE_COPY_ASSIGN( ClassName )
#define FGL_DELETE_MOVE( ClassName ) FGL_DELETE_MOVE_CTOR( ClassName ) FGL_DELETE_MOVE_ASSIGN( ClassName )
#define FGL_DELETE_ALL_Ro5( ClassName )                                                                                \
	FGL_DELETE_DEFAULT_CTOR( ClassName ) FGL_DELETE_COPY( ClassName ) FGL_DELETE_MOVE( ClassName )

#define FGL_FLATTEN [[gnu::flatten]]
#define FGL_ARTIFICIAL [[gnu::artificial]]
#define FGL_HOT [[gnu::hot]]
#define FGL_COLD [[gnu::cold]]
#define FGL_FLATTEN_HOT FGL_FLATTEN FGL_HOT
#define FGL_FORCE_INLINE [[gnu::always_inline]]
#define FGL_FORCE_INLINE_FLATTEN FGL_FLATTEN FGL_FORCE_INLINE
#define FGL_FLATTEN_HOT_INLINE FGL_FLATTEN FGL_FORCE_INLINE FGL_HOT

#define FGL_ASSUME( ... ) [[gnu::assume( __VA_ARGS__ )]]

#define FGL_ALIGN( bytesize ) [[gnu::alligned( bitsize )]]

#define FGL_FUNC_CLEANUP( func ) [[gnu::cleanup( func )]]

//! Warns if the variable is used as a string (strlen)
#define FGL_NONSTRING_DATA [[gnu::nonstring]]

//! Warns if the structure field is not alligned with a set number of bytes
#define FGL_STRICT_ALIGNMENT( bytesize ) [[gnu::warn_if_not_aligned( bytesize )]]

#define FGL_ASSERT( ... ) assert( __VA_ARGS__ );


#ifndef NDEBUG
#define FGL_UNREACHABLE()                                                                                              \
	FGL_ASSERT( false );                                                                                               \
	std::unreachable()
#else
#define FGL_UNREACHABLE() std::unreachable()
#endif