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
assert 5 'abc=2; abc+3;'
assert 35 'hoge=foo=(1+5)*2; bar=2; isTrue=0<1; hoge+foo*bar-isTrue;'
assert 123 "hoge = 1; HOGE = 2; hoGe = 3; hoge * 100 + HOGE*10 + hoGe;"

assert 14 "a = 3; b = 5 * 6 - 8; return a + b / 2;"
assert 1 "return 1; return 4;"

echo OK