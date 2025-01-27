#include <stdio.h>
#include <string>
#include <iostream>

#include "network_planning.h"

#ifndef API_DATA_FOLDER
#define API_DATA_FOLDER "../../api/winprop/data/"
#endif // !API_DATA_FOLDER
#define SERVICES_LTE 13
#define MAX_TRX 3

int main(int argc, char** argv)
{
	int                 Count, ProjectHandle, IntValue;
	int                 CurrentCarrier, CurrentService, Error, NrCarriers, NrServices;
	double              FrequencyUL, FrequencyDL, CarrierSeparation, ParaValue;
	char                ServiceName[100];

	/* ---------------------------- network planning parameters for LTE ----------------------------------- */
	char* TransmissionMode_Name[SERVICES_LTE] = { "QPSK - R=1_8", "QPSK - R=1_5", "QPSK - R=1_4", "QPSK - R=1_3", "QPSK - R=1_2", "QPSK - R=2_3", "QPSK - R=4_5", "16 QAM - R=1_2", "16 QAM - R=2_3", "16 QAM - R=4_5", "64 QAM - R=2_3", "64 QAM - R=3_4", "64 QAM - R=4_5" };
	double              TransmissionMode_Bitrate[SERVICES_LTE] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	double              TransmissionMode_SNIR[SERVICES_LTE] = { -5.4, -3.1, -2.2, -0.4, 1.6, 3.5, 5.6, 7.0, 10.7, 11.8, 14.3, 16.1, 17.2 };
	int                 TransmissionMode_Coderate_K[SERVICES_LTE] = { 1, 1, 1, 1, 1, 2, 4, 1, 2, 4, 2, 3, 4 };
	int                 TransmissionMode_Coderate_N[SERVICES_LTE] = { 8, 5, 4, 3, 2, 3, 5, 2, 3, 5, 3, 4, 5 };
	int                 TransmissionMode_MCS[SERVICES_LTE] = { 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 6, 6, 6 };
	double              TransmissionMode_Backoff[SERVICES_LTE] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0 };

	double              AntennaX[MAX_TRX] = { 22.163518, 7.137364, 7.856258 };
	double              AntennaY[MAX_TRX] = { 14.322031, 8.010193, 3.750114 };
	int                 AntennaCarrier[MAX_TRX] = { 1, 2, 2 };
	int                 AntennaSignalMIMO[MAX_TRX] = { -1, 1, 1 };
	int                 AntennaStreamMIMO[MAX_TRX] = { -1, 1, 2 };
	char* AntennaName[MAX_TRX] = { "Site 1", "Site 2", "Site 3" };
	int                 MapIndex[MAX_TRX];

	WinProp_Result* MaxThroughput = NULL, * MaxPower = NULL, * MaxSNIR = NULL, PropResults[MAX_TRX];
	WinProp_Callback    Callback;
	WinProp_Antenna     Antenna[MAX_TRX];
	WinProp_Carrier		Carrier;

	int                 x, y;
	double              RealX, RealY, Value;
	int                 Signal_for_MIMO = -1, Stream_for_MIMO = -1;
	char                ProjectName[200];
	double              Frequency = 1800.0;
	int                 NrPredictionHeights = 1;
	double              PredictionHeightsMulti[2] = { 1, 2 };

	Error = 0;
	ProjectHandle = 0;
	NrPredictionHeights = sizeof(PredictionHeightsMulti) / sizeof(PredictionHeightsMulti[0]);

	/* ------------------- Create new network project: LTE air interface   ---------------------------- */
	if (Error == 0)
	{
		Error = WinProp_Net_Project_Open(&ProjectHandle, NET_AIRINTERFACE_LTE_GENERIC, NULL);
	}
	// TDD mode
	if (Error == 0)
	{
		IntValue = 1;
		Error = WinProp_Net_Project_Para_Set(ProjectHandle, NET_PARA_DUPLEX_SUBMODE, NULL, &IntValue, NULL);
	}

	/* Set name of project (optional). */
	if (Error == 0) {
		sprintf(ProjectName, "%s", "LTE Project");
		Error = WinProp_Net_Project_Para_Set(ProjectHandle, NET_PARA_PROJECTNAME, NULL, NULL, ProjectName);
	}

	/* Retreive carrier separation (must be 20 MHz). */
	if (Error == 0)
	{
		Error = WinProp_Net_Project_Para_Get(ProjectHandle, NET_PARA_CARRIER_SEPARATION, &CarrierSeparation, NULL, NULL);
	}

	/* Add some carriers. */
	if (Error == 0)
	{
		CarrierSeparation *= 0.000001;
		for (CurrentCarrier = 0; CurrentCarrier < 3; CurrentCarrier++)
		{
			FrequencyUL = 1930.0 + (CurrentCarrier * CarrierSeparation);
			FrequencyDL = 2120.0 + (CurrentCarrier * CarrierSeparation);
			Error = WinProp_Net_Carrier_Add(ProjectHandle, CurrentCarrier + 1);
			Error = WinProp_Net_Carrier_Para_Set(ProjectHandle, CurrentCarrier + 1, NET_PARA_CARRIER_FREQ_DL, &FrequencyUL, NULL, NULL);
			Error = WinProp_Net_Carrier_Para_Set(ProjectHandle, CurrentCarrier + 1, NET_PARA_CARRIER_FREQ_UL, &FrequencyDL, NULL, NULL);
		}
	}

	/* Check if number of carriers is correct. */
	if (Error == 0)
	{
		Error = WinProp_Net_Project_Para_Get(ProjectHandle, NET_PARA_CARRIERS, NULL, &NrCarriers, NULL);
		if (Error == 0)
		{
			if (NrCarriers != 3)
				Error = 1;
		}
	}

	/* Add now 13 transmission modes for LTE. */
	if (Error == 0)
	{
		for (CurrentService = 0; CurrentService < SERVICES_LTE; CurrentService++)
		{
			/* Add new service. */
			sprintf(ServiceName, "%s", TransmissionMode_Name[CurrentService]);
			Error = WinProp_Net_TransmissionMode_Add(ProjectHandle, ServiceName, CurrentService);

			/* Set position/priority. */
			IntValue = SERVICES_LTE - CurrentService;
			Error = WinProp_Net_TransmissionMode_Para_Set(ProjectHandle, CurrentService, NET_PARA_TRANS_MODE_POSITION, NULL, &IntValue, NULL);

			/* Set bitrate. */
			ParaValue = TransmissionMode_Bitrate[CurrentService];
			Error = WinProp_Net_TransmissionMode_Para_Set(ProjectHandle, CurrentService, NET_PARA_TRANS_MODE_BITRATE_DL, &ParaValue, NULL, NULL);
			Error = WinProp_Net_TransmissionMode_Para_Set(ProjectHandle, CurrentService, NET_PARA_TRANS_MODE_BITRATE_UL, &ParaValue, NULL, NULL);

			/* Set code rate. */
			IntValue = TransmissionMode_Coderate_K[CurrentService];
			Error = WinProp_Net_TransmissionMode_Para_Set(ProjectHandle, CurrentService, NET_PARA_TRANS_MODE_CODERATE_K_DL, NULL, &IntValue, NULL);
			Error = WinProp_Net_TransmissionMode_Para_Set(ProjectHandle, CurrentService, NET_PARA_TRANS_MODE_CODERATE_K_UL, NULL, &IntValue, NULL);
			IntValue = TransmissionMode_Coderate_N[CurrentService];
			Error = WinProp_Net_TransmissionMode_Para_Set(ProjectHandle, CurrentService, NET_PARA_TRANS_MODE_CODERATE_N_DL, NULL, &IntValue, NULL);
			Error = WinProp_Net_TransmissionMode_Para_Set(ProjectHandle, CurrentService, NET_PARA_TRANS_MODE_CODERATE_N_UL, NULL, &IntValue, NULL);

			/* Set number of resource blocks. */
			IntValue = 1;
			Error = WinProp_Net_TransmissionMode_Para_Set(ProjectHandle, CurrentService, NET_PARA_TRANS_MODE_RESOURCE_BLOCKS_DL, NULL, &IntValue, NULL);
			Error = WinProp_Net_TransmissionMode_Para_Set(ProjectHandle, CurrentService, NET_PARA_TRANS_MODE_RESOURCE_BLOCKS_UL, NULL, &IntValue, NULL);

			/* Set overhead ratio. */
			ParaValue = 11.1;
			Error = WinProp_Net_TransmissionMode_Para_Set(ProjectHandle, CurrentService, NET_PARA_TRANS_MODE_OVERHEAD_RATIO_UL, &ParaValue, NULL, NULL);
			Error = WinProp_Net_TransmissionMode_Para_Set(ProjectHandle, CurrentService, NET_PARA_TRANS_MODE_OVERHEAD_RATIO_DL, &ParaValue, NULL, NULL);

			/* Set required SNIR. */
			ParaValue = TransmissionMode_SNIR[CurrentService];
			Error = WinProp_Net_TransmissionMode_Para_Set(ProjectHandle, CurrentService, NET_PARA_TRANS_MODE_SNIR_DL, &ParaValue, NULL, NULL);
			Error = WinProp_Net_TransmissionMode_Para_Set(ProjectHandle, CurrentService, NET_PARA_TRANS_MODE_SNIR_UL, &ParaValue, NULL, NULL);

			/* Set modulation scheme. */
			IntValue = TransmissionMode_MCS[CurrentService];
			Error = WinProp_Net_TransmissionMode_Para_Set(ProjectHandle, CurrentService, NET_PARA_TRANS_MODE_MODULATION_UL, NULL, &IntValue, NULL);
			Error = WinProp_Net_TransmissionMode_Para_Set(ProjectHandle, CurrentService, NET_PARA_TRANS_MODE_MODULATION_DL, NULL, &IntValue, NULL);

			/* Set power backoff. */
			ParaValue = TransmissionMode_Backoff[CurrentService];
			Error = WinProp_Net_TransmissionMode_Para_Set(ProjectHandle, CurrentService, NET_PARA_TRANS_MODE_POWER_BACKOFF_UL, &ParaValue, NULL, NULL);
			Error = WinProp_Net_TransmissionMode_Para_Set(ProjectHandle, CurrentService, NET_PARA_TRANS_MODE_POWER_BACKOFF_DL, &ParaValue, NULL, NULL);

			/* Get bitrate in kBit/s for current transmission mode. */
			/* Bitrate is automatically computed based on parameters previously set and general air interface parameters. */
			Error = WinProp_Net_TransmissionMode_Para_Get(ProjectHandle, CurrentService, NET_PARA_TRANS_MODE_BITRATE_DL, &ParaValue, NULL, NULL);
			TransmissionMode_Bitrate[CurrentService] = ParaValue;
		}
	}

	/* Check if number of services is correct. */
	if (Error == 0)
	{
		Error = WinProp_Net_Project_Para_Get(ProjectHandle, NET_PARA_SERVICES, NULL, &NrServices, NULL);
		if (Error == 0)
		{
			if (NrServices != SERVICES_LTE)
				Error = 1;
		}
	}

	/* Set minimum required SNIR. */
	if (Error == 0)
	{
		ParaValue = -5.4;
		Error = WinProp_Net_Project_Para_Set(ProjectHandle, NET_PARA_CELL_ASSIGNMENT_MIN_REQ_SNIR, &ParaValue, NULL, NULL);
	}

	/* Disable min power criterion. */
	if (Error == 0)
	{
		IntValue = 0;
		Error = WinProp_Net_Project_Para_Set(ProjectHandle, NET_PARA_CELL_ASSIGNMENT_MIN_REQ_POWER_USED, NULL, &IntValue, NULL);
	}

	/* Set cell assignment mode. */
	if (Error == 0)
	{
		IntValue = 0;
		Error = WinProp_Net_Project_Para_Set(ProjectHandle, NET_PARA_CELL_ASSIGNMENT_MODE, NULL, &IntValue, NULL);
	}

	/* Set cell assignment signals. */
	if (Error == 0)
	{
		IntValue = 0;
		Error = WinProp_Net_Project_Para_Set(ProjectHandle, NET_PARA_CELL_ASSIGNMENT_SIGNALS, NULL, &IntValue, NULL);
	}

	/* Set resolution for result matrix. */
	if (Error == 0)
	{
		ParaValue = 1.0;
		Error = WinProp_Net_Project_Para_Set(ProjectHandle, NET_PARA_RESOLUTION, &ParaValue, NULL, NULL);
	}

	/* Set size of area: Automatic mode. */
	if (Error == 0)
	{
		IntValue = 0;
		Error = WinProp_Net_Project_Para_Set(ProjectHandle, NET_PARA_AREA_MODE, NULL, &IntValue, NULL);
	}

	/* Set paths for additional output in WinProp file format. */
	if (Error == 0)
	{
		Error = WinProp_Net_Project_Para_Set(ProjectHandle, NET_PARA_OUTPUT_WINPROP, NULL, NULL, API_DATA_FOLDER "output");
	}
	/* Compute wave propagation for three transmitters                          */
	/* Init. antennas and results */
	for (Count = 0; Count < MAX_TRX; Count++)
	{
		/* Init of antenna */
		WinProp_Structure_Init_Antenna(&Antenna[Count]);
		/* Determine frequency for antenna. */
		Frequency = 2120.0 + ((AntennaCarrier[Count] - 1) * CarrierSeparation);

		/* Set properties now. */
		AntennaPropertiesSet(&Antenna[Count], AntennaX[Count], AntennaY[Count], 1.5, Frequency, AntennaName[Count]);

		// Init of carrier settings 
		WinProp_Structure_Init_Carrier(&Carrier);

		// set carrier properties 
		Carrier.CarrierID = AntennaCarrier[Count];
		Carrier.SystemID = Signal_for_MIMO;
		Carrier.MimoID = Stream_for_MIMO;
		CarrierPropertiesSet(&Antenna[Count], &Carrier);

		/* Init of result */
		WinProp_Structure_Init_Result(&PropResults[Count]);
	}
	/* Compute wave propagation for all antennas. */
	if (Error == 0)
	{
		char* database = API_DATA_FOLDER "../data/indoor/PDZ1.idb";
		WavePropagation(MAX_TRX, Antenna, PropResults, NrPredictionHeights, PredictionHeightsMulti, database);
	}

	/* Now do network planning */
	/* Add all propagation maps which have been computed before. */
	if (Error == 0)
	{
		for (Count = 0; Count < MAX_TRX; Count++)
		{
			if (Error == 0)
			{
				Error = WinProp_Net_PropagationMap_Add(ProjectHandle, &MapIndex[Count], &Antenna[Count], &PropResults[Count]);
			}
		}
	}

	if (Error == 0)
	{
		char HeightString[500];
		sprintf(HeightString, "%s", "");

		/* Generate string with height values, e.g. a string like "1.5 2.5 3.5". */
		for (int Height = 0; Height < NrPredictionHeights; Height++)
		{
			/* Add current height to string. */
			sprintf(HeightString, "%s%.2f ", HeightString, PredictionHeightsMulti[Height]);
		}

		/* Send heights to WinProp API. */
		Error = WinProp_Net_Project_Para_Set(ProjectHandle, NET_PARA_HEIGHT_MULTIPLE, NULL, NULL, HeightString);

		/* Start computation. */
		if (Error == 0)
		{
			Callback.Percentage = CallbackProgress;
			Callback.Message = CallbackMessage;
			Callback.Error = CallbackError;
			Error = WinProp_Net_Project_Compute(ProjectHandle, &Callback);
		}
	}

	// -----------------------------------------------------------------------------
	// Retrieve results
	// -----------------------------------------------------------------------------
	/* As an example: retrieve max. throughput (kbps) per pixel. */
	if (Error == 0)
	{
		Error = WinProp_Net_NetworkMap_Get(ProjectHandle, -1, NET_RESULT_MAX_THROUGHPUT, &MaxThroughput);
	}

	/* Write max. throughput result to ASCII file. */
	if (Error == 0)
	{
		char NameForOutput[200];
		sprintf(NameForOutput, API_DATA_FOLDER "output/%s Max Throughput.txt", ProjectName);
		FILE* Outfile = fopen(NameForOutput, "w");
		if (Outfile)
		{
			for (x = 0; x < MaxThroughput->Columns; x++)
			{
				RealX = MaxThroughput->LowerLeftX + ((double)x + 0.5) * MaxThroughput->Resolution;
				for (y = 0; y < MaxThroughput->Lines; y++)
				{
					RealY = MaxThroughput->LowerLeftY + ((double)y + 0.5) * MaxThroughput->Resolution;
					Value = MaxThroughput->Matrix[0][x][y];
					fprintf(Outfile, "%.2f\t%.2f\t%.2f\n", RealX, RealY, Value);
				}
			}

			fclose(Outfile);
		}
	}

	/* As another example: retrieve SNIR (dB) per pixel. */
	//if (Error == 0)
	//{
	//	Error = WinProp_Net_NetworkMap_Get(ProjectHandle, -1, NET_RESULT_SNIR, &MaxSNIR);
	//}

	/* Write max. throughput result to ASCII file. */
	//if (Error == 0)
	//{
	//	char NameForOutput[200];
	//	sprintf(NameForOutput, API_DATA_FOLDER "output/%s Max SNIR.txt", ProjectName);
	//	FILE *Outfile = fopen(NameForOutput, "w");
	//	if (Outfile)
	//	{
	//		for (x = 0; x<MaxSNIR->Columns; x++)
	//		{
	//			RealX = MaxSNIR->LowerLeftX + ((double)x + 0.5) * MaxSNIR->Resolution;
	//			for (y = 0; y<MaxSNIR->Lines; y++)
	//			{
	//				RealY = MaxSNIR->LowerLeftY + ((double)y + 0.5) * MaxSNIR->Resolution;
	//				Value = MaxSNIR->Matrix[0][x][y];
	//				fprintf(Outfile, "%.2f\t%.2f\t%.2f\n", RealX, RealY, Value);
	//			}
	//		}

	//		fclose(Outfile);
	//	}
	//}

	// --------------------------------------------------------------------------
	// Free Memory
	// --------------------------------------------------------------------------
	/* Free propagation results. */
	//for (Count = 0; Count<MAX_TRX; Count++)
	//	WinProp_FreeResult(&PropResults[Count]);
	/* Close network project. */
	//Error = WinProp_Net_Project_Close(ProjectHandle);

	return 0;
}

int _STD_CALL CallbackMessage(const char* Text)
{
	if (Text == nullptr)
		return 0;

	std::cout << "\n" << Text;

	return(0);
}

int _STD_CALL CallbackError(const char* Text, int Error)
{
	if (Text == nullptr)
		return 0;

	std::cout << "\n";

#ifdef __LINUX
	std::cout << "\033[31m" << "Error (" << Error << "): "; // highlight error in red color
#else
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
	std::cout << "Error (" << Error << "): ";
#endif // __LINUX
	std::cout << Text;

#ifdef __LINUX
	std::cout << "\033[0m"; // highlight error in red color
#else
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
#endif // __LINUX

	return 0;
}

int _STD_CALL CallbackProgress(int value, const char* text)
{
	char Line[200];

	sprintf(Line, "\n%d%% %s", value, text);
	std::cout << Line;

	return(0);
}

// Helper functions
void write_ascii(WinProp_Result Resultmatrix, char* Filename) {
	FILE* OutputFile = fopen(Filename, "w");
	if (OutputFile)
	{
		/* Loop through WinPropall pixels. */
		for (int x = 0; x < Resultmatrix.Columns; x++)
		{
			for (int y = 0; y < Resultmatrix.Lines; y++)
			{
				/* Compute real coordinates. */
				double Coordinate_X = Resultmatrix.LowerLeftX + ((double)x + 0.5) * Resultmatrix.Resolution;
				double Coordinate_Y = Resultmatrix.LowerLeftY + ((double)y + 0.5) * Resultmatrix.Resolution;

				/* Check if pixel was computed or not */
				if (Resultmatrix.Matrix[0][x][y] > -1000)
					fprintf(OutputFile, "%.2f\t%.2f\t%.2f\n", Coordinate_X, Coordinate_Y, Resultmatrix.Matrix[0][x][y]);
			}
		}

		/* Close file. */
		fclose(OutputFile);
	}
	else
		printf("\nCould not open the File: %s for writing.\n", Filename);
}

void AntennaPropertiesSet(WinProp_Antenna* Antenna, double CoordinateX, double CoordinateY, double Height, double Frequency, char* Name) {
	Antenna->Longitude_X = CoordinateX;
	Antenna->Latitude_Y = CoordinateY;
	Antenna->Height = Height;
	Antenna->Power = 20.0;
	Antenna->PowerMode = 0;
	Antenna->Frequency = Frequency;
	Antenna->Name = Name;
	Antenna->Model = WINPROP_MODEL_COST231;
}

void CarrierPropertiesSet(WinProp_Antenna* Antenna, WinProp_Carrier* Carrier) {
	Antenna->Carriers.CarrierID = Carrier->CarrierID;
	Antenna->Carriers.SystemID = Carrier->SystemID;
	Antenna->Carriers.MimoID = Carrier->MimoID;
}

// Wave propagation
void WavePropagation(int NumberAntennas, WinProp_Antenna* Antenna, WinProp_Result* Result, int NrHeights, double* Heights, char* database)
{
	int                     PredictionHandle;
	WinProp_Area            Area;
	WinProp_Scenario        Scenario;
	WinProp_Callback        Callback;
	WinProp_Result* DummyResult;
	int                     Error, Count;
	double                  Height = 1.5;
	int                     BitmapMode = 0;
	WinProp_Legend          Legend;
	// init
	Error = 0;
	PredictionHandle = 0;
	WinProp_Structure_Init_Area(&Area);
	WinProp_Structure_Init_Scenario(&Scenario);
	WinProp_Structure_Init_Legend(&Legend);
	// Open scenario
	Scenario.Scenario = WINPROP_SCENARIO_INDOOR;
	Scenario.VectorDatabase = database;
	Callback.Percentage = CallbackProgress;
	Callback.Message = CallbackMessage;
	Callback.Error = CallbackError;

	/* Open new project. */
	if (Error == 0)
	{
		Error = WinProp_Open(&PredictionHandle, &Scenario, &Callback);
	}

	/* Define area */
	if (Error == 0)
	{
		/* Defintion of area. */
		Area.NrHeights = NrHeights;
		Area.Heights = (double*)malloc(sizeof(double) * NrHeights);
		for (int C = 0; C < NrHeights; C++)
			Area.Heights[C] = Heights[C];
		Area.LowerLeftX = -5;
		Area.LowerLeftY = -5;
		Area.UpperRightX = 30;
		Area.UpperRightY = 20;
		Area.Resolution = 1.0;
	}
	// ------------------------------------------------------------------------
	// Compute wave propagation
	// ------------------------------------------------------------------------
	for (Count = 0; Count < NumberAntennas; Count++)
	{
		if (Error == 0)
		{
			DummyResult = NULL;
			Error = WinProp_Predict(PredictionHandle, &Antenna[Count], &Area, NULL, NULL, &DummyResult, NULL, NULL, NULL, NULL);
			if (Error == 0) {
				/* Copy result. */
				WinProp_CopyResult(&Result[Count], DummyResult);
			}
			else {
				/* Error during prediction. Print error message. */
				printf("\n\nSimulation returned with Error %d\n\n", Error);
			}
		}
	}
	// Free memory
	if (Area.Heights)
		free(Area.Heights);
	// Close project
	WinProp_Close(PredictionHandle);
}