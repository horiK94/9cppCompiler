#!/bin/bash
assert(){
  expected="$1"
  input="$2"

  ./main "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "✔ $input => $actual"
  else
    echo "✖ $input => $actual (expected $expected)"
    exit 1
  fi
}

assert 0 "0;"
assert 7 "7;"
assert 7 "+7;"
assert 10 "-10 +20 ;"
assert 21 "5+ 20 -4;"
assert 57 " 12 - 8 +  18 + 35 ;"
assert 47 '5 + 6* 7;'
assert 9 '3*(9 - 6 );'
assert 7 '(3 + 6 * 2) / 2;'
assert 3 '(2 + (2 + 3) * -1 * -1) / 2;'

assert 0 '0==1;'
assert 1 '42 == 42;'
assert 1 '0 !=1;'
assert 0 '42!= 42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'

assert 1 '2+3>4==-1<2-2;'
# assert 5 'a=2; a+3;'
assert 23 'b=a=(1+5)*2; g=1; i=0<1; a+b*i-g;'

echo OK