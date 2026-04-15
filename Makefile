
run: main.o remove_comments.o tokenizer.o
	g++ main.o remove_comments.o tokenizer.o -o run

main.o: main.cpp
	g++ -c main.cpp

tokenizer.o: tokenizer.cpp include/tokenizer.h include/token.h
	g++ -c tokenizer.cpp

remove_comments.o: remove_comments.cpp include/remove_comments.h
	g++ -c remove_comments.cpp

clean:
	rm *.o run