test:
	g++ scheduler.cc scheduler_test.cc -o scheduler_test -lgtest -lgtest_main -lpthread -g
	./scheduler_test
