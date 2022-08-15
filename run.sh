for threads in 8 16 32 64 128
do
  iter=0
  while [ $iter -lt 3 ]
  do
       ./Finedex_benchmark --thread_num $threads --read 0.5 --insert 0.3 --remove 0.2 --benchmark 5
      iter=`expr $iter + 1`
  done
done

for threads in 8 16 32 64 128
do
  iter=0
  while [ $iter -lt 3 ]
  do
       ./Finedex_benchmark --thread_num $threads --read 0.95 --insert 0.03 --remove 0.02 --benchmark 5
      iter=`expr $iter + 1`
  done
done

for threads in 8 16 32 64 128
do
  iter=0
  while [ $iter -lt 3 ]
  do
       ./Kanva_benchmark --thread_num $threads --read 0.50 --insert 0.30 --remove 0.20 --benchmark 5
      iter=`expr $iter + 1`
  done
done

for threads in 8 16 32 64 128
do
  iter=0
  while [ $iter -lt 3 ]
  do
       ./Kanva_benchmark --thread_num $threads --read 0.95 --insert 0.03 --remove 0.02 --benchmark 5
      iter=`expr $iter + 1`
  done
done
