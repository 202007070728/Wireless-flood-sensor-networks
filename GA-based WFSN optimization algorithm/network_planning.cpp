#include <stdio.h>
#include <string>
#include <iostream>

#include "network_planning.h"

#ifndef API_DATA_FOLDER
#define API_DATA_FOLDER "../../api/winprop/data/"
#endif // !API_DATA_FOLDER
#define SERVICES_LTE 13
#define MAX_TRX 3

typedef struct {
	int minX, minY, maxX, maxY;
} RectRegion;
const int chromosome_size = 100;//种群大小
const int chromosome_length = 6;//染色体长度，主要是3个传感器的xy坐标
const int epoch_max = 10;    // 最大迭代次数a
const double cross_probability = 0.1;   // 交叉概率
const double mutation_probability = 0.1;   // 变异概率
void initializeChromosomes(double chromosomes[chromosome_size][chromosome_length]);

double generateRandomCoordinate(double min, double max);
// 染色题本身 二维就是这个解的x，y存储 
double chromosome[chromosome_size][chromosome_length];
// 染色体适应度函数 二维就是这个解的x，y对应的函数的值的存储 
double chromosome_fitness[chromosome_size];
// 个体被选中的概率，这个是遗传算法的特色
double chromosome_chosen[chromosome_size] = { 0 };
// 每次迭代的最优解
double epoch_best_fitness[epoch_max];
// 每次迭代的最优解的位置，在二维里就是对应的x, y 
double epoch_best[epoch_max];
// 之前所有迭代的最优解的平均值
double average_epoch_best_fitness[epoch_max];
// 存储目前所有迭代中的最优解
double all_best_fitness;
// 存储目前所有迭代中的最优解出现的代数
int all_best_epoch;

double fitting(WinProp_Result Resultmatrix1, WinProp_Result Resultmatrix2, WinProp_Result Resultmatrix3, double fitness);
double* max_in_community(double fit[], int size);
void cross(double chromosome[chromosome_size][chromosome_length]);
void mutation(double chromosomes[chromosome_size][chromosome_length]);

int main(int argc, char** argv)
{
	int                 Count, ProjectHandle, IntValue;
	int                 CurrentCarrier, CurrentService, Error, NrCarriers, NrServices;
	double              FrequencyUL, FrequencyDL, CarrierSeparation = 0, ParaValue;
	char                ServiceName[100];

	/* ---------------------------- network planning parameters for LTE ----------------------------------- */
	char* TransmissionMode_Name[SERVICES_LTE] = { "QPSK - R=1_8", "QPSK - R=1_5", "QPSK - R=1_4", "QPSK - R=1_3", "QPSK - R=1_2", "QPSK - R=2_3", "QPSK - R=4_5", "16 QAM - R=1_2", "16 QAM - R=2_3", "16 QAM - R=4_5", "64 QAM - R=2_3", "64 QAM - R=3_4", "64 QAM - R=4_5" };
	double              TransmissionMode_Bitrate[SERVICES_LTE] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	double              TransmissionMode_SNIR[SERVICES_LTE] = { -5.4, -3.1, -2.2, -0.4, 1.6, 3.5, 5.6, 7.0, 10.7, 11.8, 14.3, 16.1, 17.2 };
	int                 TransmissionMode_Coderate_K[SERVICES_LTE] = { 1, 1, 1, 1, 1, 2, 4, 1, 2, 4, 2, 3, 4 };
	int                 TransmissionMode_Coderate_N[SERVICES_LTE] = { 8, 5, 4, 3, 2, 3, 5, 2, 3, 5, 3, 4, 5 };
	int                 TransmissionMode_MCS[SERVICES_LTE] = { 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 6, 6, 6 };
	double              TransmissionMode_Backoff[SERVICES_LTE] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0 };

	double              AntennaX[MAX_TRX];
	double              AntennaY[MAX_TRX];
	int                 AntennaCarrier[MAX_TRX] = { 1, 2, 2 };
	int                 AntennaSignalMIMO[MAX_TRX] = { -1, 1, 1 };
	int                 AntennaStreamMIMO[MAX_TRX] = { -1, 1, 2 };
	char* AntennaName[MAX_TRX] = { "Site 1", "Site 2", "Site 3" };
	int                 MapIndex[MAX_TRX];

	WinProp_Result* RSRP = NULL, * SNIR = NULL, * RSRQ = NULL, PropResults[MAX_TRX], * DummyResult;
	WinProp_Callback    Callback;
	WinProp_Antenna     Antenna[MAX_TRX];
	WinProp_Carrier		Carrier;
	WinProp_Area        Area;
	WinProp_Scenario    Scenario;
	WinProp_Legend      Legend;

	int                 x, y;
	double              RealX, RealY, Value;
	int                 Signal_for_MIMO = -1, Stream_for_MIMO = -1;
	char                ProjectName[200];
	double              Frequency = 1800.0, fitness = 0;;
	int                 NrPredictionHeights = 1;
	double              PredictionHeightsMulti[3] = { 0.3, 1.5,2.7 };
	double              Height = 2.8;
	int                 BitmapMode = 0;

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
	//if (Error == 0) {
		//sprintf(ProjectName, "%s", "LTE Project");
		//Error = WinProp_Net_Project_Para_Set(ProjectHandle, NET_PARA_PROJECTNAME, NULL, NULL, ProjectName);
	//}

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
		AntennaPropertiesSet(&Antenna[Count], AntennaX[Count], AntennaY[Count], 2.8, Frequency, AntennaName[Count]);

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

	initializeChromosomes(chromosome);

	int                     PredictionHandle;
	Error = 0;
	PredictionHandle = 0;
	WinProp_Structure_Init_Area(&Area);
	WinProp_Structure_Init_Scenario(&Scenario);
	WinProp_Structure_Init_Legend(&Legend);
	// Open scenario
	Scenario.Scenario = WINPROP_SCENARIO_INDOOR;
	char* database = API_DATA_FOLDER "../data/indoor/PDZ1.idb";
	Scenario.VectorDatabase = API_DATA_FOLDER "../data/indoor/PDZ1.idb";
	Callback.Percentage = CallbackProgress;
	Callback.Message = CallbackMessage;
	Callback.Error = CallbackError;

	/* Open new project. */
	for (int i = 0; i < chromosome_size; i++)
	{
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
		if (Error == 0)
		{
			Error = WinProp_Open(&PredictionHandle, &Scenario, &Callback);
		}

		/* Define area */
		if (Error == 0)
		{
			/* Defintion of area. */
			Area.NrHeights = NrPredictionHeights;
			Area.Heights = (double*)malloc(sizeof(double) * NrPredictionHeights);
			for (int C = 0; C < NrPredictionHeights; C++)
				Area.Heights[C] = PredictionHeightsMulti[C];
			Area.LowerLeftX = 0;
			Area.LowerLeftY = 0;
			Area.UpperRightX = 26.5;
			Area.UpperRightY = 16.5;
			Area.Resolution = 1.0;
		}
		for (Count = 0; Count < MAX_TRX; Count++)
		{
			if (Count == 0)
			{
				AntennaX[Count] = chromosome[i][0];
				AntennaY[Count] = chromosome[i][1];
			}
			else if (Count == 1)
			{
				AntennaX[Count] = chromosome[i][2];
				AntennaY[Count] = chromosome[i][3];
			}
			else if (Count == 2)
			{
				AntennaX[Count] = chromosome[i][4];
				AntennaY[Count] = chromosome[i][5];
			}
			AntennaPropertiesSet(&Antenna[Count], AntennaX[Count], AntennaY[Count], 2.8, Frequency, AntennaName[Count]);
			DummyResult = NULL;
			char initialFilename[200];
			sprintf(initialFilename, "初始化时的第%d个染色体，%d个传感器分开结果", i, Count);

			Error = WinProp_Predict(PredictionHandle, &Antenna[Count], &Area, NULL, NULL, &DummyResult, NULL, NULL, NULL, NULL);
			//printf("传感器坐标：(%f, %f)\n" ,AntennaX[Count], AntennaY[Count]);没问题
			FILE* Outfile = fopen(initialFilename, "w");
			//if (Outfile)
			//{
			//	for (x = 0; x < DummyResult->Columns; x++)
			//	{
			//		RealX = DummyResult->LowerLeftX + ((double)x + 0.5) * DummyResult->Resolution;
			//		for (y = 0; y < DummyResult->Lines; y++)
			//		{
			//			RealY = DummyResult->LowerLeftY + ((double)y + 0.5) * DummyResult->Resolution;
			//			Value = DummyResult->Matrix[0][x][y];
			//			fprintf(Outfile, "%.2f\t%.2f\t%.2f\n", RealX, RealY, Value);
			//		}
			//	}

			//	fclose(Outfile);
			//}
			write_ascii(*DummyResult, initialFilename);
			if (Error == 0) {
				/* Copy result. */
				WinProp_CopyResult(&PropResults[Count], DummyResult);
			}
			else {
				/* Error during prediction. Print error message. */
				printf("\n\nSimulation returned with Error %d\n\n", Error);
			}

		}
		// Free memory
		if (Area.Heights)
			free(Area.Heights);
		WinProp_Close(PredictionHandle);
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

		// -----------------------------------------------------------------------------
		// Retrieve results
		// -----------------------------------------------------------------------------
		/* As an example: retrieve max. throughput (kbps) per pixel. */
		char Nameput[200];
		sprintf(Nameput, API_DATA_FOLDER "output/初始第%d个种群坐标.txt", i);
		FILE* Outfile = fopen(Nameput, "w");
		if (Outfile)
			fprintf(Outfile, " (% lf, % lf), (% lf, % lf), (% lf, % lf)", chromosome[i][0], chromosome[i][1], chromosome[i][2], chromosome[i][3], chromosome[i][4], chromosome[i][5]);
		fclose(Outfile);
		if (Error == 0)
		{
			Error = WinProp_Net_NetworkMap_Get(ProjectHandle, -1, NET_RESULT_LTE_RSRQ, &RSRQ);
			Error = WinProp_Net_NetworkMap_Get(ProjectHandle, -1, NET_RESULT_LTE_RSRP, &RSRP);
			Error = WinProp_Net_NetworkMap_Get(ProjectHandle, -1, NET_RESULT_SNIR, &SNIR);
		}
		chromosome_fitness[i] = fitting(*RSRQ, *RSRP, *SNIR, fitness);
		/* Write max. throughput result to ASCII file. */
		/*if (Error == 0)
		{
			char NameForOutput[200];
			sprintf(NameForOutput, API_DATA_FOLDER "output/初始时第%d个种群合并结果.txt", i);
			FILE* Outfile = fopen(NameForOutput, "w");
			if (Outfile)
			{
				for (x = 0; x < MaxPower->Columns; x++)
				{
					RealX = MaxPower->LowerLeftX + ((double)x + 0.5) * MaxPower->Resolution;
					for (y = 0; y < MaxPower->Lines; y++)
					{
						RealY = MaxPower->LowerLeftY + ((double)y + 0.5) * MaxPower->Resolution;
						Value = MaxPower->Matrix[0][x][y];
						fprintf(Outfile, "%.2f\t%.2f\t%.2f\n", RealX, RealY, Value);
					}
				}

				fclose(Outfile);
			}
			//write_ascii(*Power, NameForOutput);
		} */
	}
	double* best_fitness_index;
	double fitness_sum = 0;
	best_fitness_index = max_in_community(chromosome_fitness, chromosome_size);
	for (int i = 0; i < chromosome_size; i++)
	{
		fitness_sum += chromosome_fitness[i];
		average_epoch_best_fitness[0] = fitness_sum / chromosome_size;
	}
	all_best_fitness = best_fitness_index[0];
	all_best_epoch = 0;
	for (int j = 0; j < chromosome_length; j++)
		epoch_best[j] = chromosome[(int)best_fitness_index[1]][j];
	char* Filename = API_DATA_FOLDER "output/Results.txt";
	FILE* OutputFile = fopen(Filename, "w");
	//以上均为初始化

	for (int a = 0; a < epoch_max; a++)
	{
		//计算每个染色体的概率(累计概率）
		for (int i = 0; i < chromosome_size; i++)
		{

			if (i == 0)
				chromosome_chosen[i] = chromosome_fitness[i] / fitness_sum;
			else
				chromosome_chosen[i] = chromosome_fitness[i] / fitness_sum + chromosome_chosen[i - 1];
		}
		// 选择的主体 => 随机数筛选，染色体的概率越大，越容易被select到 
		int index[chromosome_size];
		for (int i = 0; i < chromosome_size; i++)
		{

			double rand0 = ((double)rand()) / RAND_MAX;
			while (rand0 < 0.001)
				rand0 = ((double)rand()) / RAND_MAX;
			for (int b = 0; b < chromosome_size; b++)
			{
				if (rand0 <= chromosome_chosen[b])
				{
					index[i] = b;
					printf("index[%d] = %d\n", i, b);
					break;
				}
			}
		}
		char middle[200];
		sprintf(middle, API_DATA_FOLDER "output/111第%d代坐标.txt", a);
		FILE* Outfile = fopen(middle, "w");
		fprintf(Outfile, "fitness_sum=%lf\n", fitness_sum);
		for (int i = 0; i < chromosome_size; i++)
		{
			fprintf(Outfile, "第%d条染色体的适应度为%lf,第%d条染色体被选择概率为%lf\t", i, chromosome_fitness[i], i, chromosome_chosen[i]);
			for (int j = 0; j < chromosome_length; j++)
				fprintf(Outfile, "chromosome[%d][%d]=%lf\n", i, j, chromosome[i][j]);
		}
		fclose(Outfile);
		// 选择完了，在这里记录
		for (int i = 0; i < chromosome_size; i++)
		{
			for (int j = 0; j < chromosome_length; j++)
				chromosome[i][j] = chromosome[index[i]][j];
			chromosome_fitness[i] = chromosome_fitness[index[i]];
		}
		char middle1[200];
		sprintf(middle1, API_DATA_FOLDER "output/222第%d代坐标.txt", a);
		Outfile = fopen(middle1, "w");
		for (int i = 0; i < chromosome_size; i++)
		{
			fprintf(Outfile, "\t");
			for (int j = 0; j < chromosome_length; j++)
				fprintf(Outfile, "chromosome[%d][%d]=%lf\n", i, j, chromosome[i][j]);
		}
		fclose(Outfile);
		//交叉
		cross(chromosome);
		//变异

		mutation(chromosome);
		//更新一系列数据 
		// 计算操作后染色体对应的适应度

		fitness_sum = 0;
		for (int i = 0; i < chromosome_size; i++)
		{
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
			fitness = 0;
			Error = WinProp_Open(&PredictionHandle, &Scenario, &Callback);
			/* Define area */
			/* Defintion of area. */
			Area.NrHeights = NrPredictionHeights;
			Area.Heights = (double*)malloc(sizeof(double) * NrPredictionHeights);
			for (int C = 0; C < NrPredictionHeights; C++)
				Area.Heights[C] = PredictionHeightsMulti[C];
			Area.LowerLeftX = -1;
			Area.LowerLeftY = -1;
			Area.UpperRightX = 26.5;
			Area.UpperRightY = 16.5;
			Area.Resolution = 1.0;
			//fitness = 0;
			for (Count = 0; Count < MAX_TRX; Count++)
			{
				if (Count == 0)
				{
					AntennaX[Count] = chromosome[i][0];
					AntennaY[Count] = chromosome[i][1];
				}
				else if (Count == 1)
				{
					AntennaX[Count] = chromosome[i][2];
					AntennaY[Count] = chromosome[i][3];
				}
				else if (Count == 2)
				{
					AntennaX[Count] = chromosome[i][4];
					AntennaY[Count] = chromosome[i][5];
				}
				AntennaPropertiesSet(&Antenna[Count], AntennaX[Count], AntennaY[Count], 2.8, Frequency, AntennaName[Count]);
				DummyResult = NULL;
				char overFilename[200];
				sprintf(overFilename, API_DATA_FOLDER "output/第%d个染色体，%d个传感器分开结果middle.txt", i, Count);

				Error = WinProp_Predict(PredictionHandle, &Antenna[Count], &Area, NULL, NULL, &DummyResult, NULL, NULL, NULL, NULL);
				//printf("传感器坐标：(%f, %f)\n" ,AntennaX[Count], AntennaY[Count]);没问题
				FILE* Outfile = fopen(overFilename, "w");
				if (Outfile)
				{
					for (x = 0; x < DummyResult->Columns; x++)
					{
						RealX = DummyResult->LowerLeftX + ((double)x + 0.5) * DummyResult->Resolution;
						for (y = 0; y < DummyResult->Lines; y++)
						{
							RealY = DummyResult->LowerLeftY + ((double)y + 0.5) * DummyResult->Resolution;
							Value = DummyResult->Matrix[0][x][y];
							fprintf(Outfile, "%.2f\t%.2f\t%.2f\n", RealX, RealY, Value);
						}
					}

					fclose(Outfile);
				}
				//write_ascii(*DummyResult, Filename);
				if (Error == 0) {
					/* Copy result. */
					WinProp_CopyResult(&PropResults[Count], DummyResult);
				}
				else {
					/* Error during prediction. Print error message. */
					printf("\n\nSimulation returned with Error %d\n\n", Error);
				}

			}
			// Free memory
			if (Area.Heights)
				free(Area.Heights);
			WinProp_Close(PredictionHandle);
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
			char overNameput[200];
			sprintf(overNameput, API_DATA_FOLDER "output/第%d代第%d个种群坐标.txt", a, i);
			FILE* Outfile = fopen(overNameput, "w");
			if (Outfile)
				fprintf(Outfile, " (% lf, % lf), (% lf, % lf), (% lf, % lf)", chromosome[i][0], chromosome[i][1], chromosome[i][2], chromosome[i][3], chromosome[i][4], chromosome[i][5]);
			fclose(Outfile);
			//write_ascii(*MaxSNIR, overNameput);


			// -----------------------------------------------------------------------------
			// Retrieve results
			// -----------------------------------------------------------------------------
			/* As an example: retrieve max. throughput (kbps) per pixel. */
			if (Error == 0)
			{
				Error = WinProp_Net_NetworkMap_Get(ProjectHandle, -1, NET_RESULT_LTE_RSRQ, &RSRQ);
				Error = WinProp_Net_NetworkMap_Get(ProjectHandle, -1, NET_RESULT_LTE_RSRP, &RSRP);
				Error = WinProp_Net_NetworkMap_Get(ProjectHandle, -1, NET_RESULT_SNIR, &SNIR);
			}
			chromosome_fitness[i] = fitting(*RSRQ, *RSRP, *SNIR, fitness);

			/* Write max. throughput result to ASCII file. */
			/*if (Error == 0)
			{
				char overNameForOutput[200];
				sprintf(overNameForOutput, API_DATA_FOLDER "output/第%d代的%d个种群合并结果.txt", a, i);
				FILE* Outfile = fopen(overNameForOutput, "w");
				if (Outfile)
				{
					for (x = 0; x < MaxPower->Columns; x++)
					{
						RealX = MaxPower->LowerLeftX + ((double)x + 0.5) * MaxPower->Resolution;
						for (y = 0; y < MaxPower->Lines; y++)
						{
							RealY = MaxPower->LowerLeftY + ((double)y + 0.5) * MaxPower->Resolution;
							Value = MaxPower->Matrix[0][x][y];
							fprintf(Outfile, "%.2f\t%.2f\t%.2f\n", RealX, RealY, Value);
						}
					}

				}
			}*/
			fitness_sum += chromosome_fitness[i];
		}
		// 计算平均值
		average_epoch_best_fitness[a + 1] = fitness_sum / chromosome_size;
		// 储存最优表现 
		best_fitness_index = max_in_community(chromosome_fitness, chromosome_size);
		if (best_fitness_index[0] > all_best_fitness)
		{
			all_best_fitness = best_fitness_index[0];
			for (int j = 0; j < chromosome_length; j++)
				epoch_best[j] = chromosome[(int)best_fitness_index[1]][j];
			all_best_epoch = a + 1;
		}

		printf("第%d次迭代：(%lf,%lf),(%lf,%lf),(%lf,%lf)\t\n", a, chromosome[(int)best_fitness_index[1]][0], chromosome[(int)best_fitness_index[1]][1], chromosome[(int)best_fitness_index[1]][2], chromosome[(int)best_fitness_index[1]][3], chromosome[(int)best_fitness_index[1]][4], chromosome[(int)best_fitness_index[1]][5]);
		if (OutputFile)
			fprintf(OutputFile, "第%d次迭代：(%lf,%lf),(%lf,%lf),(%lf,%lf)\n", a, chromosome[(int)best_fitness_index[1]][0], chromosome[(int)best_fitness_index[1]][1], chromosome[(int)best_fitness_index[1]][2], chromosome[(int)best_fitness_index[1]][3], chromosome[(int)best_fitness_index[1]][4], chromosome[(int)best_fitness_index[1]][5]);

	}
	/* Free propagation results. */
	for (Count = 0; Count < MAX_TRX; Count++)
		WinProp_FreeResult(&PropResults[Count]);
	/* Close network project. */
	Error = WinProp_Net_Project_Close(ProjectHandle);
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

void initializeChromosomes(double chromosomes[chromosome_size][chromosome_length]) {
	srand(time(0)); // 初始化随机数种子
	for (int i = 0; i < chromosome_size; i++) {
		for (int j = 0; j < chromosome_length; j++) {
			double lower_bound, upper_bound;
			if (j % 2 == 0) { // 偶数索引代表x坐标
				lower_bound = 0.0;
				upper_bound = 25;
			}
			else { // 奇数索引代表y坐标
				lower_bound = 0.0;
				upper_bound = 15;
			}
			// 生成随机数的工具函数
			chromosomes[i][j] = generateRandomCoordinate(lower_bound, upper_bound);
		}
	}
}

double generateRandomCoordinate(double min, double max) {
	return (max - min) * ((double)rand() / RAND_MAX) + min;
}

void AntennaPropertiesSet(WinProp_Antenna* Antenna, double CoordinateX, double CoordinateY, double Height, double Frequency, char* Name) {
	Antenna->Longitude_X = CoordinateX;
	Antenna->Latitude_Y = CoordinateY;
	Antenna->Height = Height;
	Antenna->Power = 10.0;
	Antenna->PowerMode = 0;
	Antenna->Frequency = Frequency;
	Antenna->Name = Name;
	Antenna->Model = WINPROP_MODEL_DPM;
}

void CarrierPropertiesSet(WinProp_Antenna* Antenna, WinProp_Carrier* Carrier) {
	Antenna->Carriers.CarrierID = Carrier->CarrierID;
	Antenna->Carriers.SystemID = Carrier->SystemID;
	Antenna->Carriers.MimoID = Carrier->MimoID;
}

double fitting(WinProp_Result Resultmatrix1, WinProp_Result Resultmatrix2, WinProp_Result Resultmatrix3, double fitness) {
	// 定义矩形区域  
	RectRegion regions[] = //minX, minY, maxX, maxY
	{
		{4.5,0.5,24.5,7.5}// 同上，负坐标需要特别注意  
	};
	int numRegions = sizeof(regions) / sizeof(RectRegion);

	// 遍历Resultmatrix的每一个点  
	for (int x = -0.5; x < Resultmatrix1.Columns; x++) {
		for (int y = -0.5; y < Resultmatrix1.Lines; y++)
		{
			int inSpecialRegion = 0;
			// 检查点是否在任何一个特殊区域内  
			for (int i = 0; i < numRegions; i++)
			{
				if (x >= regions[i].minX && x <= regions[i].maxX && y >= regions[i].minY && y <= regions[i].maxY) {
					inSpecialRegion = 1;
					break;
				}
			}

			if (inSpecialRegion)
			{
				fitness = fitness + 20 * (Resultmatrix1.Matrix[0][x][y] + 19.5) / 16.5 + 20 * (Resultmatrix2.Matrix[0][x][y] + 140) / 96 + 20 * Resultmatrix3.Matrix[0][x][y] / 100;
			}
			else
			{
				fitness = fitness + (Resultmatrix1.Matrix[0][x][y] + 19.5) / 16.5 + (Resultmatrix2.Matrix[0][x][y] + 140) / 96 + Resultmatrix3.Matrix[0][x][y] / 100;
			}
		}
	}
	return fitness;
}



double* max_in_community(double fit[], int size) {
	static double result[2];
	result[0] = *fit;
	result[1] = 0;
	for (int i = 0; i < size; i++)
		if (fit[i] > result[0]) {
			result[0] = fit[i];
			result[1] = i;
		};
	return result;
}

void cross(double chromosome[chromosome_size][chromosome_length])
{
	// 用于记录父代染色体的数组
	double parents[chromosome_size][chromosome_length];
	int num[500];
	int number = 0;
	for (int i = 0; i < chromosome_size; i++)
	{
		// 上来先判断是否进行交叉操作，大于交叉概率就跳过 
		double pick = ((double)rand()) / RAND_MAX;
		if (pick > cross_probability)
		{
			number++;
			num[number] = i;
			// 记录下对应的染色体为父代染色体
			for (int j = 0; j < chromosome_length; j++)
			{
				parents[i][j] = chromosome[i][j];
			}
			continue;
		}
	}
	if (number >= 2)
	{
		int parent1, parent2;
		parent1 = rand() % (number - 1) + 1;
		parent2 = rand() % (number - 1) + 1;
		while (parent2 == parent1)
			parent2 = rand() % number + 1;
		int crossover_point = rand() % chromosome_length;
		for (int j = 0; j < chromosome_size; j++)
		{
			if (j < crossover_point)
			{
				chromosome[num[parent2]][j] = parents[num[parent1]][j];
				chromosome[num[parent1]][j] = parents[num[parent2]][j];
			}
		}
	}
}


// 变异 
void mutation(double chromosomes[chromosome_size][chromosome_length]) {
	for (int i = 0; i < chromosome_size; i++) {
		// 判断是否进行变异操作，大于变异概率就跳过
		double pick = ((double)rand()) / RAND_MAX;
		if (pick > mutation_probability)
			continue;

		// 随机抽取染色体的一个位置
		int where = (int)((chromosome_length - 1) * ((double)rand()) / RAND_MAX);

		// 根据基因位置确定上下限
		double lower_bound, upper_bound;
		if (where % 2 == 0) {
			// x 的范围
			lower_bound = 0;
			upper_bound = 25;
		}
		else {
			// y 的范围
			lower_bound = 0;
			upper_bound = 15;
		}

		// 计算当前基因值的上限和下限差距
		double current_value = chromosomes[i][where];
		double v1 = upper_bound - current_value;  // 向上限变异的潜力
		double v2 = current_value - lower_bound;  // 向下限变异的潜力

		// 计算变异幅度的随机权重和动态因子
		double r = ((double)rand()) / RAND_MAX;
		double r1 = ((double)rand()) / RAND_MAX;

		// 计算变异幅度
		double mutation_amount = v1 * r1 * (1 - ((double)i) / epoch_max) * (1 - ((double)i) / epoch_max);

		// 应用变异，并根据方向和范围限制进行调整
		if (r >= 0.5) {
			// 增加基因值
			if (chromosomes[i][where] + mutation_amount > upper_bound)
				chromosomes[i][where] = upper_bound;
			else
				chromosomes[i][where] += mutation_amount;
		}
		else {
			// 减少基因值
			if (chromosomes[i][where] + mutation_amount < lower_bound)
				chromosomes[i][where] = lower_bound;
			else
				chromosomes[i][where] += mutation_amount;
		}
	}
}

