for j in {0..95..5}; do for i in $(ps aux| grep test_parallel$j | awk '{print($2)}'); do kill -9 $i; done;done;
