/*
=================================================================================
 Name        : pcd8544_rpi.c
 Version     : 0.2

 Copyright (C) 2012 by Andre Wussow, 2012, desk@binerry.de

 UPDATE 0.2  Add display for  ip and cpu temp 
	Copyright (C) 2014 by Cao Cheng, 2014, caoc@live.com


 Description :
     A simple PCD8544 LCD (Nokia3310/5110) for Raspberry Pi for displaying some system informations.
   Makes use of WiringPI-library of Gordon Henderson (https://projects.drogon.net/raspberry-pi/wiringpi/)

	 Recommended connection (http://www.raspberrypi.org/archives/384):
	 LCD pins      Raspberry Pi
	 LCD1 - GND    P06  - GND
	 LCD2 - VCC    P01 - 3.3V
	 LCD3 - CLK    P11 - GPIO0
	 LCD4 - Din    P12 - GPIO1
	 LCD5 - D/C    P13 - GPIO2
	 LCD6 - CS     P15 - GPIO3
	 LCD7 - RST    P16 - GPIO4
	 LCD8 - LED    P01 - 3.3V 

================================================================================
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
================================================================================
 */
#include <wiringPi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include "PCD8544.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

#define TEMP_PATH "/sys/class/thermal/thermal_zone0/temp"
#define MAX_SIZE 32

// pin setup
int _din = 1;
int _sclk = 0;
int _dc = 2;
int _rst = 4;
int _cs = 3;


  
// lcd contrast 
int contrast = 65;

//get cpu temp
double get_cputemp(){
int fd;
  double temp = 0;
  char buf[MAX_SIZE];
  
  fd = open(TEMP_PATH, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "failed to open thermal_zone0/temp\n");
    return -1;
  }
  
  if (read(fd, buf, MAX_SIZE) < 0) {
    fprintf(stderr, "failed to read temp\n");
    return -1;
  }
  
//  printf("temp: %.2f\n", temp);
  close(fd);
  temp = atoi(buf) / 1000.0;
  return temp;
  

}

//get wifi or eth ip
char* get_ip(char ifname[10]) {
  int fd;
  struct ifreq ifr;

  fd = socket(AF_INET, SOCK_DGRAM, 0);

  ifr.ifr_addr.sa_family = AF_INET;

  strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);

  ioctl(fd, SIOCGIFADDR, &ifr);

  close(fd);

  //printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
  return  (char *) inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}
  
int main (void)
{



  // print infos
  printf("Raspberry Pi PCD8544 sysinfo display\n");
  printf("========================================\n");
  
  // check wiringPi setup
  if (wiringPiSetup() == -1)
  {
	printf("wiringPi-Error\n");
    exit(1);
  }
  
  // init and clear lcd
  LCDInit(_sclk, _din, _dc, _cs, _rst, contrast);
  LCDclear();


  
  // show logo
  LCDshowLogo();


  
  delay(2000);
  
  for (;;)
  {
	  // clear lcd
	  LCDclear();
	  
	  // get system usage / info
	  struct sysinfo sys_info;
	  if(sysinfo(&sys_info) != 0)
	  {
		printf("sysinfo-Error\n");
	  }
	  
	  // uptime
	  char uptimeInfo[15];
	  unsigned long uptime = sys_info.uptime / 60;
	  sprintf(uptimeInfo, "Uptime %ld min.", uptime);
	  
	  // cpu info
	  char cpuInfo[20]; 
	  unsigned long avgCpuLoad = sys_info.loads[0] / 1000;
	  sprintf(cpuInfo, "CPU %ld%% %.2f", avgCpuLoad,get_cputemp());
	  
	  // ram info
	  char ramInfo[10]; 
	  unsigned long totalRam = sys_info.freeram / 1024 / 1024;
	  sprintf(ramInfo, "RAM %ld MB", totalRam);
	 //printf("RAM %ld MB\n", totalRam);
	  
          char ethipInfo[24];
          char wifiipInfo[24];
	  sprintf(ethipInfo, "E%s", get_ip("eth0"));
	  sprintf(wifiipInfo, "W%s", get_ip("wlan0"));
	  
	  // build screen
	  LCDdrawstring(0, 0, "Raspberry Pi:");
	  LCDdrawline(0, 10, 83, 10, BLACK);
	//  LCDdrawstring(0, 12, uptimeInfo);
          LCDdrawstring(0, 12, ethipInfo);
          LCDdrawstring(0, 20, wifiipInfo);
	  LCDdrawstring(0, 28, cpuInfo);
	  LCDdrawstring(0, 36, ramInfo);
	  
	  LCDdisplay();
	  
	  delay(2000);
  }
  
    //for (;;){
  //  printf("LED On\n");
  //  digitalWrite(pin, 1);
  //  delay(250);
  //  printf("LED Off\n");
  //  digitalWrite(pin, 0);
  //  delay(250);
  //}

  return 0;
}
