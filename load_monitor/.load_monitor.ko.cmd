cmd_/root/CLionProjects/kernel_study/load_monitor/load_monitor.ko := ld -r -m elf_x86_64  -z max-page-size=0x200000  --build-id  -T ./scripts/module-common.lds -o /root/CLionProjects/kernel_study/load_monitor/load_monitor.ko /root/CLionProjects/kernel_study/load_monitor/load_monitor.o /root/CLionProjects/kernel_study/load_monitor/load_monitor.mod.o;  true