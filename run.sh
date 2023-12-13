./test_sqrt ${1}
wait
./test_lam ${1}
wait
./test_factor ${1}
wait
./test_lru ${1}
wait
cd tests
rm cachegrind* test-*