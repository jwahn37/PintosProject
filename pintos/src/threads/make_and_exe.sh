make clean
make -j	4


pintos -v -k -T 60 --qemu  -- -q  run alarm-single
pintos -v -k -T 60 --qemu  -- -q  run alarm-multiple
pintos -v -k -T 60 --qemu  -- -q  run alarm-simultaneous
pintos -v -k -T 60 --qemu  -- -q  run alarm-priority
pintos -v -k -T 60 --qemu  -- -q  run alarm-zero
pintos -v -k -T 60 --qemu  -- -q  run alarm-negative

pintos -v -k -T 60 --qemu  -- -q  run priority-change 
pintos -v -k -T 60 --qemu  -- -q  run priority-change-2 
pintos -v -k -T 60 --qemu  -- -q  run priority-fifo
#pintos -v -k -T 60 --qemu  -- -q  run alarm-priority
#pintos -v -k -T 60 --qemu  -- -q -aging run priority-aging
#pintos -v -k -T 60 --qemu  -- -q  run priority-sema
pintos -v -k -T 60 --qemu  -- -q run priority-lifo
pintos -v -k -T 60 --qemu  -- -q run priority-preempt
pintos -v -k -T 60 --qemu  -- -q run priority-sema
pintos -v -k -T 60 --qemu  -- -q -aging run priority-aging


pintos -v -k -T 480 --qemu  -- -q -mlfqs run mlfqs-load-1
