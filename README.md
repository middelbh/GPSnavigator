# GPSnavigator
Super low cost GPS navigation system based on Arduino UNO, 320x240 TFT and GPS unit.I bought these at Banggood.com.

I now added the .ino file. It is based on the topo maps of the Netherlands in 8 bit BMP format. 
You will have to convert the GEOTIFFS from top25raster (www.kadaster.nl) into the BMP format (e.g. with Irfanview).
Put these BMP's on an SD card, together with boundkl.dat; this last file gives the boundaries of the BMP maps.

Some remarks:

1) now tested with the GPS unit,

2) problems still exist when at a position on a map at the boundaries, I do not yet have sufficient space to display data       from multiple maps in one screen. That's why I need to make the sketch smaller.

3) the contents of the file boundkl.dat still is not completely correct; I realised that I need to renew this. Will take some    time.

4) if you want to use other maps: that is possible without problems, but you have to georeference them and put the data in      boundkl.dat. Please take care: I only read 304 items from this file at a max at this moment!

5) the data in boundkl.dat must be:

   for positions 53.4984780814,05.5976731524
   
   and 53.3848795756,05.9898364354
   
   you should see:
   
    534984780814,533848795756,055976731524,059898364354,01H.bmp
    
    top talitude (12 characters, no decimal comma or period)
    
    comma
    
    bottom latitude (12 characters, no decimal comma or period)
    
    comma
    
    left longitude (12 characters, no decimal comma or period)
    
    comma
    
    right longitude (12 characters, no decimal comma or period)
    
    comma
    
    bmp filename (3 characters, a period and then BMP)  
    
6) please note that there are only two positions in front of the period; if you are at another side of our globe you will       have to change some parts in the program!
