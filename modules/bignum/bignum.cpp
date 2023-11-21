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
#include <stdio.h>
#include <stdlib.h>
#include "bignum.h"
using namespace std;

bignum::bignum()
{
  val = "0";
  sign = false;
}
bignum::bignum(const std::string& str)
{
  if(str.length() == 0)
    throw std::logic_error("empty string not allowed!");
  auto it = str.rbegin();
  auto rend = str.rend();
  sign =false;
  while(it != rend)
  {
      char ch = *it;
      if(ch < '0' || ch > '9') //not a digit
      {
        if(ch == '-' && ++it == rend)
        {
          sign = true;
          break;
        }
        throw std::logic_error("only digits allowed in bignum value!");
      }
      val.push_back(ch);
      it++;
  }
  
}
bignum::bignum(const bignum& obj)
{
  val = obj.val;
  sign = obj.sign;
}
bignum bignum::operator+(const bignum& rhs)const
{
  if(sign && !rhs.sign)
  {
    static bignum aux;
    aux = *this;
    aux.sign = false;
    return rhs - aux;
  }
  else if(!sign && rhs.sign)
  {
    bignum tmp = rhs;
    tmp.sign = false;
    return (*this) - tmp;
  }
  bool ansNeg = sign && rhs.sign;
  bignum res;
  res.val = "";
  size_t i = 0;
  size_t j = 0;
  size_t l1 = this->val.length();
  size_t l2 = rhs.val.length();
  uint8_t carry = 0;
  while(i<l1 && j<l2)
  {
      uint8_t k = (val[i]-48) + (rhs.val[j]-48) + carry;
      if(k>=10)
      {
        k-=10;
        carry = 1;
      }
      else
        carry = 0;
      res.val.push_back((char)(k+48));
      i++;
      j++;
  }
  while(i<l1)
  {
      uint8_t k = val[i]-48 + carry;
      if(k>=10)
      {
      k-=10;
      carry = 1;
      }
      else
        carry = 0;
      res.val.push_back((char)(k+48)); 
      i++;
  }
  while(j<l2)
  {
      uint8_t k = rhs.val[j]-48 + carry;
      if(k>=10)
      {
        k-=10;
        carry = 1;
      }
      else
        carry = 0;
      res.val.push_back((char)(k+48)); 
      j++;
  }
  if(carry)
    res.val.push_back('1');
  res.sign = ansNeg;
  return res;
}
void bignum::addMag(const bignum& rhs)
{
  size_t i = 0;
  size_t j = 0;
  size_t l1 = this->val.length();
  size_t l2 = rhs.val.length();
  uint8_t carry = 0;
  while(i<l1 && j<l2)
  {
      uint8_t k = (val[i]-48) + (rhs.val[j]-48) + carry;
      if(k>=10)
      {
        k-=10;
        carry = 1;
      }
      else
        carry = 0;
      val[i] = (char)(k+48);
      i++;
      j++;
  }
  while(i<l1)
  {
      uint8_t k = val[i]-48 + carry;
      if(k>=10)
      {
      k-=10;
      carry = 1;
      }
      else
        carry = 0;
      val[i] = (char)(k+48);
      i++;
  }
  while(j<l2)
  {
      uint8_t k = rhs.val[j]-48 + carry;
      if(k>=10)
      {
        k-=10;
        carry = 1;
      }
      else
        carry = 0;
      val.push_back((char)(k+48)); 
      j++;
  }
  if(carry)
    val.push_back('1');
}
bool bignum::operator==(const bignum& rhs)const
{
  return (sign == rhs.sign) && (val == rhs.val) ;
}
bool bignum::operator!=(const bignum& rhs)const
{
  return !((sign == rhs.sign) && (val == rhs.val)) ;
}
bool bignum::operator<(const bignum& rhs)const
{
  size_t l1 = val.length();
  size_t l2 = rhs.val.length();
  if(sign && !rhs.sign)
    return true;
  if(!sign && rhs.sign)
    return false;
  //signs are same

  if(l1 < l2 )
    return !sign;
  else if(l1 > l2)
    return sign;
  //lengths are equal
  size_t k = l1 - 1;
  size_t i = 1;

  while(i<=l1)
  {
    //compare most significant digits
    if(val[k] > rhs.val[k])
      return sign;
    else if(val[k] < rhs.val[k])
      return !sign;
    i++;
    k--;
  }
  //are equal
  return false;

}
bool bignum::operator<=(const bignum& rhs)const
{
  size_t l1 = val.length();
  size_t l2 = rhs.val.length();
  if(sign && !rhs.sign)
    return true;
  if(!sign && rhs.sign)
    return false;
  //signs are same

  if(l1 < l2 )
    return !sign;
  else if(l1 > l2)
    return sign;
  //lengths are equal
  size_t k = l1 - 1;
  size_t i = 1;
  while(i<=l1)
  {
    if(val[k] > rhs.val[k])
      return sign; 
    //not necessary but function finishes quicker due to the following
    else if(val[k] < rhs.val[k])
      return !sign;
    i++;
    k--;
  }
  return true;
}
bool bignum::operator>(const bignum& rhs)const
{
    return !(*this <= rhs);
}
bool bignum::operator>=(const bignum& rhs)const
{
  return !(*this < rhs);
}
bignum bignum::operator-(const bignum& rhs)const
{
  if(sign && !rhs.sign)
  {
    bignum a = *this;
    bignum b = rhs;
    a.sign = b.sign = false;
    bignum res =a+b;
    res.sign = true;
    return res;
  }
  if(!sign && rhs.sign)
  {
    bignum tmp = rhs;
    tmp.sign = false;
    return *this + tmp;
  }
  if(sign && rhs.sign)
  {
    bignum a = *this;
    bignum b = rhs;
    a.sign = b.sign = false;
    return b - a;
  }
  static string a,b;
  bignum res;
  res.val = "";
  if(*this < rhs)
  {
    a = rhs.val;
    b = val;
    res.sign = true;
  }
  else
  {
    a = val;
    b = rhs.val;
  }
  //compute a-b
  
  int8_t borrow = 0;
  size_t l1 = a.length();
  size_t l2 = b.length();
  size_t i = 0;
  size_t j = 0;
  while(i<l1 && j<l2)
  {
    int8_t ans = (a[i]-48) - (b[j]-48) + borrow;
    if(ans < 0)
    {
      borrow = -1;
      ans += 10;
    }
    else
      borrow = 0;
    
    res.val.push_back((char)(48+ans));
    i++;
    j++;
  }

  //if anything has greater length, it's a
  while(i < l1)
  {
    int8_t ans = (a[i] - 48) + borrow;
    if(ans < 0)
    {
      borrow = -1;
      ans += 10;
    }
    else
      borrow = 0;
    res.val.push_back((char)(48+ans));
    i++;
  }
  while(res.val.length() > 1 && res.val.back() == '0')
    res.val.pop_back();
  return res;
}
bignum bignum::operator/(const bignum& rhs)const
{
  //Not the best algo
  //but will be improved in future
  bool ansNeg = sign ^ rhs.sign;
  bignum a = *this;
  bignum b = rhs;
  a.sign = b.sign = false;
  bignum ans;
  while(a >= b)
  {
    a = a-b;
    ans.increment();
  }
  ans.sign = ansNeg;
  return ans;
}
bignum bignum::operator*(const bignum& rhs)const
{
  //Also not the best algo
  //will improve in future
  bool ansNeg = sign ^ rhs.sign;
  size_t l1 = val.length();
  size_t l2 = rhs.val.length();
  if(l1 > l2)
    return rhs * (*this);
  bignum res;
  bignum row;
  static int8_t mods[] = {
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9
};
  static int8_t divs[] = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  3,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  4,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  6,
  6,
  6,
  6,
  6,
  6,
  6,
  6,
  6,
  6,
  7,
  7,
  7,
  7,
  7,
  7,
  7,
  7,
  7,
  7,
  8,
  8,
  8,
  8,
  8,
  8,
  8,
  8,
  8,
  8
};
  for(size_t i=0;i<l1;i++)
  {
    char ch1 = val[i];
    row.val.erase(row.val.begin()+i,row.val.end());
    int8_t carry = 0;
 //   printf("ch1 = %c\n",ch1);
    for(size_t j=0;j<l2;j++)
    {
      char ch2 = rhs.val[j];
      int8_t ans = (ch1 - 48) * (ch2 - 48) + carry;
   //   printf("%c * %c = %d\n",ch1,ch2,ans);
      row.val.push_back(48 + mods[ans]);
      carry = divs[ans];
    }
    if(carry)
      row.val.push_back(48 + carry);
  //  printf("row.val = %s\n",row.str().c_str());
    //res = res + row;
    res.addMag(row);
    row.val.insert(row.val.begin(),'0');
  }
  res.sign = ansNeg;
  return res;
}

void bignum::decrementMag()
{
  if(val == "0" || (val == "1" && sign))
    return;//we dont want to do any sign changing operation
  int8_t borrow = -1;
  size_t i =0;
  size_t l = val.length();
  while(i < l && borrow!=0)
  {
    int8_t ans = (val[i] - 48) + borrow;
    if(ans < 0)
    {
      borrow = -1;
      val[i] = 58 + ans;
    }
    else
    {
      val[i] = 48+ans;
      borrow = 0;
    }
    i++;
  }
  while(val.back() == '0' && val.length()>1)
   val.pop_back();
}
void bignum::incrementMag()//increments magnitude without taking into account the sign
{
  uint8_t carry = 1;
  size_t i = 0;
  size_t l = val.length();
  while(i<l)
  {
    uint8_t k = (val[i] - 48) + carry;
    if(k >= 10)
    {
      k-=10;
      carry = 1;
      val[i] = (char)(k+48);
    }
    else
    {
      val[i] = (char)(k+48);
      carry = 0;
      break;
    }
    i++;
  }
  if(carry)
    val.push_back('1');
}
void bignum::increment()
{
  if(sign)
  {
    if(val == "1")
    {
      val = "0";
      sign = false;
    }
    else
      decrementMag();
    return;
  }
  uint8_t carry = 1;
  size_t i = 0;
  size_t l = val.length();
  while(i<l)
  {
    uint8_t k = (val[i] - 48) + carry;
    if(k >= 10)
    {
      k-=10;
      carry = 1;
      val[i] = (char)(k+48);
    }
    else
    {
      val[i] = (char)(k+48);
      carry = 0;
      break;
    }
    i++;
  }
  if(carry)
    val.push_back('1');

}
void bignum::decrement()
{
  if(sign)
  {
    incrementMag();
    return;
  }
  if(val == "0")
  {
    val = "1";
    sign = true;
    return;
  }
  int8_t borrow = -1;
  size_t i =0;
  size_t l = val.length();
  while(i < l && borrow!=0)
  {
    int8_t ans = (val[i] - 48) + borrow;
    if(ans < 0)
    {
      borrow = -1;
      val[i] = 58 + ans;
    }
    else
    {
      val[i] = 48+ans;
      borrow = 0;
    }
    i++;
  }
  while(val.back() == '0' && val.length()>1)
   val.pop_back();

}
string bignum::str()const
{
  string tmp =  this->val;
  std::reverse(tmp.begin(),tmp.end());
  if(sign)
    tmp = "-"+tmp;
  return tmp;
}

//Testing and stuff
void panic(bool b,const char* str)
{
  if(!b)
  {
    fprintf(stderr,"%s\n",str);

    exit(1);
  }
}

//Testing
void test_init()  
{
  printf("--Test Initialization--\n");
  bignum x("34834");
  panic(x.str() == "34834","test_init failed");
  bignum y("-1234");
  panic(y.str() == "-1234","test_init failed");
  bignum z;
  panic(z.str() == "0","test_init failed");
  puts("(Success)");
}
void test_cmp()
{
  printf("--Test Comparision Operators--\n");
  // <
  //+ve +ve
  bignum a("432");
  bignum b("543");
  panic(a<b,"test_cmp failed");
  //+ve -ve
  bignum c("400");
  bignum d("-760");
  panic(!(c<d),"test_cmp failed");
  //-ve +ve
  bignum e("-497");
  bignum f("343333");
  panic(e<f,"test_cmp failed");
  // -ve -ve
  bignum g("-10");
  bignum h("-20");
  panic(!(g<h),"test_cmp failed");  
  // <=
  //+ve +ve
  panic(a<=b,"test_cmp failed");
  //+ve -ve
  panic(!(c<=d),"test_cmp failed");
  //-ve +ve
  panic(e<=f,"test_cmp failed");
  // -ve -ve
  panic(!(g<=h),"test_cmp failed"); 
  //other operators use these two so testing them is not required
  puts("(Success)");
}
void test_addition()
{
  printf("--Test Addition--\n");
  //+ve + +ve
  bignum a("399");
  bignum b("101");
  panic((a+b).str() == "500","test_addtion(1) failed");
  //+ve + -ve
  bignum c("499");
  bignum d("-300");
  panic((c+d).str() == "199","test_addtion(2) failed");
  //-ve + +ve
  bignum e("-439");
  bignum f("109");
  panic((e+f).str() == "-330","test_addtion(3) failed");
  //-ve + -ve
  bignum g("-1032");
  bignum h("-68");
  panic((g+h).str() == "-1100","test_addtion(4) failed");
  puts("(Success)");
}
void test_subtraction()
{
  printf("--Test subtraction--\n");
  //+ve - +ve
  bignum a("3000");
  bignum b("110");
  panic((a-b).str() == "2890","test_subtraction failed");
  //-ve - +ve
  bignum c("-120");
  bignum d("100");
  panic((c-d).str() == "-220","test_subtraction failed");
  //+ve - -ve
  bignum e("4301");
  bignum f("-99");
  panic((e-f).str() == "4400","test_subtraction failed.");
  //-ve - -ve
  bignum g("-4203");
  bignum h("-204");
  panic((g-h).str()=="-3999","test_subtraction failed");

  puts("(Success)");
}
void test_div()
{
  puts("--Test Division--");
  bignum a("500");
  bignum b("30");
  panic((a/b).str() == "16","test_div failed");
  puts("(Success)");
}
void test_increment()
{
  puts("--Test increment--");
  //+ve
  bignum a("340");
  a.increment();
  panic(a.str() == "341","test_increment(1) failed");
  //-ve
  bignum b("-1");
  b.increment();
  panic(b.str() == "0","test_increment(2) failed");
  //zero
  bignum c("0");
  c.increment();
  panic(c.str() == "1","test_increment(3) failed");
  //
  puts("(Success)");
}
void test_incrementMag()
{
  puts("--Test incrementMag--");
  //+ve
  bignum a("340");
  a.incrementMag();
  panic(a.str() == "341","test_incrementMag(1) failed");
  //-ve
  bignum b("-1");
  b.incrementMag();
  panic(b.str() == "-2","test_incrementMag(2) failed");
  //zero
  bignum c("0");
  c.incrementMag();
  panic(c.str() == "1","test_incrementMag(3) failed");
  //
  puts("(Success)");
}
void test_decrement()
{
  puts("--Test decrement--");
  //+ve
  bignum a("340");
  a.decrement();
  panic(a.str() == "339","test_decrement(1) failed");
  //-ve
  bignum b("-1");
  b.decrement();
  panic(b.str() == "-2","test_decrement(2) failed");
  //zero
  bignum c("0");
  c.decrement();
  panic(c.str() == "-1","test_decrement(3) failed");
  //
  puts("(Success)");
}
void test_decrementMag()
{
  puts("--Test decrementMag--");
  //+ve
  bignum a("340");
  a.decrementMag();
  panic(a.str() == "339","test_decrement(1) failed");
  //-ve
  bignum b("-1");
  b.decrementMag();
  panic(b.str() == "-1","test_decrementMag(2) failed");
  //zero
  bignum c("0");
  c.decrementMag();
  panic(c.str() == "0","test_decrementMag(3) failed");
  //
  puts("(Success)");
}
void test_fwdloop()
{
  puts("--Test fwdloop--");
  bignum k("-10000");
  bignum l("10000");
  int i = -10000;
  while(k <= l)
  {
;
    panic(k.str() == to_string(i),"test_fwdloop failed");
    k.increment();
    i++;
  }
  puts("(Success)");
}
void test_revloop()
{
  puts("--Test revloop--");
  bignum k("10000");
  bignum l("-10000");
  int i = 10000;
  while(k >= l)
  {
    panic(k.str() == to_string(i),"test_revloop failed");
    k.decrement();
    i--;
  }
  puts("(Success)");
}
void test_mul()
{
  puts("--Test multiplication--");
 //+ve * +ve
 bignum m("123");
 bignum n("456");
 panic((m*n).str() == "56088","test_mul(1) failed");
 bignum a("3489348348");
 bignum b("-348943898348934");
 bignum c = a*b;
 panic(c.str() == "-1217586815248532780461032","test_mul(2) failed");
 puts("(Success)");
}
void test_addmethod()
{
  bignum a("99777");
  bignum   b("223");
  panic((a+b).str() == "100000","test addmethod(1) failed");
}
//
void benchmark1()
{
  bignum n("1000");
  bignum k("1");
  bignum ans("1");
  while(k <= n)
  {
    ans = ans * k;
    k.incrementMag();//faster
  }
  printf("%s\n",ans.str().c_str());
}
