make clean
make -j20
cd build
pintos-mkdisk filesys.dsk --filesys-size=2
pintos -f -q

#pintos -p ../../examples/echo -a echo -- -q
#pintos -q run 'echo a'

#pintos -p ../../examples/bin -a /bin -- -q

#pintos -p ../../examples/bin/ls -a /bin/ls -- -q
#pintos -v -k -T 360 --qemu  --filesys-size=2 -p tests/userprog/no-vm/multi-oom -a multi-oom -- -q  -f run multi-oom

pintos -p tests/filesys/base/syn-read -a syn-read -- -q
pintos -q run 'syn-read'

#pintos -p tests/userprog/no-vm/multi-oom -a multi-oom -- -q
#pintos -q run 'multi-oom'
#pintos -p ../../examples/child-simple -a child-simple -- -q
#pintos -p ../../examples/halt -a halt -- -q
#pintos -q run 'halt'
#pintos -p ../../examples/ -a sum -- -q
#pintos -q run 'sum 5 5 1 4'
