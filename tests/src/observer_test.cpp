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
        else if (phase == 1)
            CHECK(v == 4);
        else
            CHECK(false);
    });

    auto obs2 = observe(result, [&] (int v)
    {
        observeCount2++;

        if (phase == 0)
            CHECK(v == 3);
        else if (phase == 1)
            CHECK(v == 4);
        else
            CHECK(false);
    });

    auto obs3 = observe(result, [&] (int v)
    {
        observeCount3++;

        if (phase == 0)
            CHECK(v == 3);
        else if (phase == 1)
            CHECK(v == 4);
        else
            CHECK(false);
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

TEST_SUITE_END();
