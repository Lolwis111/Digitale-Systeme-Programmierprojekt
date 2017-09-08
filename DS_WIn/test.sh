#!/bin/bash
# automatic tests for bash
# in order for it to compare correctly try convert.sh to adjust line endings to be
# strictly UNIX-Style \n

echo "Testing for results... (expected: nothing)"

# make sure every file you wann test has file extension in
# also make sure that for every in file there is an out file
# (graph.in needs graph.out, error.in needs error.out)

for file in ./data/*.in; # adjust this path to point to the graph files
do	
    echo 'Testing '${file}
    # adjust ./bin/loesung to your executable
    # if else is printed the results are good, if there are differences it will be written
    ./bin/loesung < $file | sort -n | diff -q -- - ${file%in}out
done

echo
echo "Testing for leaks... (expected: nothing)"

for file in ./data/*.in; # also adjust this path to point to the in/out files
do
	echo 'Testing '${file}
	# runs valgrind on all the graphs, if nothing is printed its good
	# if something is printed its because of errors
	# adjust ./bin/loesung to your executable
	valgrind --quiet --leak-check=full ./bin/loesung < $file > /dev/null
done

echo "Testing incorrect data... (expected: errors)"

echo "  Testing empty data"

valgrind --quiet --leak-check=full ./bin/loesung < /dev/null

echo "  Testing random data"

for i in 1 2 3 4 5 6 7 8 9 10;
do
	valgrind --quiet --leak-check=full ./bin/loesung < /dev/urandom
done

echo "  Testing out files"

for file in ./data/*.out; # also adjust this path to point to the out files
do
	echo 'Testing '${file}
	valgrind --quiet --leak-check=full ./bin/loesung < $file
done

echo
echo "All done!"
