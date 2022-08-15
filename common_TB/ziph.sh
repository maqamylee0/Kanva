for threads in 8 16 32 64 128
do
  iter=0
  while [ $iter -lt 10 ]
  do
       ./lfabtree $threads 50 30 20 0 0
      iter=`expr $iter + 1`
  done
done

for threads in 8 16 32 64 128
do
  iter=0
  while [ $iter -lt 10 ]
  do
       ./lfabtree $threads 95 3 2 0 0
      iter=`expr $iter + 1`
  done
done

for threads in 8 16 32 64 128
do
  iter=0
  while [ $iter -lt 10 ]
  do
       ./lfabtree $threads 50 30 20 1 0
      iter=`expr $iter + 1`
  done
done

for threads in 8 16 32 64 128
do
  iter=0
  while [ $iter -lt 10 ]
  do
       ./lfabtree $threads 95 3 2 1 0
      iter=`expr $iter + 1`
  done
done

for threads in 8 16 32 64 128
do
  iter=0
  while [ $iter -lt 10 ]
  do
       ./lfabtree $threads 50 30 20 2 0
      iter=`expr $iter + 1`
  done
done

for threads in 8 16 32 64 128
do
  iter=0
  while [ $iter -lt 10 ]
  do
       ./lfabtree $threads 95 3 2 2 0
      iter=`expr $iter + 1`
  done
done

for threads in 8 16 32 64 128
do
  iter=0
  while [ $iter -lt 10 ]
  do
       ./lfabtree $threads 50 30 20 3 0
      iter=`expr $iter + 1`
  done
done

for threads in 8 16 32 64 128
do
  iter=0
  while [ $iter -lt 10 ]
  do
       ./lfabtree $threads 95 3 2 3 0
      iter=`expr $iter + 1`
  done
done