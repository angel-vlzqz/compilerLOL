To compile the compiler, run 
```make```

To run the compiler, use
```./main_program```

If you use mac and are running into a segmentation
fault when running the program, you will have to use
the built-in `lldb` compiler. Do this by first
running
```lldb ./main_program"```
then, type and enter
```run```
to run the program utilizing mac's built-in debugger