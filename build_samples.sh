#!/bin/bash
#set -e

cd GBA/samples/basic
make

cd resource
rm -f character_sample1.gbfs
grit texture/character_2head.png  -gB4 -gzl -ftb -pe 16
cat character_2head.pal.bin character_2head.img.bin > character_2head.png
grit texture/character_3head.png  -gB4 -gzl -ftb -pe 16
cat character_3head.pal.bin character_3head.img.bin > character_3head.png
gbfs character_sample1.gbfs character_sample1.ssbp character_2head.png character_3head.png

cd ..
cat basic.gba resource/character_sample1.gbfs > character_sample1.gba

cd ../../..
ls GBA/samples/basic/character_sample1.gba

