# Matrices

The matrix is the most fundamental data type in Ikaros. It is used for all communication between modules and for computation within every module. A matrix in Ikaros is a multidimensional array of floats.

A matrix is characterised by its rank and its size. The rank describes the dimensionality of the matrix. A normal two-dimensional matrix has rank 2. This can also be called a table. A row of floats has rank 1. And a single scalar value has rank 0. This allows for a matrix algebra over matrices that includes operations that changes the dimensionality of a matrix.

The size of a matrix is the number of values in each dimensions. Foe example, a 2x3 matrix has two rows of three elements each. Its rank is 2.

## Creating a matrix

The simplest way to declare a matrix is as:

```C++
matrix m;
```

This will create an empty matrix of rank 0.

A matrix can also be created with a particular size, for example:

```C++
matrix m(4,2,3);
```

This would create a matrix of rank three with the sizes 4, 2 and 3 for the three dimensions.

Matrices can be initialized from an initialization list.
A two dimensional matrix in Ikaros can be defined inline as

```C++
matrix m = {{1, 2, 3}, {4, 5, 6}};
```

This would create a 2x3 matrix and assign it to the variable m. This type of initialization also works for higher dimensions.

A matrix can be assiged from a string. This only works for one or two-dimensional matrices. A comma is used to separate individual values on a row and a semicolon is used to end a row.

```C++
matrix m = "1, 2; 3 4";
```

## Accessing the elements of a matrix

Elements of a matrix are accessed using parenthesis notation. For a two-dimensional matrix the first index is the row and the second is the column.

```C++
m(1,2) = 42;
x = m(1,2);
```

Submatrices are accessed using square brackets. For a two dimensional matrix the following would access the second row:

```C++
m[1];
```

The same notation works for higher dimensional matrices. This means that elements can also be accessed using square brackets, but this is less efficient and should only be used for backward compatibility with earlier version of Ikaros:

```C++
m[1][2] = 42;
x = m[1][2];
```

## Printing a matrix

A matrix  can be printed using

```C++
std::cout << m << std::endl;
```

Alternatively, the print function can be used:

```C++
m.print();
```

The print function also allows for a variable name to be added:

```C++
m.print("m");
```

## Getting information about a matrix

The internal data structures of a matrix can be printed using info()

```C++
m.info();
```

The rank of a matrix is obtained using the rank function:

```C++
m.rank();
```

The size of each dimension of a matrix is obtained using the shape function:

```C++
std::vector<int> s =  m.shape();
```

The total number of elements in a matrix is given by the size function:

```C++
int s =  m.size();
```

The size of an individual dimension is accessed using the size function with an index:

```C++
int s =  m.size(1);
```

There are number of convenience functions to access eth sizes of two dimensional matrices:

```C++
    m.rows();
    m.cols();
    m.size_x();
    m.size_y();
```

## Naming matrix and dimensions

A matrix can be given a name that is used when it is printed:

```C++
m.set_name ("m")
m.print();
```

Individual elements of a dimensions can also be named:

```C++
m.set_labels(0, "Row1", "Row2");
m.set_labels(1, "Col1", "Col2, Col3");
m.print();
```

To inspect the internal structure of a matrix use the function info() to print all internal values:

```C++
m.info();
```

The JSON representation of atrsing can be obtained by the json() function:

```C++
std::string s = m.json();
```
