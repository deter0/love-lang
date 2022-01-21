#!/usr/bin/sh
TESTS=(
	"hello-world.love"
)

echo "Running tests..."

for TEST_FILE in "${TESTS[@]}"; do
	echo
	echo "Running: $TEST_FILE"
	bin/love tests/$TEST_FILE
	nasm -f elf64 out.asm
	ld -o out out.o
	rm out.asm
	rm out.o
	mv out tests/hello-world-love-bin
	echo "Running ./out ..."
	tests/hello-world-love-bin
done
