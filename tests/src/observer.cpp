#include "doctest_extra.h"
#include "ureact/ureact.hpp"

// default constructive
static_assert( std::is_default_constructible_v<ureact::observer> );

// nothrow movable only
static_assert( !std::is_copy_constructible_v<ureact::observer> );
static_assert( !std::is_copy_assignable_v<ureact::observer> );
static_assert( std::is_move_constructible_v<ureact::observer> );
static_assert( std::is_move_assignable_v<ureact::observer> );
static_assert( std::is_nothrow_move_constructible_v<ureact::observer> );
static_assert( std::is_nothrow_move_assignable_v<ureact::observer> );


// scoped_observer is not intended to be default constructive, but move constructive from observer
static_assert( !std::is_default_constructible_v<ureact::scoped_observer> );
static_assert( std::is_constructible_v<ureact::scoped_observer, ureact::observer&&> );
static_assert( !std::is_constructible_v<ureact::scoped_observer, const ureact::observer&> );

// nothrow movable only
static_assert( !std::is_copy_constructible_v<ureact::scoped_observer> );
static_assert( !std::is_copy_assignable_v<ureact::scoped_observer> );
static_assert( std::is_move_constructible_v<ureact::scoped_observer> );
static_assert( std::is_move_assignable_v<ureact::scoped_observer> );
static_assert( std::is_nothrow_move_constructible_v<ureact::scoped_observer> );
static_assert( std::is_nothrow_move_assignable_v<ureact::scoped_observer> );
