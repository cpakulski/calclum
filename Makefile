DEBUG=-g
test:
	g++ scheduler.cc scheduler_test.cc -o scheduler_test -lgmock -lgtest -lgtest_main -lgmock_main \
         -lpthread $(DEBUG)
	./scheduler_test
	g++ frameJob.cc frameJob_test.cc -o frameJob_test -lgmock -lgtest -lgtest_main -lgmock_main \
	 -lpthread $(DEBUG) `pkg-config --cflags --libs opencv`
	./frameJob_test

calclum:
	g++ scheduler.cc calclum.cc frameJob.cc -lpthread $(DEBUG) -o calclum  `pkg-config --cflags --libs opencv`
