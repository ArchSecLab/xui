for i in {0..31}
do
cat stats/statNoInterruptSize$i.txt | grep "board.cache_hierarchy.l1dcaches1.ReadReq.missRate::total"
done
