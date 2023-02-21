gcc -O1 a.c queue.c harness.c report.c console.c linenoise.c web.c -Idudect -I.
./a.out
python3 a.py
rm a.out
rm a.txt
rm b.txt