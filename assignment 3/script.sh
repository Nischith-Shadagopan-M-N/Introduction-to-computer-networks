g++ -pthread -o ospf router.cpp
mkdir output

for ((i=0; i<$1; i++))
do
   bash -c "exec -a process$i ./ospf -i $i -f $2 -o ./output/$3 -h $4 -a $5 -s $6 &"
done

sleep $7

for ((i=0; i<$1; i++))
do
  pkill -f process$i
done