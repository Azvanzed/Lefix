nasm -f elf64 example/main.asm -o example/main.o
ld -o example/main example/main.o
rm -f example/main.o
chmod +x example/main
./example/main
echo "Exit code: $?"