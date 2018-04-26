make clean
#make -j20
make
cd build
#make check
#make check -j20

make tests/threads/alarm-single.result
make tests/threads/alarm-multiple.result
make tests/threads/alarm-simultaneous.result
make tests/threads/alarm-priority.result
make tests/threads/alarm-zero.result
make tests/threads/alarm-negative.result

make tests/threads/priority-change.result
make tests/threads/priority-change-2.result
make tests/threads/priority-fifo.result
make tests/threads/priority-lifo.result
make tests/threads/priority-preempt.result
make tests/threads/priority-sema.result
make tests/threads/priority-aging.result

