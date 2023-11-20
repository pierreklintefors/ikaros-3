# Ranges

Ranges are used to iterate over indices in one or several dimensions.

```C++
range(n) // range of a single value n

range(n,m) // range from n to m-1
```

```C++
range(n,m,i) // range from n to m-1 with step i
```

```C++
range(n,m,-i) // range from n to m-1 with step i in reverse order
```

Range over several dimensions

```C++
range r;
r.push(0, 5); // first dimension goes from 0 to 4
r.push(2, 4); // second dimension goes from 2 to 3
```

Loop over a range in one or several dimensions:

```C++
for(; r.more(); r++)
   std::cout << r << std::endl;
```

Print a range in bracket style by converting to a string:

```C++
std::cout << std::string(r) << std::endl; // will print [0:5][2:4]
```

Set a range from a string:

```C++
range r("[0:5][2:4:-1]");
```

This looks like ranges in Python but the semantics is different. The first value should always be the lower number even for a negative increment. A negative increment will generate exactly the same numbers as a positive but in reverse order.

Examples:

```C++
range r(0,16,7); // generates 0, 7, 14
range r(0,16,-7); // generates 14, 7, 0
```

Print numbers 0 to 4.

```C++
for(range r=range(0,5);r.more();r++)
   std::cout << r << std::endl;
```
