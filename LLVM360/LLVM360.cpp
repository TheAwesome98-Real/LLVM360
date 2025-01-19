#include <iostream>
#include <fstream> 
#include <stdio.h>
#include "Utils.h"
#include "XexLoader.h"




int main()
{
	// Parse the XEX file. and use as file path a xex file in a folder relative to the exetubable
	XexImage* xex = new XexImage(L"MinecraftX360.xex");
    xex->LoadXex();

    printf("Hello, World!\n");
}