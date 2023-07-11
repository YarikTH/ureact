# Introduction

"The universe as we know it is a joint product of the observer and the
observed." \
**— Teilhard De Chardin**

[Observer pattern](https://en.wikipedia.org/wiki/Observer_pattern) is a great
way to decouple observed object from its observers that can be known only at
runtime.

Description from the Gang of Four to those who don't know it: "Define a
one-to-many dependency between objects so that when one object changes state,
all its dependents are notified and updated automatically".

```
                       ╔════════════╗
                notify ║            ║
                   /──>╢  Observer  ║
                  /    ║            ║
                 /     ╚════════════╝
                /      
╔════════════╗ /       ╔════════════╗
║            ╟/ notify ║            ║
║  Observed  ╟────────>╢  Observer  ║
║            ╟\        ║            ║
╚══════════╤═╝ \       ╚════════════╝
   ^       │    \      
   └───────┘     \     ╔════════════╗
  has changed     \    ║            ║
                   \──>╢  Observer  ║
                notify ║            ║
                       ╚════════════╝ 
```

It is very to understand pattern because we all see "follow" and "subscribe"
buttons literally everywhere.

But what if we try to use observer pattern at scale? At some point, we will
inevitably encounter a situation where two observed/observer pairs are fused
together.

```
                         has changed
                          ┌───────┐
                          v       │
                       ╔══════════╧═╗         ╔════════════╗
                       ║            ║         ║            ║
╔════════════╗         ║  Observed  ╟────────>╢  Observer  ║
║            ║         ╟────────────╢ notify  ║            ║
║  Observed  ╟────────>╢  Observer  ║         ╚════════════╝
║            ║ notify  ║            ║
╚══════════╤═╝         ╚════════════╝
   ^       │
   └───────┘
  has changed
```

Or a way more compact form:

```
╔════════════╗         ╔════════════╗         ╔════════════╗
║            ║         ║  Observed  ║         ║            ║
║  Observed  ╟────────>╣     /      ╟────────>╣  Observer  ║
║            ║ notify  ║  Observer  ║ notify  ║            ║
╚══════════╤═╝         ╚══════════╤═╝         ╚════════════╝
   ^       │              ^       │
   └───────┘              └───────┘
  has changed            has changed
```

To emphasize the blurring of the boundary between the observed and the observer,
let's call both the observed and the observer, and their fusion, the word
"reactor". In the sense of one that changes in response to a stimulus (for e.g.
notification).

```
╔═══════════╗     ╔═══════════╗     ╔═══════════╗
║           ║     ║           ║     ║           ║
║  Reactor  ╟────>╢  Reactor  ╟────>╢  Reactor  ║
║           ║     ║           ║     ║           ║
╚═══════════╝     ╚═══════════╝     ╚═══════════╝
```

From now on "has changed" arrow is omitted, because it is obvious, and it
distracts from important details, "notify" labels are omitted, because all
arrows mean it by themselves and additional label for each line is distracting.
Each double lined rect means reactor, even it contains value, formulae or some
other content. Each of shown reactors can have additional observers not shown on
the graph.

One fusion is not a limit, but only the beginning. Such reactors can form the
Directed Acyclic
Graphs ([DAG](https://en.wikipedia.org/wiki/Directed_acyclic_graph)). Like this:

```
╔═══╗   ╔═══╗   ╔═══════╗
║   ╟──>╢   ╟──>╢       ║
╚═══╝\  ╚═══╝   ╢       ║
      \        /║       ║
       \╔═══╗ / ╚═══════╝
        ╢   ╟X
       /╚═══╝ \ ╔═══════╗
      /        \║       ║
╔═══╗/  ╔═══╗   ╢       ║
║   ╟──>╢   ╟──>╢       ║
╚═══╝   ╚═══╝   ╚═══════╝
```

As you can see reactor can depend on several other reactors and forming of
diamond-like dependencies is not uncommon. We'll discuss problem this presents
below.

##  

```
 Input │    Reactive system    │ Output
       │                       │
     ╔═══╗   ╔═══╗   ╔═══════╗ │
     ║   ╟──>╢   ╟──>╢       ║ │
     ╚═══╝\  ╚═══╝   ╢       ║ │
       │   \        /║       ║ │
       │    \╔═══╗ / ╚═══════╝ │
       │     ╢   ╟X            │
       │    /╚═══╝ \ ╔═══════╗ │
       │   /        \║       ║ │
     ╔═══╗/  ╔═══╗   ╢       ║ │
     ║   ╟──>╢   ╟──>╢       ║ │
     ╚═══╝   ╚═══╝   ╚═══════╝ │
       │                       │
```

## Reactor types

Let's introduce two types of reactors that is used in ureact:

1. `signal<S>` - reactor that hold a value and notify its observers when the
   value has changed. Can automatically recalculate its value when values of its
   dependencies have changed.
   `var_signal<S>` - is a `signal<S>` with additional interface to set its value
   from imperative code.
2. `events<E>` - reactor that notify its observers that something had happened.
   For e.g. button was clicked, mouse cursor was moved to. Can represent not
   only source of events, but some kind of modification of another event stream(
   s) (filter, transform, merge etc).
   `event_source<E>` - is an `events<E>` with additional interface to emit event
   values from imperative code.

### Signal

```
╔═══╗
║ a ╟
╚═══╝\╔═══╗
      ╢sum║
╔═══╗/╚═══╝
║ b ╟
╚═══╝
```

```c++
using namespace ureact;
using namespace ureact::default_context;

var_signal<int> a = make_var( 1 );
signal<int> b = make_const( 2 );
signal<int> sum = a + b;

// Subscribe to receive callback each time sum is changed
observe( sum, []( int s ){ /*print*/ } );

// sum - 3 (1+2)
a.set( 10 ); //< recalculate dependant values here
// sum - 12 (10+2)
```

### Events

```
╔═══╗
║ a ╟
╚═══╝\╔════╗
      ╢both║
╔═══╗/╚════╝
║ b ╟
╚═══╝
```

```c++
using namespace ureact;
using namespace ureact::default_context;

event_source<int> a = make_source<int>();
event_source<int> b = make_source<int>();
events<int> both = merge( a, b );

// Subscribe to receive callback each time both event is received
observe( both, []( int e ){ /*print*/ } );

a.emit( 1 );
b.emit( 2 );
```

## So, what was the problem again?

### Glitch

Dependency topology where the reactor depends on the same reactor several times
directly or indirectly can be normally solved by independent observers.

Example [from Wikipedia](https://en.wikipedia.org/wiki/Reactive_programming#Glitches)
below shows it:

```
╔═════════════╗          1            ╔═════════════╗
║   seconds   ╟──────────────────────>╢g=(t>seconds)║
╚═════════════╩\                     /╩═════════════╝
                \ 2               3 /
                 \ ╔═════════════╗ /
                  >╢t=(seconds+1)╟/
                   ╚═════════════╝
```

Value of `g` reactor should be always `true`, because `s + 1 > s` is
always `true`. Let's ignore integer overflow and floating point overflow cases
here. But if it always true when we use observer pattern?

Let's start from the `seconds` value - 0, `t` value - 1 and `g` value - true.

Observer pattern specifies that after some event happens (value has changed in
particular) in observed it notifies its observers in unspecified order. So,
after changing value of `seconds` from 0 to 1 we have a chose which observer we
notify first.

If we notify `t` first, then it is recalculated as `2` (1+1), then `g` notifies
its own observer `g`, which is calculated as `true` (2 > 1). Then `time`
notifies its second observer `g` and it is recalculated as `true` (2 > 1)
the second time.

If we notify `g` first, then it is recalculated as `false` (1 > 1) and notifies
its own observers because its value has changed from `true` to `false`.
Then `time` notifies its second observer `t`, which is recalculated as `2`,
then `t` notifies its own observer `g`, which is recalculated as `true` (
2 > 1) on the second try, then notifies its own observers because its value has
changed from `false` to `true`.

First scenario leads to double recalculation of `g`, but both calculation is
performed with the same arguments, so we just waste some time. While the second
scenario is problematic, because due to incorrectly chosen order of computations
we recalculated `g` twice, but first time with the old (not yet recalculated)
argument. That resulted a flutter of a value from `true`
to `false` and back (with observer notification about it), while assumable value
should be always `true`. Such effect is known as
a [glitch](https://en.wikipedia.org/wiki/Reactive_programming#Glitches).

To prevent unneeded recalculations and glitches, reactors should stop beeng a
bunch of independent observed/observers pairs, but form a reactive system
instead, where all reactors are aware of each other and about their
dependencies. So, involved reactors could be sorted topologically and
recalculated exactly once, when all their changed dependencies are recalculated.
