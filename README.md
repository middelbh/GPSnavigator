# GPSnavigator
Super low cost GPS navigation system based on Arduino UNO, 320x240 TFT and GPS unit.

I just added the .ino file. It is based on the topo maps of the Netherlands in 8 bit BMP format. 
You will have to convert the GEOTIFFS from top25raster (www.kadaster.nl) into the BMP format (e.g. with Irfanview).
Put these BMP's on an SD card, together with boudarykl.dat; this last file gives the boundaries of the BMP maps.

Some remarks:
1) not yet completely tested with the GPS unit,
2) problems still exist when at a position on a map at the boundaries, I do not yet have sufficient space to display data from multiple maps in one screen. That's why I need to make the sketch smaller.
3) the file boundarykl.dat still is not optimal; I realised that I need to renew this....
4) if you want to use other maps: that is possible without problems, but you have to georeference them and put the data in boundarykl.dat. Please take care: I only read 304 items from this file at a max at this moment!
