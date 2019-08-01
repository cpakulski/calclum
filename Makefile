test:
	g++ scheduler.cc scheduler_test.cc -o scheduler_test -lgmock -lgtest -lgtest_main -lgmock_main -lpthread -g
	./scheduler_test

calclum:
	g++ scheduler.cc calclum.cc frameJob.cc -lpthread -g -o calclum  `pkg-config --cflags --libs opencv`
