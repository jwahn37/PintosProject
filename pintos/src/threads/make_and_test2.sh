make clean
make -j4
cd build

#make tests/threads/mlfqs-load-1.result
#make tests/threads/mlfqs-load-60.result
#make tests/threads/mlfqs-load-avg.result
#make tests/threads/mlfqs-nice-2.result
make tests/threads/mlfqs-nice-10.result
