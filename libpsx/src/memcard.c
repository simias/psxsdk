/*
 * PSXSDK Memory Card Helper Functions
 *
 * These functions help to manage memory card loading/saving
 *
 * Normal file functions can be used to do this, but it will be very tedious...
 */
 
 #include <stdio.h>
 #include <stdlib.h>
 #include <fcntl.h>
 #include <memcard.h>

char fname_buf[256];
char slot_hdr[8192];

void McdDefToHeader(McdSlotDef *def, unsigned char *hdr)
{
	hdr[0] = 'S';
	hdr[1] = 'C';
	hdr[2] = 0x10 + def->num_of_icons;
	hdr[3] = def->num_of_slots;
	strncpy(&hdr[4], def->name, 64);
	memcpy(&hdr[96], def->clut, 32);
	memcpy(&hdr[128], def->icon[0], 128);
	memcpy(&hdr[256], def->icon[1], 128);
	memcpy(&hdr[384], def->icon[2], 128);
}
	
int McdWriteData(McdSlotDef *def, int cardn, unsigned char *data)
{
	int fd,data_len,i;
	
	if(cardn > 1)
		return 0;
	
	if(def->num_of_icons > 3 || def->num_of_icons < 1)
		return 0;
	
	if(def->product_code > 99999)
		def->product_code = 99999;
	
	// ugly hack... but works
	
	strncpy(slot_hdr, def->devel_name, 8);
	slot_hdr[8] = 0;
	
	sprintf(fname_buf, "bu%d0:BISCPS-%05d%-8s", cardn, 
		def->product_code, slot_hdr);
	
	printf("Opening memory card file: %s\n", fname_buf);
	
	McdDefToHeader(def, slot_hdr);
	
	StartCARD();
	
	fd = open(fname_buf, O_CREAT | (def->num_of_slots << 16));
	if(fd == -1)
		return 0;
	
	close(fd);
	
	fd = open(fname_buf, O_WRONLY);
	
	memcpy(slot_hdr+512, data, 8192 - 512);
	write(fd, slot_hdr, 8192);
	
	data += 8192 - 512;
	i = def->num_of_slots - 1;
	
	while(i > 0)
	{
		write(fd, data, 8192);
		data+=8192;
		i--;
	}
	
	close(fd);
	
	StopCARD();
	
	return 1;
}
