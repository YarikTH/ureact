#pragma once

// C++ language standard detection
#if ( defined( __cplusplus ) && __cplusplus >= 202002L ) || ( defined( _MSVC_LANG ) && _MSVC_LANG >= 202002L )
#    define REACT_HAS_CPP_20
#    define REACT_HAS_CPP_17
#    define REACT_HAS_CPP_14
#elif ( defined( __cplusplus ) && __cplusplus >= 201703L ) || ( defined( _HAS_CXX17 ) && _HAS_CXX17 == 1 ) // fix for issue #464
#    define REACT_HAS_CPP_17
#    define REACT_HAS_CPP_14
#elif ( defined( __cplusplus ) && __cplusplus >= 201402L ) || ( defined( _HAS_CXX14 ) && _HAS_CXX14 == 1 )
#    define REACT_HAS_CPP_14
#endif
