#pragma once
//
// File configured by CMake do not edit Include.h //
//

// #define ALTERNATIVE_EXCEPTIONS_FATAL
// #define NO_DEFAULT_DATAINDEX

// If defined some select classes are leaked into global namespace
#define LEAK_INTO_GLOBAL
#define ALLOW_INTERNAL_EXCEPTIONS

// Sandbox properly works on non-windows platforms
#ifndef _WIN32
#undef CEF_ENABLE_SANDBOB
#else
// #define CEF_ENABLE_SANDBOX
// I can't figure out how to link this as it its built with the wrong runtime type and
// I'd probably have to compile CEF myself if this is needed
#undef CEF_ENABLE_SANDBOB
#endif //_WIN32



/* #undef LEVIATHAN_NO_DEBUG */

/* #undef LEVIATHAN_USING_ANGELSCRIPT */

/* #undef LEVIATHAN_USING_BOOST */
// Can't be disabled
#define LEVIATHAN_USING_BOOST

/* #undef LEVIATHAN_USING_OGRE */

/* #undef LEVIATHAN_USING_ANGELSCRIPT */

/* #undef LEVIATHAN_USING_BULLET */

/* #undef LEVIATHAN_USING_CEF */

/* #undef LEVIATHAN_USING_SDL2 */

/* #undef LEVIATHAN_USING_SFML */
#ifdef LEVIATHAN_USING_SFML
#define SFML_PACKETS
#endif // LEVIATHAN_USING_SFML

/* #undef LEVIATHAN_USING_LEAP */

#define LEVIATHAN_USE_ACTUAL_OBJECT_POOLS

#define LEVIATHAN_VERSION 0.210
#define LEVIATHAN_VERSIONS L"0.2.1.0"
#define LEVIATHAN_VERSION_ANSIS "0.2.1.0"

#define LEVIATHAN_VERSION_STABLE 0
#define LEVIATHAN_VERSION_MAJOR 2
#define LEVIATHAN_VERSION_MINOR 1
#define LEVIATHAN_VERSION_PATCH 0

#define LEVIATHAN

#ifdef __GNUC__
#define __FUNCSIG__ __PRETTY_FUNCTION__
#endif

#ifndef DLLEXPORT
#ifdef ENGINE_EXPORTS
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
// This might not be needed for gcc
#define DLLEXPORT __attribute__((visibility("default")))
#endif
// Json-cpp //
#define JSON_DLL_BUILD
#else

#ifdef _WIN32
#define DLLEXPORT __declspec(dllimport)
#else
#define DLLEXPORT
#endif


#define JSON_DLL
#endif // ENGINE_EXPORTS
#endif

#ifndef FORCE_INLINE
#ifndef _WIN32

#define FORCE_INLINE __attribute__((always_inline))

#else
// Windows needs these //
#define FORCE_INLINE __forceinline
#endif
#endif // FORCE_INLINE

#ifndef NOT_UNUSED
#define NOT_UNUSED(x) (void)x;
#endif // NOT_UNUSED
