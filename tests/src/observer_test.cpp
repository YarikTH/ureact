#include "doctest/doctest.h"
#include "ureact/context.hpp"
#include "ureact/signal.hpp"

namespace
{

}

TEST_SUITE_BEGIN( "ObserverTest" );

TEST_CASE( "detach" )
{
    ureact::context ctx;
    
    auto a1 = make_var(&ctx, 1);
    auto a2 = make_var(&ctx, 1);

    auto result = a1 + a2;

    int observeCount1 = 0;
    int observeCount2 = 0;
    int observeCount3 = 0;

    int phase = 0;

    auto obs1 = observe(result, [&] (int v)
    {
        observeCount1++;

        if (phase == 0)
            CHECK(v == 3);
        else
            FAIL("We shouldn't be here");
    });

    auto obs2 = observe(result, [&] (int v)
    {
        observeCount2++;

        if (phase == 0)
            CHECK(v == 3);
        else if (phase == 1)
            CHECK(v == 4);
        else
            FAIL("We shouldn't be here");
    });

    auto obs3 = observe(result, [&] (int v)
    {
        observeCount3++;

        if (phase == 0)
            CHECK(v == 3);
        else if (phase == 1)
            CHECK(v == 4);
        else
            FAIL("We shouldn't be here");
    });

    phase = 0;
    a1 <<= 2;
    CHECK(observeCount1 == 1);
    CHECK(observeCount2 == 1);
    CHECK(observeCount3 == 1);

    phase = 1;
    obs1.detach();
    a1 <<= 3;
    CHECK(observeCount1 == 1);
    CHECK(observeCount2 == 2);
    CHECK(observeCount3 == 2);

    phase = 2;
    obs2.detach();
    obs3.detach();
    a1 <<= 4;
    CHECK(observeCount1 == 1);
    CHECK(observeCount2 == 2);
    CHECK(observeCount3 == 2);
}

TEST_CASE( "NoObserveOnNoChanged" )
{
    ureact::context ctx;
    
    auto a = make_var(&ctx, 1);
    auto b = make_var(&ctx, 1);

    auto product = a * b;
    
    auto expressionString = make_signal(with(a, b, product),
        []( const int a_, const int b_, const int product_ )
        {
            return std::to_string(a_) + " * " + std::to_string(b_) + " = " + std::to_string(product_);
        });
        
    int aObserveCount = 0;
    int bObserveCount = 0;
    int productObserveCount = 0;
    
    observe(a, [&] (int /*v*/)
    {
        ++aObserveCount;
    });
    observe(b, [&] (int /*v*/)
    {
        ++bObserveCount;
    });
    observe(product, [&] (int /*v*/)
    {
        ++productObserveCount;
    });
    
    CHECK(aObserveCount == 0);
    CHECK(bObserveCount == 0);
    CHECK(productObserveCount == 0);
    CHECK(expressionString.value() == "1 * 1 = 1");
    
    b <<= 2;
    CHECK(aObserveCount == 0);
    CHECK(bObserveCount == 1);
    CHECK(productObserveCount == 1);
    CHECK(expressionString.value() == "1 * 2 = 2");
    
    b <<= 2; // Shouldn't change
    CHECK(aObserveCount == 0);
    CHECK(bObserveCount == 1);
    CHECK(productObserveCount == 1);
    CHECK(expressionString.value() == "1 * 2 = 2");
    
    ctx.do_transaction([&]()
    {
        b <<= 1;
        b <<= 2; // Shouldn't change
    });
    CHECK(aObserveCount == 0);
    CHECK(bObserveCount == 1);
    CHECK(productObserveCount == 1);
    CHECK(expressionString.value() == "1 * 2 = 2");
    
    a <<= 0;
    CHECK(aObserveCount == 1);
    CHECK(bObserveCount == 1);
    CHECK(productObserveCount == 2);
    CHECK(expressionString.value() == "0 * 2 = 0");
    
    b <<= 3;
    CHECK(aObserveCount == 1);
    CHECK(bObserveCount == 2);
    CHECK(productObserveCount == 2); // Product shouldn't change
    CHECK(expressionString.value() == "0 * 3 = 0");
}

TEST_CASE( "ScopedObserverTest" )
{
    ureact::context ctx;

    std::vector<int> results;

    auto in = make_var(&ctx, 1);

    {
        ureact::scoped_observer obs = observe(in, [&] (int v) {
            results.push_back(v);
        });

        in <<=2;
    }

    in <<=3;

    CHECK(results.size() == 1);
    CHECK(results[0] == 2);
}

TEST_CASE( "SelfObserverDetachTest" )
{
    ureact::context ctx;
    
    std::vector<int> results;
    
    auto in = make_var(&ctx, 0);
    
    ureact::observer obs = observe(in, [&] (int v) {
        if( v < 0 )
        {
            return ureact::observer_action::stop_and_detach;
        }
        else
        {
            results.push_back(v);
            return ureact::observer_action::next;
        }
    });
    
    in <<= 1;
    in <<= 2;
    in <<= -1;
    in <<= 3;
    
    CHECK(results.size() == 2);
    CHECK(results[0] == 1);
    CHECK(results[1] == 2);
}

TEST_SUITE_END();
