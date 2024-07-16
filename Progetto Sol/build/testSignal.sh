#!/bin/bash

if [ ! -e generafile ];
then
    echo "Compilare generafile, eseguibile mancante!";
    exit 1
fi
if [ ! -e farm ];
then
    echo "Compilare farm, eseguibile mancante!"
    exit 1
fi

#
# il file expected.txt contiene i risultati attesi per i file
# generati nel seguito con il programma generafile
#
cat > expected.txt <<EOF
64834211 file100.dat
103453975 file2.dat
153259244 file1.dat
193031985 testdir/testdir2/file150.dat
258119464 file116.dat
293718900 file3.dat
380867448 file5.dat
518290132 file17.dat
532900090 file117.dat
584164283 file4.dat
748176663 file16.dat
890024017 testdir/file19.dat
985077644 testdir/file8.dat
1146505381 file10.dat
1485251680 file12.dat
1674267050 file13.dat
1878386755 file14.dat
1884778221 testdir/testdir2/file111.dat
2086317503 file15.dat
2322416554 file18.dat
2560452408 file20.dat
EOF

#
# generafile genera i file file100.dat file150.dat file19.dat file116.dat...
# in modo deterministico
#
j=1
for i in 100 150 19 116 2 1 117 3 5 17 4 16 19 8 10 111 12 13 14 15 18 20; do
    ./generafile file$i.dat $(($i*11 + $j*117)) > /dev/null
    j=$(($j+3))
done

mkdir -p testdir
mv file19.dat file8.dat testdir
mkdir -p testdir/testdir2
mv file111.dat file150.dat testdir/testdir2


./farm -n 100 -d testdir -q 100 -t 1000 file* 2>&1 > /dev/null &
pid=$!
sleep 1
pkill -HUP farm
wait $pid
if [[ $? != 0 ]]; then
    echo "test HUP failed"
else
    echo "test HUP passed"
fi

./farm -n 2 -d testdir -q 23 -t 4 file* 2>&1 > /dev/null &
pid=$!
sleep 1
pkill -INT farm
wait $pid
if [[ $? != 0 ]]; then
    echo "test INT failed"
else
    echo "test INT passed"
fi

./farm -n 20 -d testdir -q 9 -t 4 file* 2>&1 > /dev/null &
pid=$!
sleep 1
pkill -QUIT farm
wait $pid
if [[ $? != 0 ]]; then
    echo "test QUIT failed"
else
    echo "test QUIT passed"
fi

./farm -n 100 -d testdir -q 3 -t 1000 file* 2>&1 > /dev/null &
pid=$!
sleep 1
pkill -TERM farm
wait $pid
if [[ $? != 0 ]]; then
    echo "test TERM failed"
else
    echo "test TERM passed"
fi

./farm -n 10 -d testdir -q 10 file* 2>&1 > /dev/null &
pid=$!
sleep 1
pkill -PIPE farm
wait $pid
if [[ $? != 0 ]]; then
    echo "test PIPE failed"
else
    echo "test PIPE passed"
fi

./farm -n 4 -d testdir -q 2 file* 2>&1 > /dev/null &
pid=$!
sleep 1
pkill -USR1 farm
wait $pid
if [[ $? != 0 ]]; then
    echo "test USR1 failed"
else
    echo "test USR1 passed"
fi