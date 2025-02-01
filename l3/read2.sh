for i in {0..31}
do
cat stats_/statNoInterruptSize$i.txt | grep "board.cache_hierarchy.l1dcaches1.ReadReq.accesses::total"
done
