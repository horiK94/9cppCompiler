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

assert 0 "0"
assert 7 "7"
assert 21 "5+ 20 -4"
assert 57 " 12 - 8 +  18 + 35 "

echo OK