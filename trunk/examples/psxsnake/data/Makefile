include ../../../Makefile.cfg

data: backgrnd.tim font.tim music.raw bomb.raw apple.raw

backgrnd.tim: backgrnd.bmp
	bmp2tim backgrnd.bmp backgrnd.tim 16 -org=320,0

font.tim: font.bmp
	bmp2tim font.bmp font.tim 4 -org=640,0

music.raw: music.wav
	wav2vag music.wav music.raw -L -raw

bomb.raw: bomb.wav
	wav2vag bomb.wav bomb.raw -raw

apple.raw: apple.wav
	wav2vag apple.wav apple.raw -raw

clean:
	rm -f *.tim
