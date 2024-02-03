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

assert 0 0
assert 42 42

echo OK