ASSEMBLER=/home/ss/projekat/assembler
LINKER=/home/ss/projekat/linker
EMULATOR=/home/ss/projekat/emulator

${ASSEMBLER} -o main.o main.s
${ASSEMBLER} -o handler.o handler.s
${ASSEMBLER} -o isr_terminal.o isr_terminal.s
${ASSEMBLER} -o isr_timer.o isr_timer.s
${LINKER} -relocatable \
  -place=code@0x40000000 \
  -o program.o \
  main.o isr_timer.o handler.o
${LINKER} -hex \
  -place=code@0x40000000 \
  -o program.hex \
  program.o isr_terminal.o
${EMULATOR} program.hex