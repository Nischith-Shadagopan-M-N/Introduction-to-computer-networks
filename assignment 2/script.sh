mkdir output
g++ -o cw simulation.cpp
K_i=(1 4)
K_m=(1 1.5)
K_n=(0.5 1)
K_f=(0.1 0.3)
P_s=(0.01 0.0001)

for i in ${!K_i[@]}; do
  for j in ${!K_m[@]}; do
    for k in ${!K_n[@]}; do
      for l in ${!K_f[@]}; do
        for m in ${!P_s[@]}; do
          ./cw -i ${K_i[$i]} -m ${K_m[$j]} -n ${K_n[$k]} -f ${K_f[$l]} -T 3000 -s ${P_s[$m]} -o ./output/output_$i$j$k$l$m
          python3 plot.py ./output/output_$i$j$k$l$m ${K_i[$i]} ${K_m[$j]} ${K_n[$k]} ${K_f[$l]} ${P_s[$m]}
        done
      done
    done
  done
done