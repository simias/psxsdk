include ../../../Makefile.cfg

data: backgrnd.tim font.tim

backgrnd.tim: backgrnd.bmp
	bmp2tim backgrnd.bmp backgrnd.tim 16 -org=320,0

font.tim: font.bmp
	bmp2tim font.bmp font.tim 4 -org=640,0

clean:
	rm -f *.tim
