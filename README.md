# fmcpp2

A VERY VERY naive [cpp2](https://github.com/hsutter/cppfront) source formatting script, made in an evening without any thought put in it. 
It assumes a relatively well formatted existing code (don’t expect multi-line formatting, or semantics etc..), just deals with indenting and spacing of the most commonly used operators. 

Requires a compiler supporting c++20: 
```
g++ -std=c++20 -O2  fmcpp2.cpp -o fmcpp2 && ./fmcpp2 source_file.cpp2

```
Uncomment   ```//ln.printBeforeAfter(i); ``` to see the line-by-line before/after on the terminal.

For some small cpp2 testing projects, no fancy code, works for me. Use at your own risk.
It's been really helpfull for me, so it might help others.

### Things that might be done right
- [X] Indentation based on { }
- [X] var : int = value;
- [X] var:= value;
- [X] func: (x) -> = { return x };
- [X] call(:() -> value = 1, arg1, arg2); spacing after, and :
- [X] some [], (), {} spacing stuff;
- [X] > < = >> << ++ -- && || != etc spacing stuff;
- [ ] object < T >, treats it like an operator
