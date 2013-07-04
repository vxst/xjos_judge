judge:judge.cpp tester
	clang++ judge.cpp -o judge -I/usr/include/mysql -lmysqlpp
tester:tester.cpp
	clang++ tester.cpp -o tester
debug:judge.cpp
	g++ judge.cpp -o judge -I/usr/include/mysql -lmysqlpp -g
clean:
	rm judge
