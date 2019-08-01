test:
	g++ scheduler.cc scheduler_test.cc -o scheduler_test -lgmock -lgtest -lgtest_main -lgmock_main -lpthread -g
	./scheduler_test
