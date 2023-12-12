mkdir experiment
run(){
    B=$1  


    ./test_theo "$B" 

    # D1_misses_unblocked=$(grep -E 'D1  misses:[[:space:]]+[0-9,]+' "valgrind_unblocked_output.txt" | awk '{print $3}' | tr -d ',')
    # D1_misses_blocked=$(grep -E 'D1  misses:[[:space:]]+[0-9,]+' "valgrind_blocked_theo_output.txt" | awk '{print $3}' | tr -d ',')
    # D1_misses_unblocked=$(grep 'D1  misses:[[:space:]]*[0-9,]*' "valgrind_unblocked_output.txt")
    # D1_misses_blocked=$(grep 'D1  misses:[[:space:]]*[0-9,]*' "valgrind_blocked_theo_output.txt")

    # echo "$D1_misses_unblocked" >> ./experiment/D1_misses_unblocked.txt
    # echo "$D1_misses_blocked" >> ./experiment/D1_misses_blocked.txt


}

# Bs=(2 3 4 5 6 7 8)

for ((B = 100; B <= 500; B++))
do
  run "$B" 
done