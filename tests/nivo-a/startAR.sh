ASSEMBLER=/home/ss/projekat/assembler
LINKER=/home/ss/projekat/linker
EMULATOR=/home/ss/projekat/emulator

${ASSEMBLER} -o main.o main.s
${ASSEMBLER} -o math.o math.s
${ASSEMBLER} -o handler.o handler.s
${ASSEMBLER} -o isr_timer.o isr_timer.s
${ASSEMBLER} -o isr_terminal.o isr_terminal.s
${ASSEMBLER} -o isr_software.o isr_software.s
${LINKER} -relocatable \
  -place=my_code@0x40000000 -place=math@0xF0000000 \
  -o program.o \
  handler.o math.o isr_terminal.o isr_timer.o isr_software.o
${LINKER} -hex \
  -place=my_code@0x40000000 -place=math@0xF0000000 \
  -o program.hex \
  main.o program.o
${EMULATOR} program.hex