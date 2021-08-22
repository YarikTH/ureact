# Signals vs callbacks

Disclaimer: based on [Signals vs. callbacks example from cpp.react](http://snakster.github.io/cpp.react/examples/Signals-vs-callbacks.html)

* [Problem statement](#problem-statement)
* [Solution 1: Simple member function](#solution-1-simple-member-function-run)
* [Solution 2: Manually triggered re-calculation](#solution-2-manually-triggered-re-calculation-run)
* [Solution 3: Callbacks](#solution-3-callbacks-run)
* [Final solution: Signals](#final-solution-signals-run)

This basic example explains the motivation behind signals by comparing them to some alternative approaches.

## Problem statement
Here's a class `Shape` with two dimensions `width` and `height`:

```cpp
class Shape
{
public:
    int width  = 1;
    int height = 1;
};
```

The size of the shape should be calculated accordingly:
```cpp
int calculateSize(int width, int height) { return width * height; }
```

We want to add a method to calculate the size of our shape class.


## Solution 1: Simple member function ([run](https://godbolt.org/z/n3Md58c8Y))

```cpp
class Shape
{
public:
    int width  = 1;
    int height = 1;
    int size() const { return width * height; }
};
```

This gets the job done, but whenever `size()` is called, the calculation is repeated, even if the shape's dimensions did not change after the previous call.
For this simple example that's fine, but let's assume calculating size would be an expensive operation.
We rather want to re-calculate it once after width or height have been changed and just return that result in `size()`.


## Solution 2: Manually triggered re-calculation ([run](https://godbolt.org/z/7fPTG53Eo))

```cpp
class Shape
{
public:
    int width() const   { return m_width; }
    int height() const  { return m_height; }
    int size() const    { return m_size; }

    void setWidth( const int v )
    {
        if( m_width == v ) return;
        m_width = v;
        updateSize();
    }

    void setHeight( const int v )
    {
        if( m_height == v ) return;
        m_height = v;
        updateSize();
    }

private:
    void updateSize()
    {
        m_size = m_width * m_height;
    }

    int m_width  = 1;
    int m_height = 1;
    int m_size   = 1;
};
```

```cpp
Shape myShape;

// Set dimensions
myShape.setWidth(20);
myShape.setHeight(20);

// Get size
const auto curSize = myShape.size();
```

We've declared the data fields as private members and exposed them through getter and setter functions,
so we can call `updateSize()` internally after width or height have been changed.

When considering where we started from, this adds quite a bit of boilerplate code, and as usual, when having to do things manually, we can make mistakes.

What if more dependent attributes should be added?
Using the current approach, updates are manually triggered from the dependencies.

This requires changing all dependencies when adding new dependent values, which gets increasingly complex.
More importantly, it's not an option, if the dependent values are not known yet or could be added and removed dynamically.
A common approach to enable this is using callbacks.


## Solution 3: Callbacks ([run](https://godbolt.org/z/szd718s3E))

```cpp
class Shape
{
public:
    using CallbackT = std::function<void( int )>;

    int width() const   { return m_width; }
    int height() const  { return m_height; }
    int size() const    { return m_size; }

    void setWidth( const int v )
    {
        if( m_width == v ) return;
        m_width = v;
        updateSize();
    }

    void setHeight( const int v )
    {
        if( m_height == v ) return;
        m_height = v;
        updateSize();
    }

    void addSizeChangeCallback( const CallbackT& f )
    {
        m_sizeCallbacks.push_back( f );
    }

private:
    void updateSize()
    {
        const auto oldSize = m_size;
        m_size = m_width * m_height;

        if( oldSize != m_size )
            notifySizeCallbacks();
    }

    void notifySizeCallbacks()
    {
        for( const auto& f : m_sizeCallbacks )
            f( m_size );
    }

    int m_width  = 1;
    int m_height = 1;
    int m_size   = 1;

    std::vector<CallbackT> m_sizeCallbacks;
};
```

```cpp
Shape myShape;

// Callback on change
myShape.addSizeChangeCallback(
    []( const int newSize ) { std::cout << "size -> " << newSize << "\n"; } );
```

For brevity, this example includes callbacks for size changes, but not for width and height.
Nonetheless, it adds even more boilerplate.
Instead of implementing the callback mechanism ourselves, we can use external libraries for that, for example, `boost::signals2`, which handles storage and batch invocation of callbacks;
but overall, it has no impact on the design.

To summarize some pressing issues with the solutions shown so far:

* Error-proneness: There is no guarantee that `size == width * height`. It's only true as long as we don't forget to call `updateSize()` after changes.
* Boilerplate: Check against previous value, trigger update of dependent internal values, trigger callback notification, register callbacks, etc.
* Complexity: Adding new dependent attributes requires changes in existing functions and potentially adding additional callback holders.
* Verbosity: Callback is sent on each change, so if we need to change both width and height, then we either perform two recalculations and two notifications, or we have to add a method to set both width and height. It might be ok for our case, but in more complex cases we'll eventually achieve a combinatorial explosion.

What it boils down to, is that the change propagation must be handled by hand.
The next example shows how signals can be used for this scenario.


## Final solution: Signals ([run](https://godbolt.org/z/x4Gzejs8j))

```cpp
#include "ureact/ureact.hpp"

class Shape
{
public:
    Shape( ureact::context& ctx )
        : width(  ctx.make_value( 1 ) )
        , height( ctx.make_value( 1 ) )
        , size( width * height )
    {}

    ureact::value<int>  width;
    ureact::value<int>  height;
    ureact::signal<int> size;
};
```

`size` now behaves like a function of `width` and `height`, similar to Solution 1.
But behind the scenes, it works like Solution 2, i.e. size is only re-calculated when width or height change.

The following code shows how to interact with these signals:
```cpp
ureact::context ctx;

Shape myShape( ctx );

// Set dimensions
myShape.width.set( 20 );
myShape.height.set( 20 );

// Get size
const auto curSize = myShape.size.get();
```

Every reactive value automatically supports registration of callbacks (they are called observers):
```cpp
// Callback on change
observe( myShape.size,
         []( int newSize )   { std::cout << "size -> "   << newSize   << "\n"; } );

// Those would work, too
observe( myShape.width,
         []( int newWidth )  { std::cout << "width -> "  << newWidth  << "\n"; } );
observe( myShape.height,
         []( int newHeight ) { std::cout << "height -> " << newHeight << "\n"; } );
```

---------------

[Home](readme.md#reference)
