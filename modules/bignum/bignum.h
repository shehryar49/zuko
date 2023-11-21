/*MIT License

Copyright (c) 2022 Shahryar Ahmad 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#ifndef BIGNUM_H_
#define BIGNUM_H_

#include <string>
#include <exception>
#include <stdexcept>
#include <algorithm>

using namespace std;
class bignum
{
private:
  string val;
  bool sign;
public:
  bignum();
  bignum(const std::string& str);
  bignum(const bignum& obj);
  bignum operator+(const bignum& rhs)const;
  bignum operator-(const bignum& rhs)const;
  bignum operator/(const bignum& rhs)const;
  bignum operator*(const bignum& rhs)const;
  void addMag(const bignum& rhs);
  bool operator==(const bignum& rhs)const;
  bool operator!=(const bignum& rhs)const;
  bool operator<(const bignum& rhs)const;
  bool operator>(const bignum& rhs)const;
  bool operator<=(const bignum& rhs)const;
  bool operator>=(const bignum& rhs)const;
  void increment();//increments the value of bignum
  void decrement();//decrements the value of bignum
  void incrementMag();//increments magnitude of bignum
  void decrementMag();//decrements magintude of bignum
  string str()const;
};

#endif