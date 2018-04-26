make clean
make -j20
cd build
#pintos-mkdisk filesys.dsk --filesys-size=2
#pintos -f -q


#pintos -p tests/vm/pt-grow-stack -a pt-grow-stack -- -q
#pintos -q run 'pt-grow-stack'


#pintos -p tests/vm/pt-write-code -a pt-write-code -- -q
#pintos -q run 'pt-write-code'


#pintos -v -k -T 60 --qemu  --filesys-size=2 -p tests/vm/pt-write-code -a pt-write-code --swap-size=4 -- -q  -f run pt-write-code

#pintos -v -k -T 60 --qemu  --filesys-size=2 -p tests/vm/pt-grow-pusha -a pt-grow-pusha --swap-size=4 -- -q  -f run pt-grow-pusha
pintos -v -k -T 60 --qemu  --filesys-size=2 -p tests/vm/pt-grow-stack -a pt-grow-stack --swap-size=4 -- -q  -f run pt-grow-stack
#pintos -v -k -T 60 --qemu  --filesys-size=2 -p tests/vm/pt-bad-addr -a pt-bad-addr --swap-size=4 -- -q  -f run pt-bad-addr
#pintos -v -k -T 60 --qemu  --filesys-size=2 -p tests/vm/pt-bad-read -a pt-bad-read --swap-size=4 -- -q  -f run pt-bad-read
