cd ./build
make -j8
openocd -f ~/ChipCode/stm32/openocd_cfg/stlink.cfg -f ~/ChipCode/stm32/openocd_cfg/stm32l4x.cfg -c "program ./082901_CAS11.elf verify reset exit"
cd ..
