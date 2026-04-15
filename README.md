
My organization is fairly messy, the comment removal and tokenization were both done in main before I decied for later projects that I should be using separate files. Though you can see I didn't use header files which isn't best practice.

Code-wise:
1. Comment remover - fine, nothing special, fairly neat I think, though I used numbers for state which makes it less easy to read.
2. Tokenizer - same as comment remover, a little messy at times.
3. Recursive descent - not great, I used recursive function calls rather than a more typical DFA loop which makes it more readable visually, but otherwise it's very repetative and inefficient. 
4. Symbol Table - good I think, I used enum for the states which helps with readability. I was a bit inconsistent with what variables I made public/private so if we went with my symbol table code I would want to clean things up just a little bit.   
