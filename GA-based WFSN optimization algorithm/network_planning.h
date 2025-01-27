#pragma once

#include "../Interface/Engine.h"
#include "../Interface/Convert.h"
#include "../Interface/OutdoorPlugIn.h"
#include "../Interface/OutdoorPlugInPrePro.h"
#include "../Net/Net.h"
#include "../Interface/Init.h"                                                         
#include "../Interface/Prepro.h"             
#include "../Public/Interface/Clutter.h"
#include "../Public/Interface/Topo.h"

int _STD_CALL CallbackMessage(const char *Text);
int _STD_CALL CallbackProgress(int value, const char* text);
int _STD_CALL CallbackError(const char *Message, int Mode);

// Helper functions
void write_ascii(WinProp_Result Resultmatrix, char* Filename);
void AntennaPropertiesSet(WinProp_Antenna *Antenna,	double CoordinateX, double CoordinateY,	double Height,double Frequency, char *Name);
void CarrierPropertiesSet(WinProp_Antenna *Antenna,	WinProp_Carrier *Carrier);
void WavePropagation(int NumberAntennas,WinProp_Antenna *Antenna,WinProp_Result  *Result,int NrHeights,	double *Heights, char* database);
