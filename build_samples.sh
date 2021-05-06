#!/bin/bash
#set -e

cd GBA/samples/basic
make

cd resource
rm -f character_2head.gbfs
grit ../chara_2head/character_2head.png  -gB4 -gzl -ftb -pe 16
cat character_2head.pal.bin character_2head.img.bin > character_2head.png
gbfs chara_2head.gbfs chara_2head.ssbp character_2head.png

cd ..
cat basic.gba resource/chara_2head.gbfs > chara_2head.gba

cd ../../..
ls GBA/samples/basic/chara_2head.gba

