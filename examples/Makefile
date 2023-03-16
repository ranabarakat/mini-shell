
#Use GNU compiler
cc = gcc -g
CC = g++ -g

all: cat_grep  cat_grep_output ls_output


cat_grep: cat_grep.cc
	$(CC) -o cat_grep cat_grep.cc

cat_grep_output: cat_grep_output.cc
	$(CC) -o cat_grep_output cat_grep_output.cc

ls_output: ls_output.cc
	$(CC) -o ls_output ls_output.cc

clean:
	rm -f ls_output cat_grep cat_grep_output *.o

