is_prime: main.o
	g++ -fprofile-arcs -ftest-coverage -o $@ $^

main.o: main.cpp
	g++ -fprofile-arcs -ftest-coverage -O0 -c $^

clean:
	@rm -v is_prime *.o *.gcno *.gcda *~
