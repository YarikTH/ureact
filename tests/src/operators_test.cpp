#include "doctest/doctest.h"
#include "ureact/ureact.hpp"

TEST_SUITE( "operators" )
{
    TEST_CASE( "unary operators" )
    {
        ureact::context ctx;

        auto v1 = make_var( ctx, 1 );

        // clang-format off
        auto unary_plus           = +v1;
        auto unary_minus          = -v1;
        auto logical_negation     = !v1;
        auto bitwise_complement   = ~v1;
        auto unary_plus_2         = +(+v1);
        auto unary_minus_2        = -(+v1);
        auto logical_negation_2   = !(+v1);
        auto bitwise_complement_2 = ~(+v1);
        // clang-format on

        auto checkValues = [&]( std::initializer_list<int> valuesToTest ) {
            for ( const int& value : valuesToTest )
            {
                v1 <<= value;

                // clang-format off
                CHECK( unary_plus.value()           == (+value) );
                CHECK( unary_minus.value()          == (-value) );
                CHECK( logical_negation.value()     == (!value) );
                CHECK( bitwise_complement.value()   == (~value) );
                CHECK( unary_plus_2.value()         == (+value) );
                CHECK( unary_minus_2.value()        == (-value) );
                CHECK( logical_negation_2.value()   == (!value) );
                CHECK( bitwise_complement_2.value() == (~value) );
                // clang-format on
            }
        };

        checkValues( { 0, 1, -4, 654 } );
    }

    TEST_CASE( "binary operators (normal)" )
    {
        ureact::context ctx;

        auto v1 = make_var( ctx, 0 );
        auto v2 = make_var( ctx, 1 );

        // clang-format off
        auto addition            = v1 + v2;
        auto subtraction         = v1 - v2;
        auto multiplication      = v1 * v2;
        auto bitwise_and         = v1 & v2;
        auto bitwise_or          = v1 | v2;
        auto bitwise_xor         = v1 ^ v2;
        auto bitwise_left_shift  = v1 << v2;
        auto bitwise_right_shift = v1 >> v2;
        auto equal               = v1 == v2;
        auto not_equal           = v1 != v2;
        auto less                = v1 < v2;
        auto less_equal          = v1 <= v2;
        auto greater             = v1 > v2;
        auto greater_equal       = v1 >= v2;
        auto logical_and         = v1 && v2;
        auto logical_or          = v1 || v2;
        // clang-format on

        auto checkValues = [&]( std::initializer_list<std::pair<int, int>> valuesToTest ) {
            for ( const auto& values : valuesToTest )
            {
                ctx.do_transaction( [&]() {
                    v1 <<= values.first;
                    v2 <<= values.second;
                } );

                // clang-format off
                CHECK( addition.value()            == (values.first +  values.second) );
                CHECK( subtraction.value()         == (values.first -  values.second) );
                CHECK( multiplication.value()      == (values.first *  values.second) );
                CHECK( equal.value()               == (values.first == values.second) );
                CHECK( not_equal.value()           == (values.first != values.second) );
                CHECK( less.value()                == (values.first <  values.second) );
                CHECK( less_equal.value()          == (values.first <= values.second) );
                CHECK( greater.value()             == (values.first >  values.second) );
                CHECK( greater_equal.value()       == (values.first >= values.second) );
                CHECK( logical_and.value()         == (values.first && values.second) );
                CHECK( logical_or.value()          == (values.first || values.second) );
                CHECK( bitwise_and.value()         == (values.first &  values.second) );
                CHECK( bitwise_or.value()          == (values.first |  values.second) );
                CHECK( bitwise_xor.value()         == (values.first ^  values.second) );
                CHECK( bitwise_left_shift.value()  == (values.first << values.second) );
                CHECK( bitwise_right_shift.value() == (values.first >> values.second) );
                // clang-format on
            }
        };

        checkValues( {
            { 2, 2 },
            { 3, -3 },
            { 0, 0 },
            { -4, 7 },
            { -8, -2 },
            { 0, -2 },
            { 5, 0 },
        } );
    }

    TEST_CASE( "binary operators (divisible)" )
    {
        ureact::context ctx;

        struct test_data
        {
            const char* test_name;
            ureact::signal<int> division;
            ureact::signal<int> modulo;
        };

        SUBCASE( "both signals" )
        {
            auto lhs = make_var( ctx, 0 );
            auto rhs = make_var( ctx, 1 );

            // clang-format off
            test_data test_data_array [] =
            {
                { "     signal op signal",        lhs  /   rhs,    lhs  %   rhs  },
                { "temp_signal op signal",      (+lhs) /   rhs,  (+lhs) %   rhs  },
                { "     signal op temp_signal",   lhs  / (+rhs),   lhs  % (+rhs) },
                { "temp_signal op temp_signal", (+lhs) / (+rhs), (+lhs) % (+rhs) },
            };
            // clang-format on

            std::initializer_list<std::pair<int, int>> values_to_test = {
                { 2, 2 },
                { 3, -3 },
                { 8, 3 },
            };

            for( const auto& values : values_to_test )
            {
                int left, right;
                std::tie(left, right) = values;
                SUBCASE( (std::to_string(left) + std::string(" op ") + std::to_string(right)).c_str() )
                {
                    ctx.do_transaction( [&]() {
                        lhs <<= left;
                        rhs <<= right;
                    } );
                    for( const test_data& data : test_data_array )
                    {
                        SUBCASE( data.test_name )
                        {
                            // clang-format off
                            CHECK( data.division.value() == (left / right) );
                            CHECK( data.modulo.value()   == (left % right) );
                            // clang-format on
                        }
                    }
                }
            }
        }

        SUBCASE( "signals and values" )
        {
            const int val = 32;
            auto var = make_var( ctx, 1 );

            SUBCASE( "left value" )
            {
                // clang-format off
                test_data test_data_array [] =
                {
                    { "      value op signal",        val  /   var ,   val  %   var  },
                    { "      value op temp_signal",   val  / (+var),   val  % (+var) },
                };
                // clang-format on

                for( const auto& right : { -32, -16, -8, -5, -4, -3, -2, -1, 1, 2, 3, 4, 5, 8, 16, 32 } )
                {
                    const int left = val;
                    SUBCASE( (std::to_string(left) + std::string(" op ") + std::to_string(right)).c_str() )
                    {
                        var <<= right;
                        for( const test_data& data : test_data_array )
                        {
                            SUBCASE( data.test_name )
                            {
                                // clang-format off
                                CHECK( data.division.value() == (left / right) );
                                CHECK( data.modulo.value()   == (left % right) );
                                // clang-format on
                            }
                        }
                    }
                }
            }

            SUBCASE( "right value" )
            {
                // clang-format off
                test_data test_data_array [] =
                {
                    { "     signal op value",         var  /   val,    var  %   val  },
                    { "temp_signal op value",       (+var) /   val,  (+var) %   val  },
                };
                // clang-format on

                for( const auto& left : { -32, -16, -8, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 8, 16, 32 } )
                {
                    const int right = val;
                    SUBCASE( (std::to_string(left) + std::string(" op ") + std::to_string(right)).c_str() )
                    {
                        var <<= left;
                        for( const test_data& data : test_data_array )
                        {
                            SUBCASE( data.test_name )
                            {
                                // clang-format off
                                CHECK( data.division.value() == (left / right) );
                                CHECK( data.modulo.value()   == (left % right) );
                                // clang-format on
                            }
                        }
                    }
                }
            }
        }
    }

    TEST_CASE( "priority" )
    {
        // https://en.cppreference.com/w/cpp/language/operator_precedence
        // 3.  +a -a          Unary plus and minus
        //     ! ~            Logical NOT and bitwise NOT
        // 5.  a*b  a/b  a%b  Multiplication, division, and remainder
        // 6.  a+b  a-b       Addition and subtraction
        // 7.  <<  >>         Bitwise left shift and right shift
        // 8.  <=>            Three-way comparison operator (since C++20)
        // 9.  <  <=          For relational operators < and ≤ respectively
        //     >  >=          For relational operators > and ≥ respectively
        // 10. ==  !=         For equality operators = and ≠ respectively
        // 11. &              Bitwise AND
        // 12. ^              Bitwise XOR (exclusive or)
        // 13. |              Bitwise OR (inclusive or)
        // 14. &&             Logical AND
        // 15. ||             Logical OR

        ureact::context ctx;

        auto _2 = make_var( ctx, 2 );

        auto result = _2 + _2 * _2;
        CHECK( result.value() == 6 );

        auto result2 = ( _2 + _2 ) * _2;
        CHECK( result2.value() == 8 );
    }

} // TEST_SUITE_END
