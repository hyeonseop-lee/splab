/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
  //
  // It's trivial.
  //
  return ~((~x) | (~y));
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
  //
  // It's trivial.
  //
  return (x >> (n << 3)) & 0xFF;
}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
  //
  // Records sign at t. Than make MSB 0, shift, recover MSB.
  //
  int t = (x >> 31) & 1;
  x = x ^ (t << 31);
  x = x >> n;
  x = x ^ (t << (32 + (~n)));
  return x;
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
  //
  // Used Divide-and-Conquer Method.
  //
  // ex)
  // 1074260208 = 0x4007e8f0 = 0b01000000000001111110100011110000
  //
  // 0 1 0 0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 0 1 0 0 0 1 1 1 1 0 0 0 0
  // 0+1 0+0 0+0 0+0 0+0 0+0 0+1 1+1 1+1 1+0 1+0 0+0 1+1 1+1 0+0 0+0
  //  01 00   00 00   00 00   01 10   10 01   01 00   10 10   00 00
  //  01+00   00+00   00+00   01+10   10+01   01+00   10+10   00+00
  //    0001 0000       0000 0011       0011 0001       0100 0000
  //    0001+0000       0000+0011       0011+0001       0100+0000
  //        00000001 00000011               00000100 00000100
  //        00000001+00000011               00000100+00000100
  //                0000000000000100 0000000000001000
  //                0000000000000100+0000000000001000
  //
  // 0b00000000000000000000000000001100 = 12
  //
  /*
  x = (x & 0x55555555) + ((x >> 1) & 0x55555555);
  x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
  x = (x & 0x0F0F0F0F) + ((x >> 4) & 0x0F0F0F0F);
  x = (x & 0x00FF00FF) + ((x >> 8) & 0x00FF00FF);
  x = (x & 0x0000FFFF) + ((x >> 16) & 0x0000FFFF);
  */
  //
  // Just same as the source above.
  // Used offset and mask variables o, m to avoid using big constant like 0x55555555.
  // Nothing matters with Arithmetic Shift because of & operations.
  //
  int o, m;

  o = 0x55 | (0x55 << 8);
  m = o | (o << 16);
  x = (x & m) + ((x >> 1) & m);

  o = 0x33 | (0x33 << 8);
  m = o | (o << 16);
  x = (x & m) + ((x >> 2) & m);

  o = 0x0F | (0x0F << 8);
  m = o | (o << 16);
  x = (x & m) + ((x >> 4) & m);

  m = 0xFF | (0xFF << 16);
  x = (x & m) + ((x >> 8) & m);

  m = 0xFF | (0xFF << 8);
  x = (x & m) + ((x >> 16) & m);

  return x;
}
/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
  //
  // Used Divide-and-Conquer Method.
  // Calculating with & or | is equivalent, but & is more likely to get answer.
  // Nothing matters with Arithmetic Shift because of & operation with 1.
  //
  x = ~x;
  x = x & (x >> 16);
  x = x & (x >> 8);
  x = x & (x >> 4);
  x = x & (x >> 2);
  x = x & (x >> 1);
  return x & 1;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  //
  // It's trivial.
  //
  return 1 << 31;
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
  //
  // Expression range of n-bit two's complement integer is [1 << (n - 1) - 1, -1 << (n - 1)].
  // Shifting right n - 1 bits integer within the range, we'll get 0 (positive) or -1 (negative).
  //
  x = x >> (n + (~0));
  return (!x) | (!(x + 1));
}
/* 
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int divpwr2(int x, int n) {
  //
  // Record rightside bits at rb.
  // Plus one to result only if x is negative and rb != 0 (Rounding Up toward 0).
  //
  int rb = x & ((1 << n) + (~0));
  return (x >> n) + ((x >> 31) & (1 ^ (!rb)));
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  //
  // It's trivial.
  //
  return (~x) + 1;
}
/* 
 * isPositive - return 1 if x > 0, return 0 otherwise 
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 3
 */
int isPositive(int x) {
  //
  // Returns Zero if Negative or Zero.
  //
  return ((x >> 31) + 1) ^ (!x);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  //
  // Because of overflow, we should signs first.
  // ps = 1 if x < y by sign, ns = 1 if y < x by sign.
  // Return value is 1 if ps = 1, 0 if ns = 1, compare x and y otherwise.
  //
  int ps = ((x >> 31) & 1) & ((y >> 31) + 1);
  int ns = ((x >> 31) + 1) & ((y >> 31) & 1);
  int ss = (((~x) + y + 1) >> 31) + 1;
  return (ps | ss) & (1 ^ ns);
}
/*
 * ilog2 - return floor(log base 2 of x), where x > 0
 *   Example: ilog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
int ilog2(int x) {
  //
  // Instead of counting bits, dimidiated maximum digits.
  // c is for counting digits, x is for dimidiated result.
  //
  int c = 0;
  int r = 0;

  r = ((!(x >> 16)) + (~0)) & 16;
  c = c + r;
  x = x >> r;

  r = ((!(x >> 8)) + (~0)) & 8;
  c = c + r;
  x = x >> r;

  r = ((!(x >> 4)) + (~0)) & 4;
  c = c + r;
  x = x >> r;

  r = ((!(x >> 2)) + (~0)) & 2;
  c = c + r;
  x = x >> r;

  r = ((!(x >> 1)) + (~0)) & 1;
  c = c + r;
  x = x >> r;

  /*
  r = ((!x) + (~0)) & 1;
  c = c + r;
  */

  return c;
}
/* 
 * float_neg - Return bit-level equivalent of expression -f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_neg(unsigned uf) {
  //
  // Calculate negative unless uf is NaN.
  //
  if(((~uf) & 0x7F800000) || !(uf & 0x007FFFFF))
  {
    uf = uf ^ (1 << 31);
  }
  return uf;
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
  //
  // Just as int-to-float instructions.
  // Used unsigned int variable t, f to avoid Arithmetic Shift problems.
  //
  int b = 0, s = 0, c = 0;
  unsigned int r = 0x00000000;
  unsigned int t = x, f = x;
  // Use absolute value if negative.
  if(x >> 31)
  {
    r = 0x80000000;
    t = f = (~x) + 1;
  }
  // Count digits.
  while(t)
  {
    b = b + 1;
    t = t >> 1;
  }
  s = 24 - b;
  // Check digit loss (in terms of precision).
  if(s < 0)
  {
    s = 0;
    while(f >> 25)
    {
      c = c | (f & 1);
      f = f >> 1;
    }
    f = (f >> 1) + (f & (c | (f >> 1)) & 1);
    if(f >> 24)
    {
      b = b + 1;
    }
  }
  // Make Exp 0 if x is 0.
  if(!b)
  {
    b = -126;
  }
  r = r | ((f << s) & 0x007FFFFF);
  r = r | ((b + 126) << 23);
  return r;
}
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
  //
  // Case-by-case.
  //
  int e = (uf >> 23) & 0xFF;
  // Denormalized Case
  if(!e)
  {
    e = 0xFF;
    // Significand overflow is okay; Automatically managed as Normalized Case.
    uf = (uf & 0x80000000) | (uf << 1);
  }
  // Overflow Case
  if(!(e ^ 0xFE))
  {
    e = 0xFF;
    // Set Infinite, not NaN
    uf = (uf & 0x80000000) | (e << 23);
  }
  // Nothing to process if Exp = 0xFF
  if(e ^ 0xFF)
  {
    uf = uf + (1 << 23);
  }
  return uf;
}
