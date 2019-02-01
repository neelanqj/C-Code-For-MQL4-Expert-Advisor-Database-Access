//+------------------------------------------------------------------+
//|                                              Sample DLL for MQL4 |
//|                 Copyright c 2004-2008, MetaQuotes Software Corp. |
//|                                        https://www.metaquotes.net |
//+------------------------------------------------------------------+

//+------------------------------------------------------------------+
//
//  YURAZ 2008  YZMSSQLExpertSample
//
//  Example DLL Integrating  MT4 with MS SQL 2000
//
//  ADO  MS SQL SERVER
//
//  software used
//
//  VISUAL C++ 6 , SP5 ,  MDAC 7 ,  MS SQL2000 + SP4
//
//+------------------------------------------------------------------+

#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
//----
#define MT4_EXPFUNC __declspec(dllexport)
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
#pragma pack(push,1)

struct RateInfo
{
	unsigned int      ctm;
	double            open;
	double            low;
	double            high;
	double            close;
	double            vol;
	double            vol1;
	double            vol2;
	double            vol3;
	double            vol4;
	double            vol5;

};

#pragma pack(pop)

struct MqlStr
{
	int               len;
	char             *string;
};

static int CompareMqlStr(const void *left, const void *right);

static int SQLexecProcedure(char *nprc);
static int SQLexecProcedureSignal(char *sSymbol, char* sProcedure);
// static int _YZSQLsqlstrinsql( char *Symbol , unsigned int DateTime , double Ask, double Bid, char *NamePrc );
static int _YZSQLprocedure(char *sSymbol, unsigned int pDateTime, double Ask, double Bid, char *NamePrc);
static int _YZSQLprocedureHISTORYPut(char *Symbol, unsigned int Period, unsigned int DateTime, double Open,
	double High, double  Low, double Close, double Volume, unsigned int Bar, char *Procedure);


//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	//----
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	//----
	return(TRUE);
}

// place ticks in MS SQL
// call the procedure as an SQL line passing parameters   "exec YZ_MT4_TICK ?,?,?,?"
/*
MT4_EXPFUNC int  __stdcall SQLsqlstringTickPut(char *Symbol,unsigned int DateTime,double Ask,double Bid,char *sSQLstring)
{
int ccc =  _YZSQLsqlstrinsql( Symbol  , DateTime ,  Ask , Bid  , sSQLstring  );
return(ccc);
}
*/

// call as a procedure passing parameters
MT4_EXPFUNC int  __stdcall SQLProcedureTickPut(char *Symbol, unsigned int DateTime, double Ask, double Bid, char *Procedure)
{
	int ccc = _YZSQLprocedure(Symbol, DateTime, Ask, Bid, Procedure);
	return(ccc);
}

// place a specific candlestick in MS SQL history
MT4_EXPFUNC int  __stdcall SQLProcedureHistoryPut(char *Symbol, unsigned int Period, unsigned int DateTime,
	double Open, double High, double  Low, double Close, double Volume, unsigned int Bar, char *Procedure)
{
	int ccc = _YZSQLprocedureHISTORYPut(Symbol, Period, DateTime, Open, High, Low, Close, Volume, Bar, Procedure);
	return(ccc);
}


// call procedure sProcedure
// 
// return -1 error
//
MT4_EXPFUNC int  __stdcall SQLProcedureGetInt(char *sProcedure)
{
	int Ret = SQLexecProcedure(sProcedure);
	return((int)Ret);
}


MT4_EXPFUNC int  __stdcall SQLProcedureGetSignal(char *sSymbol, char *sProcedure)
{

	int Ret = SQLexecProcedureSignal(sSymbol, sProcedure);
	return((int)Ret);
}

//////////////////////////////////
#include "stdafx.h"
#include <stdio.h>
#import "C:\Program Files\Common Files\System\ado\msado20.tlb" \
        rename("EOF","ADOEOF") rename("BOF","ADOBOF")

using namespace ADODB;

inline void TESTHR(HRESULT x) { if FAILED(x) _com_issue_error(x); };

// procedure call method
int _YZSQLprocedure(char *sSymbol, unsigned int pDateTime, double Ask, double Bid, char *NamePrc)
{

	HRESULT hr = S_OK;
	_CommandPtr pCmd = NULL;
	_ConnectionPtr pConnection = NULL;
	_bstr_t strMessage, strAuthorID;

	::CoInitialize(NULL);

	long codRet = -1;

	try {

		_ParameterPtr Par1;
		_ParameterPtr Par2;
		_ParameterPtr Par3;
		_ParameterPtr Par4;
		_ParameterPtr Par5;

		TESTHR(pConnection.CreateInstance(__uuidof(Connection)));
		hr = pConnection->Open("dsn=MT4_SQL_BASE;", "ForexMaster", "think123", adConnectUnspecified);
		pConnection->CursorLocation = adUseClient;
		TESTHR(pCmd.CreateInstance(__uuidof(Command)));
		pCmd->CommandText = NamePrc;  // procedure name
		pCmd->CommandType = adCmdStoredProc;

		Par1 = pCmd->CreateParameter(_bstr_t("@P1"), adInteger, adParamOutput, 0, codRet);
		pCmd->Parameters->Append(Par1);
		Par1 = pCmd->CreateParameter("@psSymbol", adChar, adParamInput, strlen(sSymbol), sSymbol);
		pCmd->Parameters->Append(Par1);
		Par2 = pCmd->CreateParameter("@piDateTime", adDouble, adParamInput, sizeof(double), (double)pDateTime);
		pCmd->Parameters->Append(Par2);
		Par3 = pCmd->CreateParameter("@pdAsk", adDouble, adParamInput, 4, Ask);
		pCmd->Parameters->Append(Par3);
		Par4 = pCmd->CreateParameter("@pdBid", adDouble, adParamInput, 4, Bid);
		pCmd->Parameters->Append(Par4);


		pCmd->ActiveConnection = pConnection;
		int hr = pCmd->Execute(0, 0, adCmdStoredProc);
		if (FAILED(hr))
		{
			codRet = -1;
		}
		else
		{
			Par1 = pCmd->Parameters->GetItem(_bstr_t("@P1"));     // obtain from the procedure
			codRet = Par1->GetValue();
		}
	}
	catch (_com_error) {
		//
		// if necessary, process the execution error
		//
		codRet = -1;

	}
	if (pConnection)
		if (pConnection->State == adStateOpen)
			pConnection->Close();

	::CoUninitialize();
	return((int)codRet);
}



// place in history Symbol , Period . DateTime, Open , High , Low , Close , Value , Bar
int _YZSQLprocedureHISTORYPut(char *pSymbol, unsigned int pPeriod, unsigned int pDateTime, double pOpen, double pHigh,
	double  pLow, double pClose, double pVolume, unsigned int pBar, char *pProcedure)
{


	HRESULT hr = S_OK;
	_CommandPtr pCmd = NULL;
	_ConnectionPtr pConnection = NULL;
	_bstr_t strMessage, strAuthorID;

	::CoInitialize(NULL);

	long codRet = -1;

	try {

		_ParameterPtr ParReturn; // 
		_ParameterPtr Par1; // SYMBOL
		_ParameterPtr Par2; // PERIOD
		_ParameterPtr Par3; // DATETIME
		_ParameterPtr Par4; // OPEN
		_ParameterPtr Par5; // HIGH
		_ParameterPtr Par6; // LOW
		_ParameterPtr Par7; // CLOSE
		_ParameterPtr Par8; // VOLUME
		_ParameterPtr Par9; // BAR


		TESTHR(pConnection.CreateInstance(__uuidof(Connection)));
		hr = pConnection->Open("dsn=MT4_SQL_BASE;", "ForexMaster", "think123", adConnectUnspecified);
		pConnection->CursorLocation = adUseClient;
		TESTHR(pCmd.CreateInstance(__uuidof(Command)));
		pCmd->CommandText = pProcedure;  // procedure name
		pCmd->CommandType = adCmdStoredProc;

		ParReturn = pCmd->CreateParameter(_bstr_t("@P1"), adInteger, adParamOutput, 0, codRet);
		pCmd->Parameters->Append(ParReturn);

		Par1 = pCmd->CreateParameter("@psSymbol", adChar, adParamInput, strlen(pSymbol), pSymbol);
		pCmd->Parameters->Append(Par1);

		Par2 = pCmd->CreateParameter("@piDateTime", adDouble, adParamInput, sizeof(double), (double)pPeriod);
		pCmd->Parameters->Append(Par2);

		Par3 = pCmd->CreateParameter("@piDateTime", adDouble, adParamInput, sizeof(double), (double)pDateTime);
		pCmd->Parameters->Append(Par3);

		Par4 = pCmd->CreateParameter("@pdOpen", adDouble, adParamInput, 4, pOpen);
		pCmd->Parameters->Append(Par4);

		Par5 = pCmd->CreateParameter("@pdHigh", adDouble, adParamInput, 4, pHigh);
		pCmd->Parameters->Append(Par5);

		Par6 = pCmd->CreateParameter("@pdLow", adDouble, adParamInput, 4, pLow);
		pCmd->Parameters->Append(Par6);

		Par7 = pCmd->CreateParameter("@pdClose", adDouble, adParamInput, 4, pClose);
		pCmd->Parameters->Append(Par7);

		Par8 = pCmd->CreateParameter("@pdVolume", adDouble, adParamInput, 4, pVolume);
		pCmd->Parameters->Append(Par8);

		Par9 = pCmd->CreateParameter("@piBar", adDouble, adParamInput, sizeof(double), (double)pBar);
		pCmd->Parameters->Append(Par9);


		pCmd->ActiveConnection = pConnection;
		int hr = pCmd->Execute(0, 0, adCmdStoredProc);
		if (FAILED(hr))
		{
			codRet = -1;
		}
		else
		{
			ParReturn = pCmd->Parameters->GetItem(_bstr_t("@P1"));     // obtain from the procedure
			codRet = ParReturn->GetValue();
		}
	}
	catch (_com_error) {
		//
		// if necessary, process the execution error
		//
		codRet = -1;

	}
	if (pConnection)
		if (pConnection->State == adStateOpen)
			pConnection->Close();

	::CoUninitialize();
	return((int)codRet);
}


//
// return the value returned by the procedure
//
int  SQLexecProcedure(char *nprc)
{

	HRESULT hr = S_OK;
	_CommandPtr pcmd = NULL;
	_ConnectionPtr pConnection = NULL;
	_bstr_t strMessage, strAuthorID;

	::CoInitialize(NULL);

	long codRet = -1;

	try {
		TESTHR(pConnection.CreateInstance(__uuidof(Connection)));
		hr = pConnection->Open("dsn=MT4_SQL_BASE;", "ForexMaster", "think123", adConnectUnspecified);
		pConnection->CursorLocation = adUseClient;
		TESTHR(pcmd.CreateInstance(__uuidof(Command)));
		pcmd->CommandText = nprc;  // procedure name
		pcmd->CommandType = adCmdStoredProc;

		_ParameterPtr  pParm1 = pcmd->CreateParameter(_bstr_t("@P1"), adInteger, adParamOutput, 0, codRet);
		pcmd->Parameters->Append(pParm1);
		pcmd->ActiveConnection = pConnection;
		int hr = pcmd->Execute(0, 0, adCmdStoredProc);
		if (FAILED(hr))
		{
			codRet = -1;
		}
		else
		{
			pParm1 = pcmd->Parameters->GetItem(_bstr_t("@P1"));     // obtain from the procedure
			codRet = pParm1->GetValue();
		}
	}
	catch (_com_error) {
		//
		// if necessary, process the execution error
		//
		codRet = -1;

	}
	if (pConnection)
		if (pConnection->State == adStateOpen)
			pConnection->Close();

	::CoUninitialize();
	return((int)codRet);
}

//
//
//
int  SQLexecProcedureSignal(char *sSymbol, char* sProcedure)
{

	HRESULT hr = S_OK;
	_CommandPtr pcmd = NULL;
	_ConnectionPtr pConnection = NULL;
	_bstr_t strMessage;
	_bstr_t strAuthorID;

	::CoInitialize(NULL);

	long codRet = 0;

	try {
		TESTHR(pConnection.CreateInstance(__uuidof(Connection)));
		hr = pConnection->Open("dsn=MT4_SQL_BASE;", "ForexMaster", "think123", adConnectUnspecified);
		pConnection->CursorLocation = adUseClient;
		TESTHR(pcmd.CreateInstance(__uuidof(Command)));
		pcmd->CommandText = sProcedure;  // procedure name
		pcmd->CommandType = adCmdStoredProc;

		_ParameterPtr pParm1 = pcmd->CreateParameter("@psSymbol", adChar, adParamInput, strlen(sSymbol), sSymbol);
		pcmd->Parameters->Append(pParm1);
		_ParameterPtr pParm2 = pcmd->CreateParameter(_bstr_t("@P1"), adInteger, adParamOutput, 0, codRet);
		pcmd->Parameters->Append(pParm2);
		pcmd->ActiveConnection = pConnection;
		int hr = pcmd->Execute(0, 0, adCmdStoredProc);
		if (FAILED(hr))
		{
			bool bSuccess = false;
		}
		pParm2 = pcmd->Parameters->GetItem(_bstr_t("@P1"));     // obtain from the procedure
		codRet = pParm2->GetValue();

		//             printf("\n [%d] \n",codRet );       // OBTAINING from the procedure
	}
	catch (_com_error) {
		//
		// if necessary, process the execution error
		//
	}
	if (pConnection)
		if (pConnection->State == adStateOpen)
			pConnection->Close();

	::CoUninitialize();
	return((int)codRet);
}