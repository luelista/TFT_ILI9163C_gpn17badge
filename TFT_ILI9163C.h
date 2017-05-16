/*
	ILI9163C - A fast SPI driver for TFT that use Ilitek ILI9163C.
	
	Features:
	- Very FAST!, expecially with Teensy 3.x where uses hyper optimized SPI.
	- It uses just 4 or 5 wires.
	- Compatible at command level with Adafruit display series so it's easy to adapt existing code.
	- It uses the standard Adafruit_GFX Library (you need to install). 
	
	Background:
	I got one of those displays from a chinese ebay seller but unfortunatly I cannot get
	any working library so I decided to hack it. ILI9163C looks pretty similar to other 
	display driver but it uses it's own commands so it's tricky to work with it unlsess you
	carefully fight with his gigantic and not so clever datasheet.
	My display it's a 1.44"", 128x128 that suppose to substitute Nokia 5110 LCD and here's the 
	first confusion! Many sellers claim that it's compatible with Nokia 5110 (that use a philips
	controller) but the only similarity it's the pin names since that this one it's color and
	have totally different controller that's not compatible.
	http://www.ebay.com/itm/Replace-Nokia-5110-LCD-1-44-Red-Serial-128X128-SPI-Color-TFT-LCD-Display-Module-/141196897388
	http://www.elecrow.com/144-128x-128-tft-lcd-with-spi-interface-p-855.html
	Pay attention that   can drive different resolutions and your display can be
	160*128 or whatever, also there's a strain of this display with a black PCB that a friend of mine
	got some weeks ago and need some small changes in library to get working.
	If you look at TFT_ILI9163C.h file you can add your modifications and let me know so I
	can include for future versions.
	
	Code Optimizations:
	The purpose of this library it's SPEED. I have tried to use hardware optimized calls
	where was possible and results are quite good for most applications, actually nly filled circles
    are still a bit slow. Many SPI call has been optimized by reduce un-needed triggers to RS and CS
	lines. Of course it can be improved so feel free to add suggestions.
	-------------------------------------------------------------------------------
    Copyright (c) 2014, .S.U.M.O.T.O.Y., coded by Max MC Costa.
    Modified by Jan--Henrik, Jan Henrik

    TFT_ILI9163C Library is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TFT_ILI9163C Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    This file needs the following Libraries:
 
    Adafruit_GFX by Adafruit:
    https://github.com/adafruit/Adafruit-GFX-Library
	Remember to update GFX library often to have more features with this library!
	From this version I'm using my version of Adafruit_GFX library:
	https://github.com/sumotoy/Adafruit-GFX-Library
	It has faster char rendering and some small little optimizations but you can
	choose one of the two freely since are both fully compatible.
	''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
	Special Thanks:
	Thanks Adafruit for his Adafruit_GFX!
	Thanks to Paul Stoffregen for his beautiful Teensy3 and DMA SPI.
	
	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	Version:

	0.1a1: First release, compile correctly. Altrough not fully working!
	0.1a3: Better but still some addressing problems.
	0.1b1: Beta! Addressing solved, now rotation works and boundaries ok.
	0.2b1: Cleaned up.
	0.2b3: Added 2.2" Red PCB parameters
	0.2b4: Bug fixes, added colorSpace (for future send image)
	0.2b5: Cleaning
	0.3b1: Complete rework on Teensy SPI based on Paul Stoffregen work
	SPI transaction,added BLACK TAG 2.2 display
	0.3b2: Minor fix, load 24bit image, Added conversion utility
	0.4:	some improvement, new ballistic gauge example!
	0.5:	Added scroll and more commands, optimizations
	0.6:	Small fix, added SD example and subroutines
	0.6b1:  Fix clearscreen, missed a parameter.
	0.6b2:  Scroll completed. (thanks Masuda)
	0.6b3:	Clear Screen fix v2. Added Idle mode.
	0.7:    Init correction.Clear Screen fix v3 (last time?)
	0.75:   SPI transactions for arduino's (beta)
	0.8:	Compatiblke with IDE 1.0.6 (teensyduino 1.20) and IDE 1.6.x (teensyduino 1.21b)
	0.9:    Many changes! Now works with more CPU's, alternative pins for Teensy and Teensy LC
	Works (in standard SPI) with Teensy LC.
	ESPVersion 1.0 Fast on the esp
	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	BugList of the current version:
	
	Please report any!

	
Here's the speed test between 0.2b5 and 0.3b1 on Teensy3.1 (major SPI changes)
------------------------------------------------------------------------
Lines                    17024  	16115	BETTER
Horiz/Vert Lines         5360		5080	BETTER
Rectangles (outline)     4384		4217	BETTER
Rectangles (filled)      96315		91265	BETTER
Circles (filled)         16053		15829	LITTLE BETTER
Circles (outline)        11540		20320	WORST!
Triangles (outline)      5359		5143	BETTER
Triangles (filled)       19088		18741	BETTER
Rounded rects (outline)  8681		12498	LITTLE WORST
Rounded rects (filled)   105453		100213	BETTER
Done!

This Library has a Framebuffer added, the esp8266 is too slow for normal usage D:
*/
#ifndef _TFT_ILI9163CLIB_H_
#define _TFT_ILI9163CLIB_H_

#include "Arduino.h"
#include "Print.h"
#include <Adafruit_GFX.h>

#include "_settings/TFT_ILI9163C_settings.h"

//--------- Keep out hands from here!-------------

#define	BLACK   		0x0000
#define WHITE   		0xFFFF

#include "_settings/TFT_ILI9163C_registers.h"

#define FRAMEBUFFER


class TFT_ILI9163C : public Adafruit_GFX {

 public:

	TFT_ILI9163C(uint8_t cspin,uint8_t dcpin,uint8_t rstpin=255);
	
	void     	begin(void),
				setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1),//graphic Addressing
				setCursor(int16_t x,int16_t y),//char addressing
				drawPixel(int16_t x, int16_t y, uint16_t color),
				setRotation(uint8_t r),
				invertDisplay(boolean i);
	uint8_t 	errorCode(void);			
	void		idleMode(boolean onOff);
	void		display(boolean onOff);	
	void		sleepMode(boolean mode);
	void 		defineScrollArea(uint16_t tfa, uint16_t bfa);
	void		scroll(uint16_t adrs);
	void 		startPushData(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
	void 		pushData(uint16_t color);
	void 		endPushData();
	void		writeFramebuffer();
	void		writeRow(uint16_t row, uint16_t row_start, uint16_t row_len, uint8_t * rowdata);
#ifndef FRAMEBUFFER
    void        drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;
    void        fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
#endif
	void		writeScreen24(const uint32_t *bitmap,uint16_t size=_TFTWIDTH*_TFTHEIGHT*2);
	void		writeScreen16(uint8_t *bitmap,uint32_t size=_TFTWIDTH*_TFTHEIGHT*2);
	inline uint16_t Color565(uint8_t r, uint8_t g, uint8_t b) {return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);};
  //convert 24bit color into packet 16 bit one (credits for this are all mine)
	inline uint16_t Color24To565(int32_t color_) { return ((((color_ >> 16) & 0xFF) / 8) << 11) | ((((color_ >> 8) & 0xFF) / 4) << 5) | (((color_) &  0xFF) / 8);}
	void 		setBitrate(uint32_t n);	
 protected:
	volatile uint8_t		_Mactrl_Data;//container for the memory access control data
	uint8_t		_colorspaceData;
	uint8_t 	_cs,_rs,_rst;	
	void		writecommand(uint8_t c);
	void		writedata(uint8_t d);
	void		writedata16(uint16_t d);
 private:
	void 		colorSpace(uint8_t cspace);
	void 		setAddr(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
	uint8_t		sleep;
	void 		chipInit();
	bool 		boundaryCheck(int16_t x,int16_t y);
	void 		homeAddress();
	uint8_t		_initError;
};
#endif