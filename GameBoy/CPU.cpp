#include "CPU.h"


#define From11Borrow12 0x100
#define From3Borrow4 0x10
#define HCarryFrom(val, change, bit) (((val & (bit - 0x01)) + (change & (bit - 0x01))) & bit) == bit
#define HCarryBorrow(val, change, bit) (((val & (bit - 0x01)) - (change & (bit - 0x01))) & bit) == bit
/*#define math8(N, C, d1, d2, f, hCarried, cCarried) \
if(N == 0) \
{\
	cCarried = (((uint16)d2 + d1 + C) >> 8) & 0x01;\
	if(d1 + C > 0x0f) { hCarried = true; }\
	else { hCarried = HCarryFrom(d2, d1 + C, From3Borrow4); } \
	d2 = d2 + d1 + C;\
}\
else \
{\
	cCarried = (((uint16) ((0xff - d2) & 0x0f) + ((0xff - d1) & 0x0f) + ((0xff - C) & 0x0f)) & 0xf0 >> 4 & 0x01);\
	if(d1 + C > 0x0f) { hCarried = true; }\
	else { hCarried = HCarryBorrow(d2, d1 + C, From11Borrow12); } \
	d2 = d2 - d1 - C; \
}\
Set_Bit(f, 6, (d2 == 0)); \
Set_Bit(f, 6, N); \
Set_Bit(f, 5, hCarried); \
Set_Bit(f, 4, cCarried);*/

void CPU::math8(BOOL method, BOOL C, uint16 dataChange, uint8& dataRef, uint8& f, BOOL& hCarried, BOOL& cCarried)
{
	if (method == 0)
	{
		uint16 cCarriedT = 0x0000 + dataRef;
		cCarriedT += (dataChange + C);
		cCarriedT &= 0x0F00;
		cCarriedT >>= 8;
		cCarriedT &= 0x01;
		cCarried = (cCarriedT == 0x0001) ? TRUE : FALSE;
		//cCarried = (((uint16)dataRef + dataChange + C) >> 8) & 0x01; 
		if (dataChange + C > 0x0f) { hCarried = true; }
		else { hCarried = HCarryFrom(dataRef, dataChange + C, From3Borrow4); }
			dataRef = dataRef + dataChange + C;
	}
	else 
	{
		uint16 cCarriedT = 0x0100 + dataRef;
		cCarriedT -= (dataChange + C);
		cCarriedT &= 0x0F00;
		cCarriedT >>= 8;
		cCarriedT &= 0x01;
		cCarried = (cCarriedT == 0x0001)? FALSE : TRUE;
		//cCarried = (((uint16)((0xff - dataRef) & 0x0f) + ((0xff - dataChange) & 0x0f) + ((0xff - C) & 0x0f)) & 0xf0 >> 4 & 0x01);
		if (dataChange + C > 0x0f) { hCarried = true; }
		else { hCarried = HCarryBorrow(dataRef, dataChange + C, From3Borrow4); }
			dataRef = dataRef - dataChange - C;
	}
	Set_Bit(_registers.F, 6, (dataRef == 0));
	Set_Bit(_registers.F, 6, method);
	Set_Bit(_registers.F, 5, hCarried);
	Set_Bit(_registers.F, 4, cCarried);
}

Input* input;
char* opcodeNames[0xff];

CPU::CPU(MMU* mmu)
{
	_mmu = mmu;
	input = new Input();
	Reset();
}

CPU::~CPU()
{
}

void CPU::Reset()
{
	for (uint8 i = 0; i < 0xFF; i++)
	{
		_opcodeTable[i] = &CPU::NOP;
		_cbOpcodeTable[i] = &CPU::NOP;
		opcodeNames[i] = "NOP";
	}

	opcodeNames[0x01] = "LD_BC_RR";		//0x01 LD BC,d16
	opcodeNames[0x02] = "LD_BC_A";		//0x02 LD (BC),A
	opcodeNames[0x03] = "INC_BC";		//0x03 INC BC
	opcodeNames[0x04] = "INC_B";		//0x04 INC B
	opcodeNames[0x05] = "DEC_B";		//0x05 DEC B
	opcodeNames[0x06] = "LD_B_R";		//0x06 LD B,d8
	opcodeNames[0x07] = "RL_CA";		//0x07
	opcodeNames[0x08] = "LD_RR_SP";		//0x08
	opcodeNames[0x09] = "ADD_HL_BC";	//0x09
	opcodeNames[0x0A] = "LD_A_BC";		//0x0A
	opcodeNames[0x0B] = "DEC_BC";		//0x0B
	opcodeNames[0x0C] = "INC_C";		//0x0C
	opcodeNames[0x0D] = "DEC_C";		//0x0D
	opcodeNames[0x0E] = "LD_C_R";		//0x0E
	opcodeNames[0x0F] = "RR_CA";		//0x0F

	opcodeNames[0x10] = "STOP";			//0x10
	opcodeNames[0x11] = "LD_DE_RR";		//0x11
	opcodeNames[0x12] = "LD_DE_A";		//0x12
	opcodeNames[0x13] = "INC_DE";		//0x13
	opcodeNames[0x14] = "INC_D";		//0x14
	opcodeNames[0x15] = "DEC_D";		//0x15
	opcodeNames[0x16] = "LD_D_R";		//0x16
	opcodeNames[0x17] = "RL_A";			//0x17
	opcodeNames[0x18] = "JR_R";			//0x18
	opcodeNames[0x19] = "ADD_HL_DE";	//0x19
	opcodeNames[0x1A] = "LD_A_DE";		//0x1A
	opcodeNames[0x1B] = "DEC_DE";		//0x1B
	opcodeNames[0x1C] = "INC_E";		//0x1C
	opcodeNames[0x1D] = "DEC_E";		//0x1D
	opcodeNames[0x1E] = "LD_E_R";		//0x1E
	opcodeNames[0x1F] = "RR_A";			//0x1F

	opcodeNames[0x20] = "JR_NZ_R";		//0x20
	opcodeNames[0x21] = "LD_HL_RR";		//0x21
	opcodeNames[0x22] = "LD_MM_PLUS_A";//0x22
	opcodeNames[0x23] = "INC_HL";		//0x23
	opcodeNames[0x24] = "INC_H";		//0x24
	opcodeNames[0x25] = "DEC_H";		//0x25
	opcodeNames[0x26] = "LD_H_R";		//0x26
	opcodeNames[0x27] = "DA_A";			//0x27
	opcodeNames[0x28] = "JR_Z_R";		//0x28
	opcodeNames[0x29] = "ADD_HL_HL";	//0x29
	opcodeNames[0x2A] = "LD_A_MM_PLUS";//0x2A
	opcodeNames[0x2B] = "DEC_HL";		//0x2B
	opcodeNames[0x2C] = "INC_L";		//0x2C
	opcodeNames[0x2D] = "DEC_L";		//0x2D
	opcodeNames[0x2E] = "LD_L_R";		//0x2E
	opcodeNames[0x2F] = "CPL";			//0x2F

	opcodeNames[0x30] = "JR_NC_R";		//0x30
	opcodeNames[0x31] = "LD_SP_RR";		//0x31
	opcodeNames[0x32] = "LD_MM_MIN_A";	//0x32
	opcodeNames[0x33] = "INC_SP";		//0x33
	opcodeNames[0x34] = "INC_MM";		//0x34
	opcodeNames[0x35] = "DEC_MM";		//0x35
	opcodeNames[0x36] = "LD_MM_R";		//0x36
	opcodeNames[0x37] = "SCF";			//0x37
	opcodeNames[0x38] = "JR_C_R";		//0x38
	opcodeNames[0x39] = "ADD_HL_SP";	//0x39
	opcodeNames[0x3A] = "LD_A_MM_MIN";	//0x3A
	opcodeNames[0x3B] = "DEC_SP";		//0x3B
	opcodeNames[0x3C] = "INC_A";		//0x3C
	opcodeNames[0x3D] = "DEC_A";		//0x3D
	opcodeNames[0x3E] = "LD_A_R";		//0x3E
	opcodeNames[0x3F] = "CCF";			//0x3F

	opcodeNames[0x40] = "LD_B_B";		//0x40
	opcodeNames[0x41] = "LD_B_C";		//0x41
	opcodeNames[0x42] = "LD_B_D";		//0x42
	opcodeNames[0x43] = "LD_B_E";		//0x43
	opcodeNames[0x44] = "LD_B_H";		//0x44
	opcodeNames[0x45] = "LD_B_L";		//0x45
	opcodeNames[0x46] = "LD_B_MM";		//0x46
	opcodeNames[0x47] = "LD_B_A";		//0x47
	opcodeNames[0x48] = "LD_C_B";		//0x48
	opcodeNames[0x49] = "LD_C_C";		//0x49
	opcodeNames[0x4A] = "LD_C_D";		//0x4A
	opcodeNames[0x4B] = "LD_C_E";		//0x4B
	opcodeNames[0x4C] = "LD_C_H";		//0x4C
	opcodeNames[0x4D] = "LD_C_L";		//0x4D
	opcodeNames[0x4E] = "LD_C_MM";		//0x4E
	opcodeNames[0x4F] = "LD_C_A";		//0x4F

	opcodeNames[0x50] = "LD_D_B";		//0x50
	opcodeNames[0x51] = "LD_D_C";		//0x51
	opcodeNames[0x52] = "LD_D_D";		//0x52
	opcodeNames[0x53] = "LD_D_E";		//0x53
	opcodeNames[0x54] = "LD_D_H";		//0x54
	opcodeNames[0x55] = "LD_D_L";		//0x55
	opcodeNames[0x56] = "LD_D_MM";		//0x56
	opcodeNames[0x57] = "LD_D_A";		//0x57
	opcodeNames[0x58] = "LD_E_B";		//0x58
	opcodeNames[0x59] = "LD_E_C";		//0x59
	opcodeNames[0x5A] = "LD_E_D";		//0x5A
	opcodeNames[0x5B] = "LD_E_E";		//0x5B
	opcodeNames[0x5C] = "LD_E_H";		//0x5C
	opcodeNames[0x5D] = "LD_E_L";		//0x5D
	opcodeNames[0x5E] = "LD_E_MM";		//0x5E
	opcodeNames[0x5F] = "LD_E_A";		//0x5F

	opcodeNames[0x60] = "LD_H_B";		//0x60
	opcodeNames[0x61] = "LD_H_C";		//0x61
	opcodeNames[0x62] = "LD_H_D";		//0x62
	opcodeNames[0x63] = "LD_H_E";		//0x63
	opcodeNames[0x64] = "LD_H_H";		//0x64
	opcodeNames[0x65] = "LD_H_L";		//0x65
	opcodeNames[0x66] = "LD_H_MM";		//0x66
	opcodeNames[0x67] = "LD_H_A";		//0x67
	opcodeNames[0x68] = "LD_L_B";		//0x68
	opcodeNames[0x69] = "LD_L_C";		//0x69
	opcodeNames[0x6A] = "LD_L_D";		//0x6A
	opcodeNames[0x6B] = "LD_L_E";		//0x6B
	opcodeNames[0x6C] = "LD_L_H";		//0x6C
	opcodeNames[0x6D] = "LD_L_L";		//0x6D
	opcodeNames[0x6E] = "LD_L_MM";		//0x6E
	opcodeNames[0x6F] = "LD_L_A";		//0x6F

	opcodeNames[0x70] = "LD_MM_B";		//0x70
	opcodeNames[0x71] = "LD_MM_C";		//0x71
	opcodeNames[0x72] = "LD_MM_D";		//0x72
	opcodeNames[0x73] = "LD_MM_E";		//0x73
	opcodeNames[0x74] = "LD_MM_H";		//0x74
	opcodeNames[0x75] = "LD_MM_L";		//0x75
	opcodeNames[0x76] = "HALT";			//0x76
	opcodeNames[0x77] = "LD_MM_A";		//0x77
	opcodeNames[0x78] = "LD_A_B";		//0x78
	opcodeNames[0x79] = "LD_A_C";		//0x79
	opcodeNames[0x7A] = "LD_A_D";		//0x7A
	opcodeNames[0x7B] = "LD_A_E";		//0x7B
	opcodeNames[0x7C] = "LD_A_H";		//0x7C
	opcodeNames[0x7D] = "LD_A_L";		//0x7D
	opcodeNames[0x7E] = "LD_A_MM";		//0x7E
	opcodeNames[0x7F] = "LD_A_A";		//0x7F


	opcodeNames[0x80] = "ADD_A_B";		//0x80
	opcodeNames[0x81] = "ADD_A_C";		//0x81
	opcodeNames[0x82] = "ADD_A_D";		//0x82
	opcodeNames[0x83] = "ADD_A_E";		//0x83
	opcodeNames[0x84] = "ADD_A_H";		//0x84
	opcodeNames[0x85] = "ADD_A_L";		//0x85
	opcodeNames[0x86] = "ADD_A_MM";		//0x86
	opcodeNames[0x87] = "ADD_A_A";		//0x87
	opcodeNames[0x88] = "ADC_A_B";		//0x88
	opcodeNames[0x89] = "ADC_A_C";		//0x89
	opcodeNames[0x8A] = "ADC_A_D";		//0x8A
	opcodeNames[0x8B] = "ADC_A_E";		//0x8B
	opcodeNames[0x8C] = "ADC_A_H";		//0x8C
	opcodeNames[0x8D] = "ADC_A_L";		//0x8D
	opcodeNames[0x8E] = "ADC_A_MM";		//0x8E
	opcodeNames[0x8F] = "ADC_A_A";		//0x8F

	opcodeNames[0x90] = "SUB_A_B";		//0x90
	opcodeNames[0x91] = "SUB_A_C";		//0x91
	opcodeNames[0x92] = "SUB_A_D";		//0x92
	opcodeNames[0x93] = "SUB_A_E";		//0x93
	opcodeNames[0x94] = "SUB_A_H";		//0x94
	opcodeNames[0x95] = "SUB_A_L";		//0x95
	opcodeNames[0x96] = "SUB_A_MM";		//0x96
	opcodeNames[0x97] = "SUB_A_A";		//0x97
	opcodeNames[0x98] = "SBC_A_B";		//0x98
	opcodeNames[0x99] = "SBC_A_C";		//0x99
	opcodeNames[0x9A] = "SBC_A_D";		//0x9A
	opcodeNames[0x9B] = "SBC_A_E";		//0x9B
	opcodeNames[0x9C] = "SBC_A_H";		//0x9C
	opcodeNames[0x9D] = "SBC_A_L";		//0x9D
	opcodeNames[0x9E] = "SBC_A_MM";		//0x9E
	opcodeNames[0x9F] = "SBC_A_A";		//0x9F

	opcodeNames[0xA0] = "AND_B";		//0xA0
	opcodeNames[0xA1] = "AND_C";		//0xA1
	opcodeNames[0xA2] = "AND_D";		//0xA2
	opcodeNames[0xA3] = "AND_E";		//0xA3
	opcodeNames[0xA4] = "AND_H";		//0xA4
	opcodeNames[0xA5] = "AND_L";		//0xA5
	opcodeNames[0xA6] = "AND_MM";		//0xA6
	opcodeNames[0xA7] = "AND_A";		//0xA7
	opcodeNames[0xA8] = "XOR_B";		//0xA8
	opcodeNames[0xA9] = "XOR_C";		//0xA9
	opcodeNames[0xAA] = "XOR_D";		//0xAA
	opcodeNames[0xAB] = "XOR_E";		//0xAB
	opcodeNames[0xAC] = "XOR_H";		//0xAC
	opcodeNames[0xAD] = "XOR_L";		//0xAD
	opcodeNames[0xAE] = "XOR_MM";		//0xAE
	opcodeNames[0xAF] = "XOR_A";		//0xAF

	opcodeNames[0xB0] = "OR_B";			//0xB0
	opcodeNames[0xB1] = "OR_C";			//0xB1
	opcodeNames[0xB2] = "OR_D";			//0xB2
	opcodeNames[0xB3] = "OR_E";			//0xB3
	opcodeNames[0xB4] = "OR_H";			//0xB4
	opcodeNames[0xB5] = "OR_L";			//0xB5
	opcodeNames[0xB6] = "OR_MM";		//0xB6
	opcodeNames[0xB7] = "OR_A";			//0xB7
	opcodeNames[0xB8] = "CP_B";			//0xB8
	opcodeNames[0xB9] = "CP_C";			//0xB9
	opcodeNames[0xBA] = "CP_D";			//0xBA
	opcodeNames[0xBB] = "CP_E";			//0xBB
	opcodeNames[0xBC] = "CP_H";			//0xBC
	opcodeNames[0xBD] = "CP_L";			//0xBD
	opcodeNames[0xBE] = "CP_MM";		//0xBE
	opcodeNames[0xBF] = "CP_A";			//0xBF

	opcodeNames[0xC0] = "RET_NZ";		//0xC0
	opcodeNames[0xC1] = "POP_BC";		//0xC1
	opcodeNames[0xC2] = "JP_NZ_RR";		//0xC2
	opcodeNames[0xC3] = "JP_RR";		//0xC3
	opcodeNames[0xC4] = "CALL_NZ_RR";	//0xC4
	opcodeNames[0xC5] = "PUSH_BC";		//0xC5
	opcodeNames[0xC6] = "ADD_A_R";		//0xC6
	opcodeNames[0xC7] = "RST_00H";		//0xC7
	opcodeNames[0xC8] = "RET_Z";		//0xC8
	opcodeNames[0xC9] = "RET";			//0xC9
	opcodeNames[0xCA] = "JP_Z_RR";		//0xCA
	opcodeNames[0xCB] = "PREFIX_CB";	//0xCB
	opcodeNames[0xCC] = "CALL_Z_RR";	//0xCC
	opcodeNames[0xCD] = "CALL_RR";		//0xCD
	opcodeNames[0xCE] = "ADC_A_R";		//0xCE
	opcodeNames[0xCF] = "RST_08H";		//0xCF

	opcodeNames[0xD0] = "RET_NC";		//0xD0
	opcodeNames[0xD1] = "POP_DE";		//0xD1
	opcodeNames[0xD2] = "JP_NC_RR";		//0xD2
	opcodeNames[0xD4] = "CALL_NC_RR";	//0xD4
	opcodeNames[0xD5] = "PUSH_DE";		//0xD5
	opcodeNames[0xD6] = "SUB_R";		//0xD6
	opcodeNames[0xD7] = "RST_10H";		//0xD7
	opcodeNames[0xD8] = "RET_C";		//0xD8
	opcodeNames[0xD9] = "RETI";			//0xD9
	opcodeNames[0xDA] = "JP_C_RR";		//0xDA
	opcodeNames[0xDC] = "CALL_C_RR";	//0xDC
	opcodeNames[0xDE] = "SBC_A_R";		//0xDE
	opcodeNames[0xDF] = "RST_18H";		//0xDF

	opcodeNames[0xE0] = "LDH_R_A";		//0xE0
	opcodeNames[0xE1] = "POP_HL";		//0xE1
	opcodeNames[0xE2] = "LD_C_A2";		//0xE2
	opcodeNames[0xE5] = "PUSH_HL";		//0xE5
	opcodeNames[0xE6] = "AND_R";		//0xE6
	opcodeNames[0xE7] = "RST_20H";		//0xE7
	opcodeNames[0xE8] = "ADD_SP_R";		//0xE8
	opcodeNames[0xE9] = "JP_MM";		//0xE9
	opcodeNames[0xEA] = "LD_RR_A";		//0xEA
	opcodeNames[0xEE] = "XOR_R";		//0xEE
	opcodeNames[0xEF] = "RST_28H";		//0xEF

	opcodeNames[0xF0] = "LDH_A_R";		//0xF0
	opcodeNames[0xF1] = "POP_AF";		//0xF1
	opcodeNames[0xF2] = "LD_A_C2";		//0xF2
	opcodeNames[0xF3] = "DI";			//0xF3
	opcodeNames[0xF5] = "PUSH_AF";		//0xF5
	opcodeNames[0xF6] = "OR_R";			//0xF6
	opcodeNames[0xF7] = "RST_30H";		//0xF7
	opcodeNames[0xF8] = "LD_HL_SPandR";	//0xF8
	opcodeNames[0xF9] = "LD_SP_HL";		//0xF9
	opcodeNames[0xFA] = "LD_A_RR";		//0xFA
	opcodeNames[0xFB] = "EI";			//0xFB
	opcodeNames[0xFE] = "CP_R";			//0xFE
	opcodeNames[0xFF] = "RST_38H";		//0xFF

	_opcodeTable[0x01] = &CPU::LD_BC_RR;	//0x01 LD BC,d16
	_opcodeTable[0x02] = &CPU::LD_BC_A;		//0x02 LD (BC),A
	_opcodeTable[0x03] = &CPU::INC_BC;		//0x03 INC BC
	_opcodeTable[0x04] = &CPU::INC_B;		//0x04 INC B
	_opcodeTable[0x05] = &CPU::DEC_B;		//0x05 DEC B
	_opcodeTable[0x06] = &CPU::LD_B_R;		//0x06 LD B,d8
	_opcodeTable[0x07] = &CPU::RL_CA;		//0x07
	_opcodeTable[0x08] = &CPU::LD_RR_SP;	//0x08
	_opcodeTable[0x09] = &CPU::ADD_HL_BC;	//0x09
	_opcodeTable[0x0A] = &CPU::LD_A_BC;		//0x0A
	_opcodeTable[0x0B] = &CPU::DEC_BC;		//0x0B
	_opcodeTable[0x0C] = &CPU::INC_C;		//0x0C
	_opcodeTable[0x0D] = &CPU::DEC_C;		//0x0D
	_opcodeTable[0x0E] = &CPU::LD_C_R;		//0x0E
	_opcodeTable[0x0F] = &CPU::RR_CA;		//0x0F

	_opcodeTable[0x10] = &CPU::STOP;		//0x10
	_opcodeTable[0x11] = &CPU::LD_DE_RR;	//0x11
	_opcodeTable[0x12] = &CPU::LD_DE_A;		//0x12
	_opcodeTable[0x13] = &CPU::INC_DE;		//0x13
	_opcodeTable[0x14] = &CPU::INC_D;		//0x14
	_opcodeTable[0x15] = &CPU::DEC_D;		//0x15
	_opcodeTable[0x16] = &CPU::LD_D_R;		//0x16
	_opcodeTable[0x17] = &CPU::RL_A;		//0x17
	_opcodeTable[0x18] = &CPU::JR_R;		//0x18
	_opcodeTable[0x19] = &CPU::ADD_HL_DE;	//0x19
	_opcodeTable[0x1A] = &CPU::LD_A_DE;		//0x1A
	_opcodeTable[0x1B] = &CPU::DEC_DE;		//0x1B
	_opcodeTable[0x1C] = &CPU::INC_E;		//0x1C
	_opcodeTable[0x1D] = &CPU::DEC_E;		//0x1D
	_opcodeTable[0x1E] = &CPU::LD_E_R;		//0x1E
	_opcodeTable[0x1F] = &CPU::RR_A;		//0x1F

	_opcodeTable[0x20] = &CPU::JR_NZ_R;		//0x20
	_opcodeTable[0x21] = &CPU::LD_HL_RR;	//0x21
	_opcodeTable[0x22] = &CPU::LD_MM_PLUS_A;//0x22
	_opcodeTable[0x23] = &CPU::INC_HL;		//0x23
	_opcodeTable[0x24] = &CPU::INC_H;		//0x24
	_opcodeTable[0x25] = &CPU::DEC_H;		//0x25
	_opcodeTable[0x26] = &CPU::LD_H_R;		//0x26
	_opcodeTable[0x27] = &CPU::DA_A;		//0x27
	_opcodeTable[0x28] = &CPU::JR_Z_R;		//0x28
	_opcodeTable[0x29] = &CPU::ADD_HL_HL;	//0x29
	_opcodeTable[0x2A] = &CPU::LD_A_MM_PLUS;//0x2A
	_opcodeTable[0x2B] = &CPU::DEC_HL;		//0x2B
	_opcodeTable[0x2C] = &CPU::INC_L;		//0x2C
	_opcodeTable[0x2D] = &CPU::DEC_L;		//0x2D
	_opcodeTable[0x2E] = &CPU::LD_L_R;		//0x2E
	_opcodeTable[0x2F] = &CPU::CPL;			//0x2F

	_opcodeTable[0x30] = &CPU::JR_NC_R;		//0x30
	_opcodeTable[0x31] = &CPU::LD_SP_RR;	//0x31
	_opcodeTable[0x32] = &CPU::LD_MM_MIN_A;	//0x32
	_opcodeTable[0x33] = &CPU::INC_SP;		//0x33
	_opcodeTable[0x34] = &CPU::INC_MM;		//0x34
	_opcodeTable[0x35] = &CPU::DEC_MM;		//0x35
	_opcodeTable[0x36] = &CPU::LD_MM_R;		//0x36
	_opcodeTable[0x37] = &CPU::SCF;			//0x37
	_opcodeTable[0x38] = &CPU::JR_C_R;		//0x38
	_opcodeTable[0x39] = &CPU::ADD_HL_SP;	//0x39
	_opcodeTable[0x3A] = &CPU::LD_A_MM_MIN;	//0x3A
	_opcodeTable[0x3B] = &CPU::DEC_SP;		//0x3B
	_opcodeTable[0x3C] = &CPU::INC_A;		//0x3C
	_opcodeTable[0x3D] = &CPU::DEC_A;		//0x3D
	_opcodeTable[0x3E] = &CPU::LD_A_R;		//0x3E
	_opcodeTable[0x3F] = &CPU::CCF;			//0x3F

	_opcodeTable[0x40] = &CPU::LD_B_B;		//0x40
	_opcodeTable[0x41] = &CPU::LD_B_C;		//0x41
	_opcodeTable[0x42] = &CPU::LD_B_D;		//0x42
	_opcodeTable[0x43] = &CPU::LD_B_E;		//0x43
	_opcodeTable[0x44] = &CPU::LD_B_H;		//0x44
	_opcodeTable[0x45] = &CPU::LD_B_L;		//0x45
	_opcodeTable[0x46] = &CPU::LD_B_MM;		//0x46
	_opcodeTable[0x47] = &CPU::LD_B_A;		//0x47
	_opcodeTable[0x48] = &CPU::LD_C_B;		//0x48
	_opcodeTable[0x49] = &CPU::LD_C_C;		//0x49
	_opcodeTable[0x4A] = &CPU::LD_C_D;		//0x4A
	_opcodeTable[0x4B] = &CPU::LD_C_E;		//0x4B
	_opcodeTable[0x4C] = &CPU::LD_C_H;		//0x4C
	_opcodeTable[0x4D] = &CPU::LD_C_L;		//0x4D
	_opcodeTable[0x4E] = &CPU::LD_C_MM;		//0x4E
	_opcodeTable[0x4F] = &CPU::LD_C_A;		//0x4F

	_opcodeTable[0x50] = &CPU::LD_D_B;		//0x50
	_opcodeTable[0x51] = &CPU::LD_D_C;		//0x51
	_opcodeTable[0x52] = &CPU::LD_D_D;		//0x52
	_opcodeTable[0x53] = &CPU::LD_D_E;		//0x53
	_opcodeTable[0x54] = &CPU::LD_D_H;		//0x54
	_opcodeTable[0x55] = &CPU::LD_D_L;		//0x55
	_opcodeTable[0x56] = &CPU::LD_D_MM;		//0x56
	_opcodeTable[0x57] = &CPU::LD_D_A;		//0x57
	_opcodeTable[0x58] = &CPU::LD_E_B;		//0x58
	_opcodeTable[0x59] = &CPU::LD_E_C;		//0x59
	_opcodeTable[0x5A] = &CPU::LD_E_D;		//0x5A
	_opcodeTable[0x5B] = &CPU::LD_E_E;		//0x5B
	_opcodeTable[0x5C] = &CPU::LD_E_H;		//0x5C
	_opcodeTable[0x5D] = &CPU::LD_E_L;		//0x5D
	_opcodeTable[0x5E] = &CPU::LD_E_MM;		//0x5E
	_opcodeTable[0x5F] = &CPU::LD_E_A;		//0x5F

	_opcodeTable[0x60] = &CPU::LD_H_B;		//0x60
	_opcodeTable[0x61] = &CPU::LD_H_C;		//0x61
	_opcodeTable[0x62] = &CPU::LD_H_D;		//0x62
	_opcodeTable[0x63] = &CPU::LD_H_E;		//0x63
	_opcodeTable[0x64] = &CPU::LD_H_H;		//0x64
	_opcodeTable[0x65] = &CPU::LD_H_L;		//0x65
	_opcodeTable[0x66] = &CPU::LD_H_MM;		//0x66
	_opcodeTable[0x67] = &CPU::LD_H_A;		//0x67
	_opcodeTable[0x68] = &CPU::LD_L_B;		//0x68
	_opcodeTable[0x69] = &CPU::LD_L_C;		//0x69
	_opcodeTable[0x6A] = &CPU::LD_L_D;		//0x6A
	_opcodeTable[0x6B] = &CPU::LD_L_E;		//0x6B
	_opcodeTable[0x6C] = &CPU::LD_L_H;		//0x6C
	_opcodeTable[0x6D] = &CPU::LD_L_L;		//0x6D
	_opcodeTable[0x6E] = &CPU::LD_L_MM;		//0x6E
	_opcodeTable[0x6F] = &CPU::LD_L_A;		//0x6F

	_opcodeTable[0x70] = &CPU::LD_MM_B;		//0x70
	_opcodeTable[0x71] = &CPU::LD_MM_C;		//0x71
	_opcodeTable[0x72] = &CPU::LD_MM_D;		//0x72
	_opcodeTable[0x73] = &CPU::LD_MM_E;		//0x73
	_opcodeTable[0x74] = &CPU::LD_MM_H;		//0x74
	_opcodeTable[0x75] = &CPU::LD_MM_L;		//0x75
	_opcodeTable[0x76] = &CPU::HALT;		//0x76
	_opcodeTable[0x77] = &CPU::LD_MM_A;		//0x77
	_opcodeTable[0x78] = &CPU::LD_A_B;		//0x78
	_opcodeTable[0x79] = &CPU::LD_A_C;		//0x79
	_opcodeTable[0x7A] = &CPU::LD_A_D;		//0x7A
	_opcodeTable[0x7B] = &CPU::LD_A_E;		//0x7B
	_opcodeTable[0x7C] = &CPU::LD_A_H;		//0x7C
	_opcodeTable[0x7D] = &CPU::LD_A_L;		//0x7D
	_opcodeTable[0x7E] = &CPU::LD_A_MM;		//0x7E
	_opcodeTable[0x7F] = &CPU::LD_A_A;		//0x7F


	_opcodeTable[0x80] = &CPU::ADD_A_B;		//0x80
	_opcodeTable[0x81] = &CPU::ADD_A_C;		//0x81
	_opcodeTable[0x82] = &CPU::ADD_A_D;		//0x82
	_opcodeTable[0x83] = &CPU::ADD_A_E;		//0x83
	_opcodeTable[0x84] = &CPU::ADD_A_H;		//0x84
	_opcodeTable[0x85] = &CPU::ADD_A_L;		//0x85
	_opcodeTable[0x86] = &CPU::ADD_A_MM;	//0x86
	_opcodeTable[0x87] = &CPU::ADD_A_A;		//0x87
	_opcodeTable[0x88] = &CPU::ADC_A_B;		//0x88
	_opcodeTable[0x89] = &CPU::ADC_A_C;		//0x89
	_opcodeTable[0x8A] = &CPU::ADC_A_D;		//0x8A
	_opcodeTable[0x8B] = &CPU::ADC_A_E;		//0x8B
	_opcodeTable[0x8C] = &CPU::ADC_A_H;		//0x8C
	_opcodeTable[0x8D] = &CPU::ADC_A_L;		//0x8D
	_opcodeTable[0x8E] = &CPU::ADC_A_MM;	//0x8E
	_opcodeTable[0x8F] = &CPU::ADC_A_A;		//0x8F

	_opcodeTable[0x90] = &CPU::SUB_A_B;		//0x90
	_opcodeTable[0x91] = &CPU::SUB_A_C;		//0x91
	_opcodeTable[0x92] = &CPU::SUB_A_D;		//0x92
	_opcodeTable[0x93] = &CPU::SUB_A_E;		//0x93
	_opcodeTable[0x94] = &CPU::SUB_A_H;		//0x94
	_opcodeTable[0x95] = &CPU::SUB_A_L;		//0x95
	_opcodeTable[0x96] = &CPU::SUB_A_MM;	//0x96
	_opcodeTable[0x97] = &CPU::SUB_A_A;		//0x97
	_opcodeTable[0x98] = &CPU::SBC_A_B;		//0x98
	_opcodeTable[0x99] = &CPU::SBC_A_C;		//0x99
	_opcodeTable[0x9A] = &CPU::SBC_A_D;		//0x9A
	_opcodeTable[0x9B] = &CPU::SBC_A_E;		//0x9B
	_opcodeTable[0x9C] = &CPU::SBC_A_H;		//0x9C
	_opcodeTable[0x9D] = &CPU::SBC_A_L;		//0x9D
	_opcodeTable[0x9E] = &CPU::SBC_A_MM;	//0x9E
	_opcodeTable[0x9F] = &CPU::SBC_A_A;		//0x9F

	_opcodeTable[0xA0] = &CPU::AND_B;		//0xA0
	_opcodeTable[0xA1] = &CPU::AND_C;		//0xA1
	_opcodeTable[0xA2] = &CPU::AND_D;		//0xA2
	_opcodeTable[0xA3] = &CPU::AND_E;		//0xA3
	_opcodeTable[0xA4] = &CPU::AND_H;		//0xA4
	_opcodeTable[0xA5] = &CPU::AND_L;		//0xA5
	_opcodeTable[0xA6] = &CPU::AND_MM;		//0xA6
	_opcodeTable[0xA7] = &CPU::AND_A;		//0xA7
	_opcodeTable[0xA8] = &CPU::XOR_B;		//0xA8
	_opcodeTable[0xA9] = &CPU::XOR_C;		//0xA9
	_opcodeTable[0xAA] = &CPU::XOR_D;		//0xAA
	_opcodeTable[0xAB] = &CPU::XOR_E;		//0xAB
	_opcodeTable[0xAC] = &CPU::XOR_H;		//0xAC
	_opcodeTable[0xAD] = &CPU::XOR_L;		//0xAD
	_opcodeTable[0xAE] = &CPU::XOR_MM;		//0xAE
	_opcodeTable[0xAF] = &CPU::XOR_A;		//0xAF

	_opcodeTable[0xB0] = &CPU::OR_B;		//0xB0
	_opcodeTable[0xB1] = &CPU::OR_C;		//0xB1
	_opcodeTable[0xB2] = &CPU::OR_D;		//0xB2
	_opcodeTable[0xB3] = &CPU::OR_E;		//0xB3
	_opcodeTable[0xB4] = &CPU::OR_H;		//0xB4
	_opcodeTable[0xB5] = &CPU::OR_L;		//0xB5
	_opcodeTable[0xB6] = &CPU::OR_MM;		//0xB6
	_opcodeTable[0xB7] = &CPU::OR_A;		//0xB7
	_opcodeTable[0xB8] = &CPU::CP_B;		//0xB8
	_opcodeTable[0xB9] = &CPU::CP_C;		//0xB9
	_opcodeTable[0xBA] = &CPU::CP_D;		//0xBA
	_opcodeTable[0xBB] = &CPU::CP_E;		//0xBB
	_opcodeTable[0xBC] = &CPU::CP_H;		//0xBC
	_opcodeTable[0xBD] = &CPU::CP_L;		//0xBD
	_opcodeTable[0xBE] = &CPU::CP_MM;		//0xBE
	_opcodeTable[0xBF] = &CPU::CP_A;		//0xBF

	_opcodeTable[0xC0] = &CPU::RET_NZ;		//0xC0
	_opcodeTable[0xC1] = &CPU::POP_BC;		//0xC1
	_opcodeTable[0xC2] = &CPU::JP_NZ_RR;	//0xC2
	_opcodeTable[0xC3] = &CPU::JP_RR;		//0xC3
	_opcodeTable[0xC4] = &CPU::CALL_NZ_RR;	//0xC4
	_opcodeTable[0xC5] = &CPU::PUSH_BC;		//0xC5
	_opcodeTable[0xC6] = &CPU::ADD_A_R;		//0xC6
	_opcodeTable[0xC7] = &CPU::RST_00H;		//0xC7
	_opcodeTable[0xC8] = &CPU::RET_Z;		//0xC8
	_opcodeTable[0xC9] = &CPU::RET;			//0xC9
	_opcodeTable[0xCA] = &CPU::JP_Z_RR;		//0xCA
	_opcodeTable[0xCB] = &CPU::PREFIX_CB;	//0xCB
	_opcodeTable[0xCC] = &CPU::CALL_Z_RR;	//0xCC
	_opcodeTable[0xCD] = &CPU::CALL_RR;		//0xCD
	_opcodeTable[0xCE] = &CPU::ADC_A_R;		//0xCE
	_opcodeTable[0xCF] = &CPU::RST_08H;		//0xCF

	_opcodeTable[0xD0] = &CPU::RET_NC;		//0xD0
	_opcodeTable[0xD1] = &CPU::POP_DE;		//0xD1
	_opcodeTable[0xD2] = &CPU::JP_NC_RR;	//0xD2
	_opcodeTable[0xD4] = &CPU::CALL_NC_RR;	//0xD4
	_opcodeTable[0xD5] = &CPU::PUSH_DE;		//0xD5
	_opcodeTable[0xD6] = &CPU::SUB_R;		//0xD6
	_opcodeTable[0xD7] = &CPU::RST_10H;		//0xD7
	_opcodeTable[0xD8] = &CPU::RET_C;		//0xD8
	_opcodeTable[0xD9] = &CPU::RETI;		//0xD9
	_opcodeTable[0xDA] = &CPU::JP_C_RR;		//0xDA
	_opcodeTable[0xDC] = &CPU::CALL_C_RR;	//0xDC
	_opcodeTable[0xDE] = &CPU::SBC_A_R;		//0xDE
	_opcodeTable[0xDF] = &CPU::RST_18H;		//0xDF

	_opcodeTable[0xE0] = &CPU::LDH_R_A;		//0xE0
	_opcodeTable[0xE1] = &CPU::POP_HL;		//0xE1
	_opcodeTable[0xE2] = &CPU::LD_C_A2;		//0xE2
	_opcodeTable[0xE5] = &CPU::PUSH_HL;		//0xE5
	_opcodeTable[0xE6] = &CPU::AND_R;		//0xE6
	_opcodeTable[0xE7] = &CPU::RST_20H;		//0xE7
	_opcodeTable[0xE8] = &CPU::ADD_SP_R;	//0xE8
	_opcodeTable[0xE9] = &CPU::JP_MM;		//0xE9
	_opcodeTable[0xEA] = &CPU::LD_RR_A;		//0xEA
	_opcodeTable[0xEE] = &CPU::XOR_R;		//0xEE
	_opcodeTable[0xEF] = &CPU::RST_28H;		//0xEF

	_opcodeTable[0xF0] = &CPU::LDH_A_R;		//0xF0
	_opcodeTable[0xF1] = &CPU::POP_AF;		//0xF1
	_opcodeTable[0xF2] = &CPU::LD_A_C2;		//0xF2
	_opcodeTable[0xF3] = &CPU::DI;			//0xF3
	_opcodeTable[0xF5] = &CPU::PUSH_AF;		//0xF5
	_opcodeTable[0xF6] = &CPU::OR_R;		//0xF6
	_opcodeTable[0xF7] = &CPU::RST_30H;		//0xF7
	_opcodeTable[0xF8] = &CPU::LD_HL_SPandR;//0xF8
	_opcodeTable[0xF9] = &CPU::LD_SP_HL;	//0xF9
	_opcodeTable[0xFA] = &CPU::LD_A_RR;		//0xFA
	_opcodeTable[0xFB] = &CPU::EI;			//0xFB
	_opcodeTable[0xFE] = &CPU::CP_R;		//0xFE
	_opcodeTable[0xFF] = &CPU::RST_38H;		//0xFF

	_cbOpcodeTable[0x00] = &CPU::CB_RLC_B;	//0xCB00
	_cbOpcodeTable[0x01] = &CPU::CB_RLC_C;	//0xCB01
	_cbOpcodeTable[0x02] = &CPU::CB_RLC_D;	//0xCB02
	_cbOpcodeTable[0x03] = &CPU::CB_RLC_E;	//0xCB03
	_cbOpcodeTable[0x04] = &CPU::CB_RLC_H;	//0xCB04
	_cbOpcodeTable[0x05] = &CPU::CB_RLC_L;	//0xCB05
	_cbOpcodeTable[0x06] = &CPU::CB_RLC_MM;	//0xCB06
	_cbOpcodeTable[0x07] = &CPU::CB_RLC_A;	//0xCB07
	_cbOpcodeTable[0x08] = &CPU::CB_RRC_B;	//0xCB08
	_cbOpcodeTable[0x09] = &CPU::CB_RRC_C;	//0xCB09
	_cbOpcodeTable[0x0A] = &CPU::CB_RRC_D;	//0xCB0A
	_cbOpcodeTable[0x0B] = &CPU::CB_RRC_E;	//0xCB0B
	_cbOpcodeTable[0x0C] = &CPU::CB_RRC_H;	//0xCB0C
	_cbOpcodeTable[0x0D] = &CPU::CB_RRC_L;	//0xCB0D
	_cbOpcodeTable[0x0E] = &CPU::CB_RRC_MM;	//0xCB0E
	_cbOpcodeTable[0x0F] = &CPU::CB_RRC_A;	//0xCB0F

	_cbOpcodeTable[0x10] = &CPU::CB_RL_B;		//0xCB10
	_cbOpcodeTable[0x11] = &CPU::CB_RL_C;		//0xCB11
	_cbOpcodeTable[0x12] = &CPU::CB_RL_D;		//0xCB12
	_cbOpcodeTable[0x13] = &CPU::CB_RL_E;		//0xCB13
	_cbOpcodeTable[0x14] = &CPU::CB_RL_H;		//0xCB14
	_cbOpcodeTable[0x15] = &CPU::CB_RL_L;		//0xCB15
	_cbOpcodeTable[0x16] = &CPU::CB_RL_MM;		//0xCB16
	_cbOpcodeTable[0x17] = &CPU::CB_RL_A;		//0xCB17
	_cbOpcodeTable[0x18] = &CPU::CB_RR_B;		//0xCB18
	_cbOpcodeTable[0x19] = &CPU::CB_RR_C;		//0xCB19
	_cbOpcodeTable[0x1A] = &CPU::CB_RR_D;		//0xCB1A
	_cbOpcodeTable[0x1B] = &CPU::CB_RR_E;		//0xCB1B
	_cbOpcodeTable[0x1C] = &CPU::CB_RR_H;		//0xCB1C
	_cbOpcodeTable[0x1D] = &CPU::CB_RR_L;		//0xCB1D
	_cbOpcodeTable[0x1E] = &CPU::CB_RR_MM;		//0xCB1E
	_cbOpcodeTable[0x1F] = &CPU::CB_RR_A;		//0xCB1F

	_cbOpcodeTable[0x20] = &CPU::CB_SLA_B;	//0xCB20
	_cbOpcodeTable[0x21] = &CPU::CB_SLA_C;	//0xCB21
	_cbOpcodeTable[0x22] = &CPU::CB_SLA_D;	//0xCB22
	_cbOpcodeTable[0x23] = &CPU::CB_SLA_E;	//0xCB23
	_cbOpcodeTable[0x24] = &CPU::CB_SLA_H;	//0xCB24
	_cbOpcodeTable[0x25] = &CPU::CB_SLA_L;	//0xCB25
	_cbOpcodeTable[0x26] = &CPU::CB_SLA_MM;	//0xCB26
	_cbOpcodeTable[0x27] = &CPU::CB_SLA_A;	//0xCB27
	_cbOpcodeTable[0x28] = &CPU::CB_SRA_B;	//0xCB28
	_cbOpcodeTable[0x29] = &CPU::CB_SRA_C;	//0xCB29
	_cbOpcodeTable[0x2A] = &CPU::CB_SRA_D;	//0xCB2A
	_cbOpcodeTable[0x2B] = &CPU::CB_SRA_E;	//0xCB2B
	_cbOpcodeTable[0x2C] = &CPU::CB_SRA_H;	//0xCB2C
	_cbOpcodeTable[0x2D] = &CPU::CB_SRA_L;	//0xCB2D
	_cbOpcodeTable[0x2E] = &CPU::CB_SRA_MM;		//0x2E
	_cbOpcodeTable[0x2F] = &CPU::CB_SRA_A;			//0x2F

	_cbOpcodeTable[0x30] = &CPU::CB_SWAP_B;		//0x30
	_cbOpcodeTable[0x31] = &CPU::CB_SWAP_C;	//0x31
	_cbOpcodeTable[0x32] = &CPU::CB_SWAP_D;	//0x32
	_cbOpcodeTable[0x33] = &CPU::CB_SWAP_E;		//0x33
	_cbOpcodeTable[0x34] = &CPU::CB_SWAP_H;		//0x34
	_cbOpcodeTable[0x35] = &CPU::CB_SWAP_L;		//0x35
	_cbOpcodeTable[0x36] = &CPU::CB_SWAP_MM;		//0x36
	_cbOpcodeTable[0x37] = &CPU::CB_SWAP_A;			//0x37
	_cbOpcodeTable[0x38] = &CPU::CB_SRL_B;		//0x38
	_cbOpcodeTable[0x39] = &CPU::CB_SRL_C;		//0x38
	_cbOpcodeTable[0x3A] = &CPU::CB_SRL_D;	//0x39
	_cbOpcodeTable[0x3B] = &CPU::CB_SRL_E;	//0x3A
	_cbOpcodeTable[0x3C] = &CPU::CB_SRL_H;		//0x3B
	_cbOpcodeTable[0x3D] = &CPU::CB_SRL_L;		//0x3C
	_cbOpcodeTable[0x3E] = &CPU::CB_SRL_MM;		//0x3D
	_cbOpcodeTable[0x3F] = &CPU::CB_SRL_A;		//0x3E

	_cbOpcodeTable[0x40] = &CPU::CB_BIT_0_B;	//0XCB40
	_cbOpcodeTable[0x41] = &CPU::CB_BIT_0_C;	//0XCB41
	_cbOpcodeTable[0x42] = &CPU::CB_BIT_0_D;	//0XCB42
	_cbOpcodeTable[0x43] = &CPU::CB_BIT_0_E;	//0XCB43
	_cbOpcodeTable[0x44] = &CPU::CB_BIT_0_H;	//0XCB44
	_cbOpcodeTable[0x45] = &CPU::CB_BIT_0_L;	//0XCB45
	_cbOpcodeTable[0x46] = &CPU::CB_BIT_0_MM;	//0XCB46
	_cbOpcodeTable[0x47] = &CPU::CB_BIT_0_A;	//0XCB47
	_cbOpcodeTable[0x48] = &CPU::CB_BIT_1_B;	//0XCB48
	_cbOpcodeTable[0x49] = &CPU::CB_BIT_1_C;	//0XCB49
	_cbOpcodeTable[0x4A] = &CPU::CB_BIT_1_D;	//0XCB4A
	_cbOpcodeTable[0x4B] = &CPU::CB_BIT_1_E;	//0XCB4B
	_cbOpcodeTable[0x4C] = &CPU::CB_BIT_1_H;	//0XCB4C
	_cbOpcodeTable[0x4D] = &CPU::CB_BIT_1_L;	//0XCB4D
	_cbOpcodeTable[0x4E] = &CPU::CB_BIT_1_MM;	//0XCB4E
	_cbOpcodeTable[0x4F] = &CPU::CB_BIT_1_A;	//0XCB4F

	_cbOpcodeTable[0x50] = &CPU::CB_BIT_2_B;	//0XCB50
	_cbOpcodeTable[0x51] = &CPU::CB_BIT_2_C;	//0XCB51
	_cbOpcodeTable[0x52] = &CPU::CB_BIT_2_D;	//0XCB52
	_cbOpcodeTable[0x53] = &CPU::CB_BIT_2_E;	//0XCB53
	_cbOpcodeTable[0x54] = &CPU::CB_BIT_2_H;	//0XCB54
	_cbOpcodeTable[0x55] = &CPU::CB_BIT_2_L;	//0XCB55
	_cbOpcodeTable[0x56] = &CPU::CB_BIT_2_MM;	//0XCB56
	_cbOpcodeTable[0x57] = &CPU::CB_BIT_2_A;	//0XCB57
	_cbOpcodeTable[0x58] = &CPU::CB_BIT_3_B;	//0XCB58
	_cbOpcodeTable[0x59] = &CPU::CB_BIT_3_C;	//0XCB59
	_cbOpcodeTable[0x5A] = &CPU::CB_BIT_3_D;	//0XCB5A
	_cbOpcodeTable[0x5B] = &CPU::CB_BIT_3_E;	//0XCB5B
	_cbOpcodeTable[0x5C] = &CPU::CB_BIT_3_H;	//0XCB5C
	_cbOpcodeTable[0x5D] = &CPU::CB_BIT_3_L;	//0XCB5D
	_cbOpcodeTable[0x5E] = &CPU::CB_BIT_3_MM;	//0XCB5E
	_cbOpcodeTable[0x5F] = &CPU::CB_BIT_3_A;	//0XCB5F

	_cbOpcodeTable[0x60] = &CPU::CB_BIT_4_B;	//0XCB60
	_cbOpcodeTable[0x61] = &CPU::CB_BIT_4_C;	//0XCB61
	_cbOpcodeTable[0x62] = &CPU::CB_BIT_4_D;	//0XCB62
	_cbOpcodeTable[0x63] = &CPU::CB_BIT_4_E;	//0XCB63
	_cbOpcodeTable[0x64] = &CPU::CB_BIT_4_H;	//0XCB64
	_cbOpcodeTable[0x65] = &CPU::CB_BIT_4_L;	//0XCB65
	_cbOpcodeTable[0x66] = &CPU::CB_BIT_4_MM;	//0XCB66
	_cbOpcodeTable[0x67] = &CPU::CB_BIT_4_A;	//0XCB67
	_cbOpcodeTable[0x68] = &CPU::CB_BIT_5_B;	//0XCB68
	_cbOpcodeTable[0x69] = &CPU::CB_BIT_5_C;	//0XCB69
	_cbOpcodeTable[0x6A] = &CPU::CB_BIT_5_D;	//0XCB6A
	_cbOpcodeTable[0x6B] = &CPU::CB_BIT_5_E;	//0XCB6B
	_cbOpcodeTable[0x6C] = &CPU::CB_BIT_5_H;	//0XCB6C
	_cbOpcodeTable[0x6D] = &CPU::CB_BIT_5_L;	//0XCB6D
	_cbOpcodeTable[0x6E] = &CPU::CB_BIT_5_MM;	//0XCB6E
	_cbOpcodeTable[0x6F] = &CPU::CB_BIT_5_A;	//0XCB6F

	_cbOpcodeTable[0x70] = &CPU::CB_BIT_6_B;	//0XCB70
	_cbOpcodeTable[0x71] = &CPU::CB_BIT_6_C;	//0XCB71
	_cbOpcodeTable[0x72] = &CPU::CB_BIT_6_D;	//0XCB72
	_cbOpcodeTable[0x73] = &CPU::CB_BIT_6_E;	//0XCB73
	_cbOpcodeTable[0x74] = &CPU::CB_BIT_6_H;	//0XCB74
	_cbOpcodeTable[0x75] = &CPU::CB_BIT_6_L;	//0XCB75
	_cbOpcodeTable[0x76] = &CPU::CB_BIT_6_MM;	//0XCB76
	_cbOpcodeTable[0x77] = &CPU::CB_BIT_6_A;	//0XCB77
	_cbOpcodeTable[0x78] = &CPU::CB_BIT_7_B;	//0XCB78
	_cbOpcodeTable[0x79] = &CPU::CB_BIT_7_C;	//0XCB79
	_cbOpcodeTable[0x7A] = &CPU::CB_BIT_7_D;	//0XCB7A
	_cbOpcodeTable[0x7B] = &CPU::CB_BIT_7_E;	//0XCB7B
	_cbOpcodeTable[0x7C] = &CPU::CB_BIT_7_H;	//0XCB7C
	_cbOpcodeTable[0x7D] = &CPU::CB_BIT_7_L;	//0XCB7D
	_cbOpcodeTable[0x7E] = &CPU::CB_BIT_7_MM;	//0XCB7E
	_cbOpcodeTable[0x7F] = &CPU::CB_BIT_7_A;	//0XCB7F

	_cbOpcodeTable[0x80] = &CPU::CB_RES_0_B;	//0XCB80
	_cbOpcodeTable[0x81] = &CPU::CB_RES_0_C;	//0XCB81
	_cbOpcodeTable[0x82] = &CPU::CB_RES_0_D;	//0XCB82
	_cbOpcodeTable[0x83] = &CPU::CB_RES_0_E;	//0XCB83
	_cbOpcodeTable[0x84] = &CPU::CB_RES_0_H;	//0XCB84
	_cbOpcodeTable[0x85] = &CPU::CB_RES_0_L;	//0XCB85
	_cbOpcodeTable[0x86] = &CPU::CB_RES_0_MM;	//0XCB86
	_cbOpcodeTable[0x87] = &CPU::CB_RES_0_A;	//0XCB87
	_cbOpcodeTable[0x88] = &CPU::CB_RES_1_B;	//0XCB98
	_cbOpcodeTable[0x89] = &CPU::CB_RES_1_C;	//0XCB99
	_cbOpcodeTable[0x8A] = &CPU::CB_RES_1_D;	//0XCB9a
	_cbOpcodeTable[0x8B] = &CPU::CB_RES_1_E;	//0XCB9b
	_cbOpcodeTable[0x8C] = &CPU::CB_RES_1_H;	//0XCB9c
	_cbOpcodeTable[0x8D] = &CPU::CB_RES_1_L;	//0XCB9d
	_cbOpcodeTable[0x8E] = &CPU::CB_RES_1_MM;	//0XCB9e
	_cbOpcodeTable[0x8F] = &CPU::CB_RES_1_A;	//0XCB9f

	_cbOpcodeTable[0x91] = &CPU::CB_RES_2_B;	//0XCB90
	_cbOpcodeTable[0x92] = &CPU::CB_RES_2_C;	//0XCB91
	_cbOpcodeTable[0x93] = &CPU::CB_RES_2_D;	//0XCB92
	_cbOpcodeTable[0x93] = &CPU::CB_RES_2_E;	//0XCB93
	_cbOpcodeTable[0x94] = &CPU::CB_RES_2_H;	//0XCB94
	_cbOpcodeTable[0x95] = &CPU::CB_RES_2_L;	//0XCB95
	_cbOpcodeTable[0x96] = &CPU::CB_RES_2_MM;	//0XCB96
	_cbOpcodeTable[0x97] = &CPU::CB_RES_2_A;	//0XCB97
	_cbOpcodeTable[0x98] = &CPU::CB_RES_3_B;	//0XCB98
	_cbOpcodeTable[0x99] = &CPU::CB_RES_3_C;	//0XCB99
	_cbOpcodeTable[0x9A] = &CPU::CB_RES_3_D;	//0XCB9A
	_cbOpcodeTable[0x9B] = &CPU::CB_RES_3_E;	//0XCB9B
	_cbOpcodeTable[0x9C] = &CPU::CB_RES_3_H;	//0XCB9C
	_cbOpcodeTable[0x9D] = &CPU::CB_RES_3_L;	//0XCB9D
	_cbOpcodeTable[0x9E] = &CPU::CB_RES_3_MM;	//0XCB9E
	_cbOpcodeTable[0x9F] = &CPU::CB_RES_3_A;	//0XCB9F

	_cbOpcodeTable[0xA0] = &CPU::CB_RES_4_B;	//0XCBA0
	_cbOpcodeTable[0xA1] = &CPU::CB_RES_4_C;	//0XCBA1
	_cbOpcodeTable[0xA2] = &CPU::CB_RES_4_D;	//0XCBA2
	_cbOpcodeTable[0xA3] = &CPU::CB_RES_4_E;	//0XCBA3
	_cbOpcodeTable[0xA4] = &CPU::CB_RES_4_H;	//0XCBA4
	_cbOpcodeTable[0xA5] = &CPU::CB_RES_4_L;	//0XCBA5
	_cbOpcodeTable[0xA6] = &CPU::CB_RES_4_MM;	//0XCBA6
	_cbOpcodeTable[0xA7] = &CPU::CB_RES_4_A;	//0XCBA7
	_cbOpcodeTable[0xA8] = &CPU::CB_RES_5_B;	//0XCBA8
	_cbOpcodeTable[0xA9] = &CPU::CB_RES_5_C;	//0XCBA9
	_cbOpcodeTable[0xAA] = &CPU::CB_RES_5_D;	//0XCBAA
	_cbOpcodeTable[0xAB] = &CPU::CB_RES_5_E;	//0XCBAB
	_cbOpcodeTable[0xAC] = &CPU::CB_RES_5_H;	//0XCBAC
	_cbOpcodeTable[0xAD] = &CPU::CB_RES_5_L;	//0XCBAD
	_cbOpcodeTable[0xAE] = &CPU::CB_RES_5_MM;	//0XCBAE
	_cbOpcodeTable[0xAF] = &CPU::CB_RES_5_A;	//0XCBAF

	_cbOpcodeTable[0xB0] = &CPU::CB_RES_6_B;	//0XCBB0
	_cbOpcodeTable[0xB1] = &CPU::CB_RES_6_C;	//0XCBB1
	_cbOpcodeTable[0xB2] = &CPU::CB_RES_6_D;	//0XCBB2
	_cbOpcodeTable[0xB3] = &CPU::CB_RES_6_E;	//0XCBB3
	_cbOpcodeTable[0xB4] = &CPU::CB_RES_6_H;	//0XCBB4
	_cbOpcodeTable[0xB5] = &CPU::CB_RES_6_L;	//0XCBB5
	_cbOpcodeTable[0xB6] = &CPU::CB_RES_6_MM;	//0XCBB6
	_cbOpcodeTable[0xB7] = &CPU::CB_RES_6_A;	//0XCBB7
	_cbOpcodeTable[0xB8] = &CPU::CB_RES_7_B;	//0XCBB8
	_cbOpcodeTable[0xB9] = &CPU::CB_RES_7_C;	//0XCBB9
	_cbOpcodeTable[0xBA] = &CPU::CB_RES_7_D;	//0XCBBA
	_cbOpcodeTable[0xBB] = &CPU::CB_RES_7_E;	//0XCBBB
	_cbOpcodeTable[0xBC] = &CPU::CB_RES_7_H;	//0XCBBC
	_cbOpcodeTable[0xBD] = &CPU::CB_RES_7_L;	//0XCBBD
	_cbOpcodeTable[0xBE] = &CPU::CB_RES_7_MM;	//0XCBBE
	_cbOpcodeTable[0xBF] = &CPU::CB_RES_7_A;	//0XCBBF

	_cbOpcodeTable[0xC0] = &CPU::CB_SET_0_B;	//0XCBC0
	_cbOpcodeTable[0xC1] = &CPU::CB_SET_0_C;	//0XCBC1
	_cbOpcodeTable[0xC2] = &CPU::CB_SET_0_D;	//0XCBC2
	_cbOpcodeTable[0xC3] = &CPU::CB_SET_0_E;	//0XCBC3
	_cbOpcodeTable[0xC4] = &CPU::CB_SET_0_H;	//0XCBC4
	_cbOpcodeTable[0xC5] = &CPU::CB_SET_0_L;	//0XCBC5
	_cbOpcodeTable[0xC6] = &CPU::CB_SET_0_MM;	//0XCBC6
	_cbOpcodeTable[0xC7] = &CPU::CB_SET_0_A;	//0XCBC7
	_cbOpcodeTable[0xC8] = &CPU::CB_SET_1_B;	//0XCBC8
	_cbOpcodeTable[0xC9] = &CPU::CB_SET_1_C;	//0XCBC9
	_cbOpcodeTable[0xCA] = &CPU::CB_SET_1_D;	//0XCBCA
	_cbOpcodeTable[0xCB] = &CPU::CB_SET_1_E;	//0XCBCB
	_cbOpcodeTable[0xCC] = &CPU::CB_SET_1_H;	//0XCBCC
	_cbOpcodeTable[0xCD] = &CPU::CB_SET_1_L;	//0XCBCD
	_cbOpcodeTable[0xCE] = &CPU::CB_SET_1_MM;	//0XCBCE
	_cbOpcodeTable[0xCF] = &CPU::CB_SET_1_A;	//0XCBCF

	_cbOpcodeTable[0xD0] = &CPU::CB_SET_2_B;	//0XCBD0
	_cbOpcodeTable[0xD1] = &CPU::CB_SET_2_C;	//0XCBD1
	_cbOpcodeTable[0xD2] = &CPU::CB_SET_2_D;	//0XCBD2
	_cbOpcodeTable[0xD3] = &CPU::CB_SET_2_E;	//0XCBD3
	_cbOpcodeTable[0xD4] = &CPU::CB_SET_2_H;	//0XCBD4
	_cbOpcodeTable[0xD5] = &CPU::CB_SET_2_L;	//0XCBD5
	_cbOpcodeTable[0xD6] = &CPU::CB_SET_2_MM;	//0XCBD5
	_cbOpcodeTable[0xD7] = &CPU::CB_SET_2_A;	//0XCBD7
	_cbOpcodeTable[0xD8] = &CPU::CB_SET_3_B;	//0XCBD8
	_cbOpcodeTable[0xD9] = &CPU::CB_SET_3_C;	//0XCBD9
	_cbOpcodeTable[0xDA] = &CPU::CB_SET_3_D;	//0XCBDA
	_cbOpcodeTable[0xDB] = &CPU::CB_SET_3_E;	//0XCBDB
	_cbOpcodeTable[0xDC] = &CPU::CB_SET_3_H;	//0XCBDC
	_cbOpcodeTable[0xDD] = &CPU::CB_SET_3_L;	//0XCBDD
	_cbOpcodeTable[0xDE] = &CPU::CB_SET_3_MM;	//0XCBDE
	_cbOpcodeTable[0xDF] = &CPU::CB_SET_3_A;	//0XCBDF

	_cbOpcodeTable[0xE0] = &CPU::CB_SET_4_B;	//0XCBE0
	_cbOpcodeTable[0xE1] = &CPU::CB_SET_4_C;	//0XCBE1
	_cbOpcodeTable[0xE2] = &CPU::CB_SET_4_D;	//0XCBE2
	_cbOpcodeTable[0xE3] = &CPU::CB_SET_4_E;	//0XCBE3
	_cbOpcodeTable[0xE4] = &CPU::CB_SET_4_H;	//0XCBE4
	_cbOpcodeTable[0xE5] = &CPU::CB_SET_4_L;	//0XCBE5
	_cbOpcodeTable[0xE6] = &CPU::CB_SET_4_MM;	//0XCBE6
	_cbOpcodeTable[0xE7] = &CPU::CB_SET_4_A;	//0XCBE7
	_cbOpcodeTable[0xE8] = &CPU::CB_SET_5_B;	//0XCBE8
	_cbOpcodeTable[0xE9] = &CPU::CB_SET_5_C;	//0XCBE9
	_cbOpcodeTable[0xEA] = &CPU::CB_SET_5_D;	//0XCBEA
	_cbOpcodeTable[0xEB] = &CPU::CB_SET_5_E;	//0XCBEB
	_cbOpcodeTable[0xEC] = &CPU::CB_SET_5_H;	//0XCBEC
	_cbOpcodeTable[0xED] = &CPU::CB_SET_5_L;	//0XCBED
	_cbOpcodeTable[0xEE] = &CPU::CB_SET_5_MM;	//0XCBEE
	_cbOpcodeTable[0xEF] = &CPU::CB_SET_5_A;	//0XCBEF

	_cbOpcodeTable[0xF0] = &CPU::CB_SET_6_B;	//0XCBF0
	_cbOpcodeTable[0xF1] = &CPU::CB_SET_6_C;	//0XCBF1
	_cbOpcodeTable[0xF2] = &CPU::CB_SET_6_D;	//0XCBF2
	_cbOpcodeTable[0xF3] = &CPU::CB_SET_6_E;	//0XCBF3
	_cbOpcodeTable[0xF4] = &CPU::CB_SET_6_H;	//0XCBF4
	_cbOpcodeTable[0xF5] = &CPU::CB_SET_6_L;	//0XCBF5
	_cbOpcodeTable[0xF6] = &CPU::CB_SET_6_MM;	//0XCBF6
	_cbOpcodeTable[0xF7] = &CPU::CB_SET_6_A;	//0XCBF7
	_cbOpcodeTable[0xF8] = &CPU::CB_SET_7_B;	//0XCBF8
	_cbOpcodeTable[0xF9] = &CPU::CB_SET_7_C;	//0XCBF9
	_cbOpcodeTable[0xFA] = &CPU::CB_SET_7_D;	//0XCBFA
	_cbOpcodeTable[0xFB] = &CPU::CB_SET_7_E;	//0XCBFB
	_cbOpcodeTable[0xFC] = &CPU::CB_SET_7_H;	//0XCBFC
	_cbOpcodeTable[0xFD] = &CPU::CB_SET_7_L;	//0XCBFD
	_cbOpcodeTable[0xFE] = &CPU::CB_SET_7_MM;	//0XCBFE
	_cbOpcodeTable[0xFF] = &CPU::CB_SET_7_A;	//0XCBFF

	//Startup sequence (GB)
	_clock.Cycle(0, 0);
	_registers.tClock.Cycle(0, 0);
	_hasSetPC = FALSE;

	_registers.A = 0;
	_registers.F = 0;
	_registers.B = 0;
	_registers.C = 0;
	_registers.D = 0;
	_registers.E = 0;
	_registers.H = 0;
	_registers.L = 0;
	_registers.SP = 0;
	_registers.PC = 0x0000;
	
	
	_registers.AF	(0x01B0);
	_registers.F	= 0xB0;
	_registers.BC	(0x0013);
	_registers.DE	(0x00D8);
	_registers.HL	(0x014D);
	_registers.SP	= 0xFFFE;
	_registers.PC = 0x0100;
	Write8(TIMA, 0x00);
	Write8(TMA, 0x00);
	Write8(TAC, 0x00);
	Write8(NR10, 0x80);
	Write8(NR11, 0xBF);
	Write8(NR12, 0xF3);
	Write8(NR14, 0xBF);
	Write8(NR21, 0x3F);
	Write8(NR22, 0x00);
	Write8(NR24, 0xBF);
	Write8(NR30, 0x7F);
	Write8(NR31, 0xFF);
	Write8(NR32, 0x9F);
	Write8(NR33, 0xBF);
	Write8(NR41, 0xFF);
	Write8(NR42, 0x00);
	Write8(NR43, 0x00);
	Write8(NR44, 0xBF);
	Write8(NR50, 0x77);
	Write8(NR51, 0xF3);
	Write8(NR52, 0xF1); //Set to 0xF0 for SGB
	Write8(LCDC, 0x91);
	Write8(SCY, 0x00);
	Write8(SCX, 0x00);
	Write8(LYC, 0x00);
	Write8(BGP, 0xFC);
	Write8(OBP0, 0xFF);
	Write8(OBP1, 0xFF);
	Write8(WY, 0x00);
	Write8(WX, 0x00);
	Write8(IE, 0x00);
	_mmu->BIOSLoaded(TRUE);
	Write8(STAT, 0x02);
}

uint16 CPU::GetCycles()
{
	return _clock.cpu_cycles;
}
uint16 CPU::ResetCycles()
{
	return _clock.cpu_cycles = 0;
}

bool CPU::Run()
{
	//check if in BIOS
	if (_registers.PC == 0x100)
	{
		_mmu->BIOSLoaded(TRUE);
	}

	if (_registers.PC == 0x0055)
	{
		int i = 0;
	}

	//(expected sequence)
	BOOL runningState = TRUE;
	uint8 val = Read8(_registers.PC);
	uint16 pc = _registers.PC;
	//printf("%s [0x%X] (0x%X)\n", opcodeNames[val] , pc, val);
	//cout << opcodeNames[val];
	_registers.PC += 0x0001;
	if (val == 0xCB)
	{
		//cb
		uint8 pc = Read8(_registers.PC);
		_registers.PC += 0x0001;
		(this->*(_cbOpcodeTable[pc]))();
		_registers.tClock.byte_call_cycles + 1;
		_registers.tClock.cpu_cycles + 4;
	}
	else
	{
		(this->*(_opcodeTable[val]))();
	}

	pc += _registers.tClock.byte_call_cycles;
	if (_hasSetPC == FALSE)
	{
		_registers.PC = pc;
	}
	else
	{
		//_registers.PC += _registers.tClock.byte_call_cycles - 1;
	}

	//reset
	_clock.byte_call_cycles += _registers.tClock.byte_call_cycles;
	_clock.cpu_cycles		+= _registers.tClock.cpu_cycles;

	_registers.tClock.Cycle(0, 0);
	_hasSetPC = FALSE;

	if (input->IsExit()) { runningState = FALSE; }
	return runningState;
}

uint8 CPU::Read8(uint16 address)
{
	return _mmu->Read8(address);
}

uint16 CPU::Read16(uint16 address)
{
	return _mmu->Read16(address);
}

void CPU::Write8(uint16 address, uint8 value)
{
	return _mmu->Write8(address, value);
}

void CPU::Write16(uint16 address, uint16 value)
{
	return _mmu->Write16(address, value);
}

uint8 CPU::POP8_SP()
{
	uint8 result = Read8(_registers.SP + 0x0001);
	Write8(_registers.SP + 0x0001, 0x0000);
	_registers.SP += 0x0001;

	return result;
}

uint16 CPU::POP16_SP()
{
	uint16 result = Read16(_registers.SP + 0x0001);
	Write16(_registers.SP + 0x0001, 0x0000);
	_registers.SP += 0x0002;

	return result;
}

void CPU::PUSH8_SP(uint8 value)
{
	Write16(_registers.SP, value);
	_registers.SP -= 0x0001;
}

void CPU::PUSH16_SP(uint16 value)
{
	Write16(_registers.SP - 0x0001, value);
	_registers.SP -= 0x0002;
}

void CPU::Set_Z(BOOL value)
{
	Set_Bit(_registers.F, 7, value);
}

void CPU::Set_N(BOOL value)
{
	Set_Bit(_registers.F, 6, value);
}

void CPU::Set_H(BOOL value)
{
	Set_Bit(_registers.F, 5, value);
}
void CPU::Set_C(BOOL value)
{
	Set_Bit(_registers.F, 4, value);
}

void CPU::NOP()
{
	_registers.tClock.Cycle(1, 4);
}

void CPU::LD_BC_RR()
{
	_registers.BC(Read16(_registers.PC));
	_registers.tClock.Cycle(3, 12);
}

void CPU::LD_BC_A()
{
	Write8(_registers.BC(), _registers.A);
	_registers.tClock.Cycle(1, 8);
}

void CPU::INC_BC()
{
	uint16 tempBC = _registers.BC();
	tempBC += 0x0001;
	_registers.BC(tempBC);

	_registers.tClock.Cycle(1, 8);
}

void CPU::INC_B()
{
	INC_M(_registers.B);
}

void CPU::DEC_B()
{
	DEC_M(_registers.B);
}

void CPU::LD_B_R()
{
	_registers.B = Read8(_registers.PC);
	_registers.tClock.Cycle(2, 8);
}

void CPU::RL_CA()
{
	uint8 oldA = _registers.A;
	_registers.A = _registers.A << 1;

	Set_Z(_registers.A == 0);
	Set_N(FALSE);
	Set_H(FALSE);
	Set_C(Get_Bit(oldA, 7));
	Set_Bit(_registers.A, 7, Get_Bit(oldA, 0));

	_registers.tClock.Cycle(1, 4);
}

void CPU::LD_RR_SP()
{
	Write8(Read16(_registers.PC), Read8(_registers.SP));
	_registers.tClock.Cycle(3, 20);
}

void CPU::ADD_HL_BC()
{
	uint32 oldHL = _registers.HL();
	uint32 oldBC = _registers.BC();
	BOOL cCarried = FALSE;
	BOOL hCarried = HCarryFrom(oldHL, oldBC, From11Borrow12);

	if (oldHL + oldBC > 0xFFFF)
	{
		cCarried = TRUE;
	}

	_registers.HL(_registers.HL() + _registers.BC());

	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, hCarried);
	Set_Bit(_registers.F, 4, cCarried);
	_registers.tClock.Cycle(1, 8);
}

void CPU::LD_A_BC()
{
	_registers.A = Read8(_registers.BC());
	_registers.tClock.Cycle(1, 8);
}

void CPU::DEC_BC()
{
	uint16 tempBC = _registers.BC();
	tempBC -= 0x0001;
	_registers.BC(tempBC);

	_registers.tClock.Cycle(1, 8);
}

void CPU::INC_C()
{
	INC_M(_registers.C);
}

void CPU::DEC_C()
{
	DEC_M(_registers.C);
}

void CPU::LD_C_R()
{
	_registers.C = Read8(_registers.PC);
	_registers.tClock.Cycle(2, 8);
}

void CPU::RR_CA()
{
	uint8 oldA = _registers.A;
	_registers.A = _registers.A >> 1;

	Set_Z(_registers.A == 0);
	Set_N(FALSE);
	Set_H(FALSE);
	Set_C(Get_Bit(oldA, 0));
	Set_Bit(_registers.A, 0, Get_Bit(oldA, 7));

	_registers.tClock.Cycle(1, 4);
}

void CPU::STOP()
{
	//TODO: HALT display/CPU
	//TODO: WAIT FOR INPUT
	while (input->HasInput() == FALSE);
	_registers.tClock.Cycle(2, 4);
}

void CPU::LD_DE_RR()
{
	uint16 tempDE = _registers.DE();
	tempDE = Read16(_registers.PC);
	_registers.DE(tempDE);
	_registers.tClock.Cycle(3, 12);
}

void CPU::LD_DE_A()
{
	Write8(Read16(_registers.DE()), _registers.A);
	_registers.tClock.Cycle(1, 8);
}

void CPU::INC_DE()
{
	uint16 tempDE = _registers.DE();
	tempDE += 0x0001;
	_registers.DE(tempDE);
	_registers.tClock.Cycle(1, 8);
}

void CPU::INC_D()
{
	INC_M(_registers.D);
}

void CPU::DEC_D()
{
	DEC_M(_registers.D);
}

void CPU::LD_D_R()
{
	_registers.D = Read8(_registers.PC);
	_registers.tClock.Cycle(2, 8);
}

void CPU::RL_A()
{
	uint8 oldA = _registers.A;
	_registers.A = _registers.A << 1;
	Set_Bit(_registers.A, 0, Get_Bit(_registers.F, 4));
	Set_C(Get_Bit(oldA, 7));
	Set_Z(_registers.A == 0);
	Set_N(0);
	Set_H(0);
	_registers.tClock.Cycle(1, 4);
}

void CPU::JR_R()
{
	uint8 e = Read8(_registers.PC);
	_registers.PC += 0x0001;
	if (Get_Bit(e, 7) == TRUE)
	{
		//Make Negative;
		e -= 0x80;
		e = 0x80 - e;
		_registers.PC -= e;
	}
	else
	{
		_registers.PC += e ;
	}
	
	_hasSetPC = TRUE;
	_registers.tClock.Cycle(2, 12);
}

void CPU::ADD_HL_DE()
{
	uint32 oldHL = _registers.HL();
	uint32 oldDE = _registers.DE();
	BOOL cCarried = FALSE;
	BOOL hCarried = HCarryFrom(oldHL, oldDE, From11Borrow12);

	if (oldHL + oldDE > 0xFFFF)
	{
		cCarried = TRUE;
	}

	_registers.HL(_registers.HL() + _registers.DE());

	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, hCarried);
	Set_Bit(_registers.F, 4, cCarried);
	_registers.tClock.Cycle(1, 8);
}

void CPU::LD_A_DE()
{
	_registers.A = Read8(_registers.DE());
	_registers.tClock.Cycle(1, 8);
}

void CPU::DEC_DE()
{
	uint16 tempDE = _registers.DE();
	tempDE -= 0x0001;
	_registers.DE(tempDE);
	_registers.tClock.Cycle(1, 8);
}

void CPU::INC_E()
{
	INC_M(_registers.E);
}

void CPU::DEC_E()
{
	DEC_M(_registers.E);
}

void CPU::LD_E_R()
{
	_registers.E = Read8(_registers.PC);
	_registers.tClock.Cycle(2, 8);
}

void CPU::RR_A()
{
	uint8 oldA = _registers.A;
	_registers.A = _registers.A >> 1;
	Set_Bit(_registers.A, 7, Get_Bit(_registers.F, 4));
	Set_C(Get_Bit(oldA, 0));
	Set_Z(_registers.A == 0);
	Set_N(0);
	Set_H(0);
	_registers.tClock.Cycle(1, 4);
}

void CPU::JR_NZ_R()
{
	if (Get_Bit(_registers.F, 7) == FALSE)
	{
		/*_registers.PC += Read8(_registers.PC);
		_registers.PC &= 0xFF;
		_hasSetPC = TRUE;
		_registers.tClock.Cycle(2, 12);*/
		JR_R();
	}
	else
	{
		_registers.tClock.Cycle(2, 8);
	}
}

void CPU::LD_HL_RR()
{
	_registers.HL(Read16(_registers.PC));
	_registers.tClock.Cycle(3, 12);
}

void CPU::LD_MM_PLUS_A()
{
	Write8(_registers.HL(), _registers.A);
	_registers.HL(_registers.HL() + 0x0001);
	_registers.tClock.Cycle(1, 8);
}

void CPU::INC_HL()
{
	uint16 tempHL = _registers.HL();
	tempHL += 0x0001;
	_registers.HL(tempHL);
	_registers.tClock.Cycle(1, 8);
}

void CPU::INC_H()
{
	INC_M(_registers.H);
}

void CPU::DEC_H()
{
	DEC_M(_registers.H);
}

void CPU::LD_H_R()
{
	_registers.H = Read8(_registers.PC);
	_registers.tClock.Cycle(2, 8);
}

void CPU::DA_A()
{
	BOOL C = Get_Bit(_registers.F, 4);
	BOOL H = Get_Bit(_registers.F, 5);

	uint8 oldA = _registers.A;
	uint8 upper = oldA >> 4;
	uint8 lower = oldA & 0x0F;
	//ADD,ADC,INC (only first line SUB,SBC,DEC,NEG)
			if ((C == FALSE) && (upper >= 0x00 && upper <= 0x09) && (H == FALSE) && (lower >= 0x00 && lower <= 0x09))
	{
		oldA += 0x00; Set_Bit(_registers.F, 4, FALSE);
	}
	else	if ((C == FALSE) && (upper >= 0x00 && upper <= 0x08) && (H == FALSE) && (lower >= 0x0A && lower <= 0x0F))
	{
		oldA += 0x06; Set_Bit(_registers.F, 4, FALSE);
	}
	else	if ((C == FALSE) && (upper >= 0x00 && upper <= 0x09) && (H == TRUE) && (lower >= 0x00 && lower <= 0x03))
	{
		oldA += 0x06; Set_Bit(_registers.F, 4, FALSE);
	}
	else	if ((C == FALSE) && (upper >= 0x0A && upper <= 0x0F) && (H == FALSE) && (lower >= 0x00 && lower <= 0x09))
	{
		oldA += 0x60; Set_Bit(_registers.F, 4, TRUE);
	}
	else	if ((C == FALSE) && (upper >= 0x09 && upper <= 0x0F) && (H == FALSE) && (lower >= 0x0A && lower <= 0x0F))
	{
		oldA += 0x66; Set_Bit(_registers.F, 4, TRUE);
	}
	else	if ((C == FALSE) && (upper >= 0x0A && upper <= 0x0F) && (H == TRUE) && (lower >= 0x00 && lower <= 0x03))
	{
		oldA += 0x66; Set_Bit(_registers.F, 4, TRUE);
	}
	else	if ((C == TRUE) && (upper >= 0x00 && upper <= 0x02) && (H == FALSE) && (lower >= 0x00 && lower <= 0x09))
	{
		oldA += 0x60; Set_Bit(_registers.F, 4, TRUE);
	}
	else	if ((C == TRUE) && (upper >= 0x00 && upper <= 0x02) && (H == FALSE) && (lower >= 0x0A && lower <= 0x0F))
	{
		oldA += 0x66; Set_Bit(_registers.F, 4, TRUE);
	}
	else	if ((C == TRUE) && (upper >= 0x00 && upper <= 0x03) && (H == TRUE) && (lower >= 0x00 && lower <= 0x09))
	{
		oldA += 0x66; Set_Bit(_registers.F, 4, TRUE);
	}
	//SUB,SBC,DEC,NEG
	else	if ((C == FALSE) && (upper >= 0x00 && upper <= 0x08) && (H == TRUE) && (lower >= 0x06 && lower <= 0x0F))
	{
		oldA += 0xFA; Set_Bit(_registers.F, 4, FALSE);
	}
	else	if ((C == TRUE) && (upper >= 0x07 && upper <= 0x0F) && (H == FALSE) && (lower >= 0x00 && lower <= 0x09))
	{
		oldA += 0xA0; Set_Bit(_registers.F, 4, TRUE);
	}
	else	if ((C == TRUE) && (upper >= 0x06 && upper <= 0x0F) && (H == TRUE) && (lower >= 0x06 && lower <= 0x0F))
	{
		oldA += 0x9A; Set_Bit(_registers.F, 4, TRUE);
	}

	Set_Bit(_registers.F, 5, FALSE);
	Set_Bit(_registers.F, 7, (_registers.A == 0x00));
	_registers.tClock.Cycle(1, 4);
}

void CPU::JR_Z_R()
{
	if (Get_Bit(_registers.F, 7) == TRUE)
	{
		/*
		_registers.PC += Read8(_registers.PC);
		_hasSetPC = TRUE;
		_registers.tClock.Cycle(2, 12);*/
		JR_R();
	}
	else
	{
		_registers.tClock.Cycle(2, 8);
	}
}

void CPU::ADD_HL_HL()
{
	uint16 oldHL = _registers.HL();
	BOOL hCarried = (oldHL > 0x800) ? TRUE : FALSE;
	BOOL cCarried = (oldHL > 0x8000)? TRUE : FALSE;

	_registers.HL(_registers.HL() + _registers.HL());
	
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, hCarried);
	Set_Bit(_registers.F, 4, cCarried);
	_registers.tClock.Cycle(1, 8);
}

void CPU::LD_A_MM_PLUS()
{
	_registers.A = Read8(_registers.HL());
	_registers.HL(_registers.HL() + 0x0001);
	_registers.tClock.Cycle(1, 8);
}

void CPU::DEC_HL()
{
	uint16 tempHL = _registers.HL();
	tempHL -= 0x0001;
	_registers.HL(tempHL);
	_registers.tClock.Cycle(1, 8);
}

void CPU::INC_L()
{
	INC_M(_registers.L);
}

void CPU::DEC_L()
{
	DEC_M(_registers.L);
}

void CPU::LD_L_R()
{
	_registers.L = Read8(_registers.PC);
	_registers.tClock.Cycle(2, 8);
}

void CPU::CPL()
{
	BOOL A[] = {
		Get_Bit(_registers.A, 0),
		Get_Bit(_registers.A, 1),
		Get_Bit(_registers.A, 2),
		Get_Bit(_registers.A, 3),
		Get_Bit(_registers.A, 4),
		Get_Bit(_registers.A, 5),
		Get_Bit(_registers.A, 6),
		Get_Bit(_registers.A, 7)
	};

	Set_Bit(_registers.A, 0, A[7]);
	Set_Bit(_registers.A, 1, A[6]);
	Set_Bit(_registers.A, 2, A[5]);
	Set_Bit(_registers.A, 3, A[4]);
	Set_Bit(_registers.A, 4, A[3]);
	Set_Bit(_registers.A, 5, A[2]);
	Set_Bit(_registers.A, 6, A[1]);
	Set_Bit(_registers.A, 7, A[0]);

	Set_Bit(_registers.F, 6, TRUE);
	Set_Bit(_registers.F, 5, TRUE);
	_registers.tClock.Cycle(1, 4);
}

void CPU::JR_NC_R()
{
	if (Get_Bit(_registers.F, 4) == FALSE)
	{
		/*_registers.PC += Read8(_registers.PC);
		_hasSetPC = TRUE;
		_registers.tClock.Cycle(2, 12);*/
		JR_R();
	}
	else
	{
		_registers.tClock.Cycle(2, 8);
	}
}

void CPU::LD_SP_RR()
{
	_registers.SP = Read16(_registers.PC);
	_registers.tClock.Cycle(3, 12);
}

void CPU::LD_MM_MIN_A()
{
	Write8(_registers.HL(), _registers.A);
	_registers.HL(_registers.HL() - 0x0001);
	_registers.tClock.Cycle(1, 8);
}

void CPU::INC_SP()
{
	_registers.SP += 0x0001;
	_registers.tClock.Cycle(1, 8);
}

void CPU::INC_MM()
{
	uint8 tempMM = Read8(_registers.HL());
	tempMM += 0x01;
	Write8(_registers.HL(), tempMM);
	Set_Z((_registers.HL() == 0));
	Set_N(FALSE);
	Set_H(HCarryFrom(tempMM - 0x01, 0x01, From3Borrow4));
	_registers.tClock.Cycle(1, 12);
}

void CPU::DEC_MM()
{
	uint8 tempMM = Read8(_registers.HL());
	tempMM -= 0x01;
	Write8(_registers.HL(), tempMM);
	Set_Z((_registers.HL() == 0));
	Set_N(TRUE);
	Set_H(HCarryBorrow(tempMM + 0x01, 0x01, From3Borrow4));
	_registers.tClock.Cycle(1, 12);
}

void CPU::LD_MM_R()
{
	Write8(_registers.HL(), Read8(_registers.PC));
	_registers.tClock.Cycle(2, 12);
}

void CPU::SCF()
{
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, FALSE);
	Set_Bit(_registers.F, 4, TRUE);
	_registers.tClock.Cycle(1, 4);
}

void CPU::JR_C_R()
{
	if (Get_Bit(_registers.F, 4) == TRUE)
	{
		/*_registers.PC += Read8(_registers.PC);
		_hasSetPC = TRUE;
		_registers.tClock.Cycle(2, 12);*/
		JR_R();
	}
	else
	{
		_registers.tClock.Cycle(2, 8);
	}
}

void CPU::ADD_HL_SP()
{
	uint32 oldHL = _registers.HL();
	uint32 oldSP = _registers.SP;
	BOOL cCarried = FALSE;
	BOOL hCarried = HCarryFrom(oldHL, oldSP, From11Borrow12);

	if (oldHL + oldSP > 0xFFFF)
	{
		cCarried = TRUE;
	}

	_registers.HL(_registers.HL() + _registers.SP);

	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, hCarried);
	Set_Bit(_registers.F, 4, cCarried);
	_registers.tClock.Cycle(1, 8);
}

void CPU::LD_A_MM_MIN()
{
	_registers.A = Read8(_registers.HL());
	_registers.HL(_registers.HL() - 0x0001);
	_registers.tClock.Cycle(1, 8);
}

void CPU::DEC_SP()
{
	_registers.SP--;
	_registers.tClock.Cycle(1, 8);
}

void CPU::INC_A()
{
	INC_M(_registers.A);
}

void CPU::DEC_A()
{
	DEC_M(_registers.A);
}

void CPU::LD_A_R()
{
	_registers.A = Read8(_registers.PC);
	_registers.tClock.Cycle(2, 8);
}

void CPU::CCF()
{
	BOOL C = Get_Bit(_registers.F, 4);

	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, C);
	Set_Bit(_registers.F, 4, (C == TRUE)? FALSE : TRUE);
	_registers.tClock.Cycle(1, 4);
}

void CPU::LD_B_B()
{
	LD_M_M(_registers.B, _registers.B);
}

void CPU::LD_B_C()
{
	LD_M_M(_registers.B, _registers.C);
}

void CPU::LD_B_D()
{
	LD_M_M(_registers.B, _registers.D);
}

void CPU::LD_B_E()
{
	LD_M_M(_registers.B, _registers.E);
}

void CPU::LD_B_H()
{
	LD_M_M(_registers.B, _registers.H);
}

void CPU::LD_B_L()
{
	LD_M_M(_registers.B, _registers.L);
}

void CPU::LD_B_MM()
{
	_registers.B = Read8(_registers.HL());
	_registers.tClock.Cycle(1, 8);
}

void CPU::LD_B_A()
{
	LD_M_M(_registers.B, _registers.A);
}

void CPU::LD_C_B()
{
	LD_M_M(_registers.C, _registers.B);
}

void CPU::LD_C_C()
{
	LD_M_M(_registers.C, _registers.C);
}

void CPU::LD_C_D()
{
	LD_M_M(_registers.C, _registers.D);
}

void CPU::LD_C_E()
{
	LD_M_M(_registers.C, _registers.E);
}

void CPU::LD_C_H()
{
	LD_M_M(_registers.C, _registers.H);
}

void CPU::LD_C_L()
{
	LD_M_M(_registers.C, _registers.L);
}

void CPU::LD_C_MM()
{
	_registers.C = Read8(_registers.HL());
	_registers.tClock.Cycle(1, 8);
}

void CPU::LD_C_A()
{
	LD_M_M(_registers.C, _registers.A);
}

void CPU::LD_D_B()
{
	LD_M_M(_registers.D, _registers.B);
}

void CPU::LD_D_C()
{
	LD_M_M(_registers.D, _registers.C);
}

void CPU::LD_D_D()
{
	LD_M_M(_registers.D, _registers.D);
}

void CPU::LD_D_E()
{
	LD_M_M(_registers.D, _registers.E);
}

void CPU::LD_D_H()
{
	LD_M_M(_registers.D, _registers.H);
}

void CPU::LD_D_L()
{
	LD_M_M(_registers.D, _registers.L);
}

void CPU::LD_D_MM()
{
	_registers.D = Read8(_registers.HL());
	_registers.tClock.Cycle(1, 8);
}

void CPU::LD_D_A()
{
	LD_M_M(_registers.D, _registers.A);
}

void CPU::LD_E_B()
{
	LD_M_M(_registers.E, _registers.B);
}

void CPU::LD_E_C()
{
	LD_M_M(_registers.E, _registers.C);
}

void CPU::LD_E_D()
{
	LD_M_M(_registers.E, _registers.D);
}

void CPU::LD_E_E()
{
	LD_M_M(_registers.E, _registers.E);
}

void CPU::LD_E_H()
{
	LD_M_M(_registers.E, _registers.H);
}

void CPU::LD_E_L()
{
	LD_M_M(_registers.E, _registers.L);
}

void CPU::LD_E_MM()
{
	_registers.E = Read8(_registers.HL());
	_registers.tClock.Cycle(1, 8);
}

void CPU::LD_E_A()
{
	LD_M_M(_registers.E, _registers.A);
}

void CPU::LD_H_B()
{
	LD_M_M(_registers.H, _registers.B);
}

void CPU::LD_H_C()
{
	LD_M_M(_registers.H, _registers.C);
}

void CPU::LD_H_D()
{
	LD_M_M(_registers.H, _registers.D);
}

void CPU::LD_H_E()
{
	LD_M_M(_registers.H, _registers.E);
}

void CPU::LD_H_H()
{
	LD_M_M(_registers.H, _registers.H);
}

void CPU::LD_H_L()
{
	LD_M_M(_registers.H, _registers.L);
}

void CPU::LD_H_MM()
{
	_registers.H = Read8(_registers.HL());
	_registers.tClock.Cycle(1, 8);
}

void CPU::LD_H_A()
{
	LD_M_M(_registers.H, _registers.A);
}

void CPU::LD_L_B()
{
	LD_M_M(_registers.L, _registers.B);
}

void CPU::LD_L_C()
{
	LD_M_M(_registers.L, _registers.C);
}

void CPU::LD_L_D()
{
	LD_M_M(_registers.L, _registers.D);
}

void CPU::LD_L_E()
{
	LD_M_M(_registers.L, _registers.E);
}

void CPU::LD_L_H()
{
	LD_M_M(_registers.L, _registers.H);
}

void CPU::LD_L_L()
{
	LD_M_M(_registers.L, _registers.L);
}

void CPU::LD_L_MM()
{
	_registers.L = Read8(_registers.HL());
	_registers.tClock.Cycle(1, 8);
}

void CPU::LD_L_A()
{
	LD_M_M(_registers.L, _registers.A);
}

void CPU::LD_MM_B()
{
	Write8(_registers.HL(), _registers.B);
	_registers.tClock.Cycle(1, 8);
}

void CPU::LD_MM_C()
{
	Write8(_registers.HL(), _registers.C);
	_registers.tClock.Cycle(1, 8);
}

void CPU::LD_MM_D()
{
	Write8(_registers.HL(), _registers.D);
	_registers.tClock.Cycle(1, 8);
}

void CPU::LD_MM_E()
{
	Write8(_registers.HL(), _registers.E);
	_registers.tClock.Cycle(1, 8);
}

void CPU::LD_MM_H()
{
	Write8(_registers.HL(), _registers.H);
	_registers.tClock.Cycle(1, 8);
}

void CPU::LD_MM_L()
{
	Write8(_registers.HL(), _registers.L);
	_registers.tClock.Cycle(1, 8);
}

void CPU::HALT()
{
	//TODO: HALT
	Write8(0xFFFF, 0x01);
	_registers.tClock.Cycle(1, 4);
}

void CPU::LD_MM_A()
{
	Write8(_registers.HL(), _registers.A);
	_registers.tClock.Cycle(1, 8);
}

void CPU::LD_A_B()
{
	LD_M_M(_registers.A, _registers.B);
}

void CPU::LD_A_C()
{
	LD_M_M(_registers.A, _registers.C);
}

void CPU::LD_A_D()
{
	LD_M_M(_registers.A, _registers.D);
}

void CPU::LD_A_E()
{
	LD_M_M(_registers.A, _registers.E);
}

void CPU::LD_A_H()
{
	LD_M_M(_registers.A, _registers.H);
}

void CPU::LD_A_L()
{
	LD_M_M(_registers.A, _registers.L);
}

void CPU::LD_A_MM()
{
	_registers.A = Read8(_registers.HL());
	_registers.tClock.Cycle(1, 8);
}

void CPU::LD_A_A()
{
	LD_M_M(_registers.A, _registers.A);
}

void CPU::ADD_A_B()
{
	ADD_A_M(_registers.B);
}

void CPU::ADD_A_C()
{
	ADD_A_M(_registers.C);
}

void CPU::ADD_A_D()
{
	ADD_A_M(_registers.D);
}

void CPU::ADD_A_E()
{
	ADD_A_M(_registers.E);
}

void CPU::ADD_A_H()
{
	ADD_A_M(_registers.H);
}

void CPU::ADD_A_L()
{
	ADD_A_M(_registers.L);
}

void CPU::ADD_A_MM()
{
	ADD_A_M(Read8(_registers.HL()));
	_registers.tClock.Cycle(1, 8);
}

void CPU::ADD_A_A()
{
	ADD_A_M(_registers.A);
}

void CPU::ADD_A_M(uint8 value)
{
	/*uint16 oldHL = Read8(_registers.HL());
	uint16 oldA = _registers.A;
	BOOL hCarried = FALSE;
	BOOL cCarried = FALSE;

	if (oldA + oldHL > 0xFF)
	{
		cCarried = TRUE;
	}
	if (oldA & 0x0F + oldHL > 0xF0)
	{
		hCarried = TRUE;
	}

	_registers.A += oldHL;

	Set_Bit(_registers.F, 6, (_registers.A == 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, hCarried);
	Set_Bit(_registers.F, 4, cCarried);
	_registers.tClock.Cycle(1, 4);*/

	BOOL hCarried = FALSE;
	BOOL cCarried = FALSE;
	math8(FALSE, 0, value, _registers.A, _registers.F, hCarried, cCarried);
	_registers.tClock.Cycle(1, 4);
}

void CPU::ADC_A_B()
{
	ADC_A_M(_registers.B);
}

void CPU::ADC_A_C()
{
	ADC_A_M(_registers.C);
}

void CPU::ADC_A_D()
{
	ADC_A_M(_registers.D);
}

void CPU::ADC_A_E()
{
	ADC_A_M(_registers.E);
}

void CPU::ADC_A_H()
{
	ADC_A_M(_registers.H);
}

void CPU::ADC_A_L()
{
	ADC_A_M(_registers.L);
}

void CPU::ADC_A_MM()
{
	ADD_A_M(Read8(_registers.HL()));
	_registers.tClock.Cycle(1, 8);
}

void CPU::ADC_A_A()
{
	ADC_A_M(_registers.A);
}

void CPU::ADC_A_M(uint8 value)
{
	/*uint16 oldHL = Read8(_registers.HL());
	uint16 oldA = _registers.A;
	uint8 C = Get_Bit(_registers.F, 4);
	BOOL hCarried = FALSE;
	BOOL cCarried = FALSE;

	if (oldA + oldHL + C > 0xFF)
	{
		cCarried = TRUE;
	}
	if (oldA & 0x0F + oldHL + C > 0xF0)
	{
		hCarried = TRUE;
	}

	_registers.A += oldHL = C;

	Set_Bit(_registers.F, 6, (_registers.A == 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, hCarried);
	Set_Bit(_registers.F, 4, cCarried);
	_registers.tClock.Cycle(1, 4);*/


	BOOL hCarried = FALSE;
	BOOL cCarried = FALSE;
	math8(FALSE, Get_Bit(_registers.F, 4), value, _registers.A, _registers.F, hCarried, cCarried);
	_registers.tClock.Cycle(1, 4);
}

void CPU::SUB_A_B()
{
	SUB_A_M(_registers.B);
}

void CPU::SUB_A_C()
{
	SUB_A_M(_registers.C);
}

void CPU::SUB_A_D()
{
	SUB_A_M(_registers.D);
}

void CPU::SUB_A_E()
{
	SUB_A_M(_registers.E);
}

void CPU::SUB_A_H()
{
	SUB_A_M(_registers.H);
}

void CPU::SUB_A_L()
{
	SUB_A_M(_registers.L);
}

void CPU::SUB_A_MM()
{
	SUB_A_M(Read8(_registers.HL()));
	_registers.tClock.Cycle(1, 8);
}

void CPU::SUB_A_A()
{
	SUB_A_M(_registers.A);
}

void CPU::SUB_A_M(uint8 value)
{
	BOOL hCarried = FALSE;
	BOOL cCarried = FALSE;
	math8(TRUE, 0, value, _registers.A, _registers.F, hCarried, cCarried);
	_registers.tClock.Cycle(1, 4);
}

void CPU::SBC_A_B()
{
	SBC_A_M(_registers.B);
}

void CPU::SBC_A_C()
{
	SBC_A_M(_registers.C);
}

void CPU::SBC_A_D()
{
	SBC_A_M(_registers.D);
}

void CPU::SBC_A_E()
{
	SBC_A_M(_registers.E);
}

void CPU::SBC_A_H()
{
	SBC_A_M(_registers.H);
}

void CPU::SBC_A_L()
{
	SBC_A_M(_registers.L);
}

void CPU::SBC_A_MM()
{
	SBC_A_M(Read8(_registers.HL()));
	_registers.tClock.Cycle(1, 8);
}

void CPU::SBC_A_A()
{
	SBC_A_M(_registers.A);
}

void CPU::SBC_A_M(uint8 value)
{
	BOOL hCarried = FALSE;
	BOOL cCarried = FALSE;
	math8(TRUE, Get_Bit(_registers.F, 4), Read8(_registers.HL()), _registers.A, _registers.F, hCarried, cCarried);
	_registers.tClock.Cycle(1, 4);
}

void CPU::AND_B()
{
	AND_M(_registers.B);
}

void CPU::AND_C()
{
	AND_M(_registers.C);
}

void CPU::AND_D()
{
	AND_M(_registers.D);
}

void CPU::AND_E()
{
	AND_M(_registers.E);
}

void CPU::AND_H()
{
	AND_M(_registers.H);
}

void CPU::AND_L()
{
	AND_M(_registers.L);
}

void CPU::AND_MM()
{
	_registers.A &= Read8(_registers.HL());
	Set_Bit(_registers.F, 7, (_registers.A == 0));
	Set_Bit(_registers.F, 6, 0);
	Set_Bit(_registers.F, 5, 1);
	Set_Bit(_registers.F, 4, 0);
	_registers.tClock.Cycle(1, 8);
}

void CPU::AND_A()
{
	AND_M(_registers.A);
}

void CPU::AND_M(uint8 value)
{
	_registers.A &= value;
	Set_Bit(_registers.F, 7, (_registers.A == 0));
	Set_Bit(_registers.F, 6, 0);
	Set_Bit(_registers.F, 5, 1);
	Set_Bit(_registers.F, 4, 0);
	_registers.tClock.Cycle(1, 4);
}

void CPU::XOR_B()
{
	XOR_M(_registers.B);
}

void CPU::XOR_C()
{
	XOR_M(_registers.C);
}

void CPU::XOR_D()
{
	XOR_M(_registers.D);
}

void CPU::XOR_E()
{
	XOR_M(_registers.E);
}

void CPU::XOR_H()
{
	XOR_M(_registers.H);
}

void CPU::XOR_L()
{
	XOR_M(_registers.L);
}

void CPU::XOR_MM()
{
	XOR_M(Read8(_registers.HL()));
	_registers.tClock.Cycle(1, 8);
}

void CPU::XOR_A()
{
	XOR_M(_registers.A);
}

void CPU::XOR_M(uint8 value)
{
	_registers.A ^= value;
	Set_Bit(_registers.F, 7, (_registers.A == 0));
	Set_Bit(_registers.F, 6, 0);
	Set_Bit(_registers.F, 5, 0);
	Set_Bit(_registers.F, 4, 0);
	_registers.tClock.Cycle(1, 4);
}

void CPU::OR_B()
{
	OR_M(_registers.B);
}

void CPU::OR_C()
{
	OR_M(_registers.C);
}

void CPU::OR_D()
{
	OR_M(_registers.D);
}

void CPU::OR_E()
{
	OR_M(_registers.E);
}

void CPU::OR_H()
{
	OR_M(_registers.H);
}

void CPU::OR_L()
{
	OR_M(_registers.L);
}

void CPU::OR_MM()
{
	OR_M(Read8(_registers.HL()));
	_registers.tClock.Cycle(1, 8);
}

void CPU::OR_A()
{
	OR_M(_registers.A);
}

void CPU::OR_M(uint8 value)
{
	_registers.A |= value;
	Set_Bit(_registers.F, 7, (_registers.A == 0));
	Set_Bit(_registers.F, 6, 0);
	Set_Bit(_registers.F, 5, 0);
	Set_Bit(_registers.F, 4, 0);
	_registers.tClock.Cycle(1, 4);
}

void CPU::CP_B()
{
	CP_M(_registers.B);
}

void CPU::CP_C()
{
	CP_M(_registers.C);
}

void CPU::CP_D()
{
	CP_M(_registers.D);
}

void CPU::CP_E()
{
	CP_M(_registers.E);
}

void CPU::CP_H()
{
	CP_M(_registers.H);
}

void CPU::CP_L()
{
	CP_M(_registers.L);
}

void CPU::CP_MM()
{
	CP_M(Read8(_registers.HL()));
	_registers.tClock.Cycle(1, 8);
}

void CPU::CP_A()
{
	CP_M(_registers.A);
}

void CPU::CP_M(uint8 value)
{
	BOOL hCarried = HCarryBorrow(_registers.A, value, From3Borrow4);
	uint8 cmp = _registers.A - value;
	Set_Bit(_registers.F, 7, (cmp == 0));
	Set_Bit(_registers.F, 6, TRUE);
	Set_Bit(_registers.F, 5, hCarried);
	Set_Bit(_registers.F, 4, (_registers.A < value));
	_registers.tClock.Cycle(1, 4);
}

void CPU::RET_NZ()
{
	if (Get_Bit(_registers.F, 7) == FALSE)
	{
		RET();
		_registers.tClock.Cycle(1, 20);
	}
	_registers.tClock.Cycle(1, 8);
}

void CPU::POP_BC()
{
	_registers.BC(POP16_SP());
	_registers.tClock.Cycle(1, 12);
}

void CPU::JP_NZ_RR()
{
	if (Get_Bit(_registers.F, 7) == FALSE)
	{
		JP_RR();
		_registers.tClock.Cycle(3, 16);
	}
	_registers.tClock.Cycle(3, 12);
}

void CPU::JP_RR()
{
	uint16 jmpTo = Read16(_registers.PC);
	_registers.PC = jmpTo;
	_hasSetPC = TRUE;
	_registers.tClock.Cycle(3, 12);
}

void CPU::CALL_NZ_RR()
{
	if (Get_Bit(_registers.F, 7) == FALSE)
	{
		CALL_RR();
		_registers.tClock.Cycle(3, 24);
	}
	_registers.tClock.Cycle(3, 12);
}

void CPU::PUSH_BC()
{
	PUSH16_SP(_registers.BC());
	_registers.tClock.Cycle(1, 16);
}

void CPU::ADD_A_R()
{
	ADD_A_M(Read8(_registers.PC));
	_registers.tClock.Cycle(2, 8);
}

void CPU::RST_00H()
{
	PUSH16_SP(_registers.PC);
	_registers.PC = 0x0000;
	_registers.tClock.Cycle(1, 16);
	_hasSetPC = TRUE;
}

void CPU::RET_Z()
{
	if (Get_Bit(_registers.F, 7) == TRUE)
	{
		RET();
		_registers.tClock.Cycle(1, 20);
	}
	_registers.tClock.Cycle(1, 8);
}

void CPU::RET()
{
	uint16 jmpTo = POP16_SP();
	_registers.PC = jmpTo;
	_hasSetPC = TRUE;
	_registers.tClock.Cycle(1, 16);
}

void CPU::JP_Z_RR()
{
	if (Get_Bit(_registers.F, 7) == TRUE)
	{
		JP_RR();
		_registers.tClock.Cycle(3, 16);
	}
	_registers.tClock.Cycle(3, 12);
}

void CPU::PREFIX_CB()
{
	_registers.tClock.Cycle(1, 4);
}

void CPU::CALL_Z_RR()
{
	if (Get_Bit(_registers.F, 7) == TRUE)
	{
		CALL_RR();
		_registers.tClock.Cycle(3, 24);
	}
	_registers.tClock.Cycle(3, 12);
}

void CPU::CALL_RR()
{
	PUSH16_SP(_registers.PC + 0x02);
	uint16 nn = Read16(_registers.PC);

	_registers.PC = nn;
	_hasSetPC = TRUE;

	_registers.tClock.Cycle(3, 12);
}

void CPU::ADC_A_R()
{
	ADC_A_M(Read8(_registers.PC));
	_registers.tClock.Cycle(2, 8);
}

void CPU::RST_08H()
{
	PUSH16_SP(_registers.PC);
	_registers.PC = 0x0008;
	_registers.tClock.Cycle(1, 16);
	_hasSetPC = TRUE;
}

void CPU::RET_NC()
{
	if (Get_Bit(_registers.F, 4) == FALSE)
	{
		RET();
		_registers.tClock.Cycle(1, 20);
	}
	_registers.tClock.Cycle(1, 8);
}

void CPU::POP_DE()
{
	_registers.DE(POP16_SP());
	_registers.tClock.Cycle(1, 12);
}

void CPU::JP_NC_RR()
{
	if (Get_Bit(_registers.F, 4) == FALSE)
	{
		JP_RR();
		_registers.tClock.Cycle(3, 16);
	}
	_registers.tClock.Cycle(3, 12);
}

void CPU::CALL_NC_RR()
{
	if (Get_Bit(_registers.F, 4) == FALSE)
	{
		CALL_RR();
		_registers.tClock.Cycle(3, 24);
	}
	_registers.tClock.Cycle(3, 12);
}

void CPU::PUSH_DE()
{
	PUSH16_SP(_registers.DE());
	_registers.tClock.Cycle(1, 16);
}

void CPU::SUB_R()
{
	SUB_A_M(Read8(_registers.PC));
	_registers.tClock.Cycle(2, 8);
}

void CPU::RST_10H()
{
	PUSH16_SP(_registers.PC);
	_registers.PC = 0x0010;
	_registers.tClock.Cycle(1, 16);
	_hasSetPC = TRUE;
}

void CPU::RET_C()
{
	if (Get_Bit(_registers.F, 4) == TRUE)
	{
		RET();
		_registers.tClock.Cycle(1, 20);
	}
	_registers.tClock.Cycle(1, 8);
}

void CPU::RETI()
{
	EI();
	RET();
	//TODO: interupts
	_registers.tClock.Cycle(1, 16);
}

void CPU::JP_C_RR()
{
	if (Get_Bit(_registers.F, 4) == TRUE)
	{
		JP_RR();
		_registers.tClock.Cycle(3, 16);
	}
	_registers.tClock.Cycle(3, 12);
}

void CPU::CALL_C_RR()
{
	if (Get_Bit(_registers.F, 4) == TRUE)
	{
		CALL_RR();
		_registers.tClock.Cycle(3, 24);
	}
	_registers.tClock.Cycle(3, 12);
}

void CPU::SBC_A_R()
{
	SBC_A_M(Read8(_registers.PC));
	_registers.tClock.Cycle(2, 8);
}

void CPU::RST_18H()
{
	PUSH16_SP(_registers.PC);
	_registers.PC = 0x0018;
	_registers.tClock.Cycle(1, 16);
	_hasSetPC = TRUE;
}

void CPU::LDH_R_A()
{
	Write8(0xFF00 + Read8(_registers.PC), _registers.A);
	_registers.tClock.Cycle(2, 12);
}

void CPU::POP_HL()
{
	_registers.HL(POP16_SP());
	_registers.tClock.Cycle(1, 12);
}

void CPU::LD_C_A2()
{
	Write8(0xFF00 + _registers.C, _registers.A);
	_registers.tClock.Cycle(1, 8);
}

void CPU::PUSH_HL()
{
	PUSH16_SP(_registers.HL());
	_registers.tClock.Cycle(1, 16);
}

void CPU::AND_R()
{
	_registers.A &= Read8(_registers.PC);
	Set_Bit(_registers.F, 7, (_registers.A == 0));
	Set_Bit(_registers.F, 6, 0);
	Set_Bit(_registers.F, 5, 1);
	Set_Bit(_registers.F, 4, 0);
	_registers.tClock.Cycle(2, 8);
}

void CPU::RST_20H()
{
	PUSH16_SP(_registers.PC);
	_registers.PC = 0x0020;
	_registers.tClock.Cycle(1, 16);
	_hasSetPC = TRUE;
}

void CPU::ADD_SP_R()
{
	uint8 e = Read8(_registers.PC);
	BOOL c = FALSE;
	BOOL h = FALSE;
	if (Get_Bit(e, 7) == TRUE)
	{
		//Make Negative;
		e -= 0x80;
		e = 0x80 - e;
		c = (((_registers.SP & 0xFF) - (e & 0xFF)) >> 8) & 0xFF;
		h = (((_registers.SP & 0x0F) - (e & 0x0F)) >> 4) & 0x01;
		_registers.SP -= e;
	}
	else
	{
		c = (((_registers.SP & 0xFF) + (e & 0xFF)) >> 8) & 0xFF;
		h = (((_registers.SP & 0x0F) + (e & 0x0F)) >> 4) & 0x01;
		_registers.SP += e;
	}
	Set_Bit(_registers.F, 7, 0);
	Set_Bit(_registers.F, 6, 0);
	Set_Bit(_registers.F, 5, c);
	Set_Bit(_registers.F, 4, h);
	_registers.tClock.Cycle(2, 16);
}

void CPU::JP_MM()
{
	_registers.PC = _registers.HL();
	_registers.tClock.Cycle(1, 4);
}

void CPU::LD_RR_A()
{
	Write8(Read16(_registers.PC), _registers.A);
	_registers.tClock.Cycle(3, 16);
}

void CPU::XOR_R()
{
	XOR_M(Read8(_registers.PC));
	_registers.tClock.Cycle(2, 8);
}

void CPU::RST_28H()
{
	PUSH16_SP(_registers.PC);
	_registers.PC = 0x0028;
	_registers.tClock.Cycle(1, 16);
	_hasSetPC = TRUE;
}

void CPU::LDH_A_R()
{
	_registers.A = Read8(0xFF00 + Read8(_registers.PC));
	_registers.tClock.Cycle(2, 12);
}

void CPU::POP_AF()
{
	_registers.AF(POP16_SP());
	_registers.tClock.Cycle(1, 12);
}

void CPU::LD_A_C2()
{
	_registers.A = Read8(0xFF00 + _registers.C);
	_registers.tClock.Cycle(1, 8);
}

void CPU::DI()
{
	//TODO: DI (not important now)
	Write8(IE, FALSE);
	_registers.tClock.Cycle(1, 4);
}

void CPU::PUSH_AF()
{
	PUSH16_SP(_registers.AF());
	_registers.tClock.Cycle(1, 16);
}

void CPU::OR_R()
{
	OR_M(Read8(_registers.PC));
	_registers.tClock.Cycle(2, 8);
}

void CPU::RST_30H()
{
	PUSH16_SP(_registers.PC);
	_registers.PC = 0x0030;
	_registers.tClock.Cycle(1, 16);
	_hasSetPC = TRUE;
}

void CPU::LD_HL_SPandR()
{
	uint16 oldHL = _registers.HL();

	uint8 e = Read8(_registers.PC);
	if (Get_Bit(e, 7) == TRUE)
	{
		//Make Negative;
		e -= 0x80;
		e = 0x80 - e;
		e = _registers.SP - e;
	}
	else
	{
		e = _registers.SP + e;
	}
	_registers.HL(e);
	BOOL c = _registers.HL() & 0xFF + oldHL & 0xFF >> 8 & 0xFF;
	BOOL h = _registers.HL() & 0x0F + oldHL & 0x0F >> 4 & 0x01;
	_registers.tClock.Cycle(2, 12);
	Set_Bit(_registers.F, 7, 0);
	Set_Bit(_registers.F, 6, 0);
	Set_Bit(_registers.F, 5, c);
	Set_Bit(_registers.F, 4, h);
	_registers.tClock.Cycle(2, 12);
}

void CPU::LD_SP_HL()
{
	_registers.SP = _registers.HL();
	_registers.tClock.Cycle(1, 8);
}

void CPU::LD_A_RR()
{
	_registers.A = Read8(Read16(_registers.PC));
	_registers.tClock.Cycle(3, 16);
}

void CPU::EI()
{
	//TODO: SET INTERUPTS
	Write8(IE, 0xFF);
	_registers.tClock.Cycle(1, 4);
}

void CPU::CP_R()
{
	CP_M(Read8(_registers.PC));
	_registers.tClock.Cycle(2, 8);
}

void CPU::RST_38H()
{
	PUSH16_SP(_registers.PC);
	_registers.PC = 0x0038;
	_registers.tClock.Cycle(1, 16);
	_hasSetPC = TRUE;
}

void CPU::LD_M_M(uint8 & address, uint8 value)
{
	address = value;
	_registers.tClock.Cycle(1, 4);
}

void CPU::INC_M(uint8 & address)
{
	BOOL hCarried = HCarryFrom(address, 0x01, From3Borrow4);
	address += 0x01;
	Set_Bit(_registers.F, 7, (address == 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, hCarried);
	_registers.tClock.Cycle(1, 4);
}

void CPU::DEC_M(uint8 & address)
{
	BOOL hCarried = HCarryBorrow(address, 0x01, From3Borrow4);
	address -= 0x01;
	Set_Bit(_registers.F, 7, (address == 0));
	Set_Bit(_registers.F, 6, TRUE);
	Set_Bit(_registers.F, 5, hCarried);

	_registers.tClock.Cycle(1, 4);
}

void CPU::CB_RLC_B()
{
	CB_RLC8(_registers.B);
}

void CPU::CB_RLC_C()
{
	CB_RLC8(_registers.C);
}

void CPU::CB_RLC_D()
{
	CB_RLC8(_registers.D);
}

void CPU::CB_RLC_E()
{
	CB_RLC8(_registers.E);
}

void CPU::CB_RLC_H()
{
	CB_RLC8(_registers.H);
}

void CPU::CB_RLC_L()
{
	CB_RLC8(_registers.L);
}

void CPU::CB_RLC_A()
{
	CB_RLC8(_registers.A);
}

void CPU::CB_RRC_B()
{
	CB_RRC8(_registers.B);
}

void CPU::CB_RRC_C()
{
	CB_RRC8(_registers.C);
}

void CPU::CB_RRC_D()
{
	CB_RRC8(_registers.D);
}

void CPU::CB_RRC_E()
{
	CB_RRC8(_registers.E);
}

void CPU::CB_RRC_H()
{
	CB_RRC8(_registers.H);
}

void CPU::CB_RRC_L()
{
	CB_RRC8(_registers.L);
}

void CPU::CB_RRC_A()
{
	CB_RRC8(_registers.A);
}

void CPU::CB_RL_B()
{
	CB_RL8(_registers.B);
}

void CPU::CB_RL_C()
{
	CB_RL8(_registers.C);
}

void CPU::CB_RL_D()
{
	CB_RL8(_registers.D);
}

void CPU::CB_RL_E()
{
	CB_RL8(_registers.E);
}

void CPU::CB_RL_H()
{
	CB_RL8(_registers.H);
}

void CPU::CB_RL_L()
{
	CB_RL8(_registers.L);
}

void CPU::CB_RL_A()
{
	CB_RL8(_registers.A);
}

void CPU::CB_RR_B()
{
	CB_RR8(_registers.B);
}

void CPU::CB_RR_C()
{
	CB_RR8(_registers.C);
}

void CPU::CB_RR_D()
{
	CB_RR8(_registers.D);
}

void CPU::CB_RR_E()
{
	CB_RR8(_registers.E);
}

void CPU::CB_RR_H()
{
	CB_RR8(_registers.H);
}

void CPU::CB_RR_L()
{
	CB_RR8(_registers.L);
}

void CPU::CB_RR_A()
{
	CB_RR8(_registers.A);
}

void CPU::CB_SLA_B()
{
	CB_SLA8(_registers.B);
}

void CPU::CB_SLA_C()
{
	CB_SLA8(_registers.C);
}

void CPU::CB_SLA_D()
{
	CB_SLA8(_registers.D);
}

void CPU::CB_SLA_E()
{
	CB_SLA8(_registers.E);
}

void CPU::CB_SLA_H()
{
	CB_SLA8(_registers.H);
}

void CPU::CB_SLA_L()
{
	CB_SLA8(_registers.L);
}

void CPU::CB_SLA_A()
{
	CB_SLA8(_registers.A);
}

void CPU::CB_SRA_B()
{
	CB_SRA8(_registers.B);
}

void CPU::CB_SRA_C()
{
	CB_SRA8(_registers.C);
}

void CPU::CB_SRA_D()
{
	CB_SRA8(_registers.D);
}

void CPU::CB_SRA_E()
{
	CB_SRA8(_registers.E);
}

void CPU::CB_SRA_H()
{
	CB_SRA8(_registers.H);
}

void CPU::CB_SRA_L()
{
	CB_SRA8(_registers.L);
}

void CPU::CB_SRA_A()
{
	CB_SRA8(_registers.A);
}

void CPU::CB_SWAP_B()
{
	CB_SWAP8(_registers.B);
}

void CPU::CB_SWAP_C()
{
	CB_SWAP8(_registers.C);
}

void CPU::CB_SWAP_D()
{
	CB_SWAP8(_registers.D);
}

void CPU::CB_SWAP_E()
{
	CB_SWAP8(_registers.E);
}

void CPU::CB_SWAP_H()
{
	CB_SWAP8(_registers.H);
}

void CPU::CB_SWAP_L()
{
	CB_SWAP8(_registers.L);
}

void CPU::CB_SWAP_A()
{
	CB_SWAP8(_registers.A);
}

void CPU::CB_SRL_B()
{
	CB_SRL8(_registers.B);
}

void CPU::CB_SRL_C()
{
	CB_SRL8(_registers.C);
}

void CPU::CB_SRL_D()
{
	CB_SRL8(_registers.D);
}

void CPU::CB_SRL_E()
{
	CB_SRL8(_registers.E);
}

void CPU::CB_SRL_H()
{
	CB_SRL8(_registers.H);
}

void CPU::CB_SRL_L()
{
	CB_SRL8(_registers.L);
}

void CPU::CB_SRL_A()
{
	CB_SRL8(_registers.A);
}

void CPU::CB_BIT_0_B()
{
	CB_BIT(0, _registers.B);
}

void CPU::CB_BIT_0_C()
{
	CB_BIT(0, _registers.C);
}

void CPU::CB_BIT_0_D()
{
	CB_BIT(0, _registers.D);
}

void CPU::CB_BIT_0_E()
{
	CB_BIT(0, _registers.E);
}

void CPU::CB_BIT_0_H()
{
	CB_BIT(0, _registers.H);
}

void CPU::CB_BIT_0_L()
{
	CB_BIT(0, _registers.L);
}

void CPU::CB_BIT_0_A()
{
	CB_BIT(0, _registers.A);
}

void CPU::CB_BIT_1_B()
{
	CB_BIT(0, _registers.B);
}

void CPU::CB_BIT_1_C()
{
	CB_BIT(0, _registers.B);
}

void CPU::CB_BIT_1_D()
{
	CB_BIT(0, _registers.B);
}

void CPU::CB_BIT_1_E() { CB_BIT(1, _registers.E); }
void CPU::CB_BIT_1_H() { CB_BIT(1, _registers.H); }
void CPU::CB_BIT_1_L() { CB_BIT(1, _registers.L); }
void CPU::CB_BIT_1_A() { CB_BIT(1, _registers.A); }
void CPU::CB_BIT_2_B() { CB_BIT(2, _registers.B); }
void CPU::CB_BIT_2_C() { CB_BIT(2, _registers.C); }
void CPU::CB_BIT_2_D() { CB_BIT(2, _registers.D); }
void CPU::CB_BIT_2_E() { CB_BIT(2, _registers.E); }
void CPU::CB_BIT_2_H() { CB_BIT(2, _registers.H); }
void CPU::CB_BIT_2_L() { CB_BIT(2, _registers.L); }
void CPU::CB_BIT_2_A() { CB_BIT(2, _registers.A); }
void CPU::CB_BIT_3_B() { CB_BIT(3, _registers.B); }
void CPU::CB_BIT_3_C() { CB_BIT(3, _registers.C); }
void CPU::CB_BIT_3_D() { CB_BIT(3, _registers.D); }
void CPU::CB_BIT_3_E() { CB_BIT(3, _registers.E); }
void CPU::CB_BIT_3_H() { CB_BIT(3, _registers.H); }
void CPU::CB_BIT_3_L() { CB_BIT(3, _registers.L); }
void CPU::CB_BIT_3_A() { CB_BIT(3, _registers.A); }
void CPU::CB_BIT_4_B() { CB_BIT(3, _registers.B); }
void CPU::CB_BIT_4_C() { CB_BIT(4, _registers.C); }
void CPU::CB_BIT_4_D() { CB_BIT(4, _registers.D); }
void CPU::CB_BIT_4_E() { CB_BIT(4, _registers.E); }
void CPU::CB_BIT_4_H() { CB_BIT(4, _registers.H); }
void CPU::CB_BIT_4_L() { CB_BIT(4, _registers.L); }
void CPU::CB_BIT_4_A() { CB_BIT(4, _registers.A); }
void CPU::CB_BIT_5_B() { CB_BIT(5, _registers.B); }
void CPU::CB_BIT_5_C() { CB_BIT(5, _registers.C); }
void CPU::CB_BIT_5_D() { CB_BIT(5, _registers.D); }
void CPU::CB_BIT_5_E() { CB_BIT(5, _registers.E); }
void CPU::CB_BIT_5_H() { CB_BIT(5, _registers.H); }
void CPU::CB_BIT_5_L() { CB_BIT(5, _registers.L); }
void CPU::CB_BIT_5_A() { CB_BIT(5, _registers.A); }
void CPU::CB_BIT_6_B() { CB_BIT(6, _registers.B); }
void CPU::CB_BIT_6_C() { CB_BIT(6, _registers.C); }
void CPU::CB_BIT_6_D() { CB_BIT(6, _registers.D); }
void CPU::CB_BIT_6_E() { CB_BIT(6, _registers.E); }
void CPU::CB_BIT_6_H() { CB_BIT(6, _registers.H); }
void CPU::CB_BIT_6_L() { CB_BIT(6, _registers.L); }
void CPU::CB_BIT_6_A() { CB_BIT(6, _registers.A); }
void CPU::CB_BIT_7_B() { CB_BIT(7, _registers.B); }
void CPU::CB_BIT_7_C() { CB_BIT(7, _registers.C); }
void CPU::CB_BIT_7_D() { CB_BIT(7, _registers.D); }
void CPU::CB_BIT_7_E() { CB_BIT(7, _registers.E); }
void CPU::CB_BIT_7_H() { CB_BIT(7, _registers.H); }
void CPU::CB_BIT_7_L() { CB_BIT(7, _registers.L); }
void CPU::CB_BIT_7_A() { CB_BIT(7, _registers.A); }
void CPU::CB_BIT_0_MM()
{
	Set_Bit(_registers.F, 7, Get_Bit(Read8(_registers.PC), 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, TRUE);
	_registers.tClock.Cycle(2, 16);
}
void CPU::CB_BIT_1_MM()
{
	Set_Bit(_registers.F, 7, Get_Bit(Read8(_registers.PC), 1));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, TRUE);
	_registers.tClock.Cycle(2, 16);
}
void CPU::CB_BIT_2_MM()
{
	Set_Bit(_registers.F, 7, Get_Bit(Read8(_registers.PC), 2));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, TRUE);
	_registers.tClock.Cycle(2, 16);
}
void CPU::CB_BIT_3_MM()
{
	Set_Bit(_registers.F, 7, Get_Bit(Read8(_registers.PC), 3));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, TRUE);
	_registers.tClock.Cycle(2, 16);
}
void CPU::CB_BIT_4_MM()
{
	Set_Bit(_registers.F, 7, Get_Bit(Read8(_registers.PC), 4));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, TRUE);
	_registers.tClock.Cycle(2, 16);
}
void CPU::CB_BIT_5_MM()
{
	Set_Bit(_registers.F, 7, Get_Bit(Read8(_registers.PC), 5));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, TRUE);
	_registers.tClock.Cycle(2, 16);
}
void CPU::CB_BIT_6_MM()
{
	Set_Bit(_registers.F, 7, Get_Bit(Read8(_registers.PC), 6));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, TRUE);
	_registers.tClock.Cycle(2, 16);
}
void CPU::CB_BIT_7_MM()
{
	Set_Bit(_registers.F, 7, Get_Bit(Read8(_registers.PC), 7));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, TRUE);
	_registers.tClock.Cycle(2, 16);
}
void CPU::CB_BIT(uint8 bit, uint8 data)
{
	Set_Bit(_registers.F, 7, Get_Bit(data, bit));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, TRUE);
	_registers.tClock.Cycle(2, 8);
}
void CPU::CB_RES_0_B() { CB_RES(0, _registers.B); }
void CPU::CB_RES_0_C() { CB_RES(0, _registers.C); }
void CPU::CB_RES_0_D() { CB_RES(0, _registers.D); }
void CPU::CB_RES_0_E() { CB_RES(0, _registers.E); }
void CPU::CB_RES_0_H() { CB_RES(0, _registers.H); }
void CPU::CB_RES_0_L() { CB_RES(0, _registers.L); }
void CPU::CB_RES_0_A() { CB_RES(0, _registers.A); }
void CPU::CB_RES_1_B() { CB_RES(1, _registers.B); }
void CPU::CB_RES_1_C() { CB_RES(1, _registers.C); }
void CPU::CB_RES_1_D() { CB_RES(1, _registers.D); }
void CPU::CB_RES_1_E() { CB_RES(1, _registers.E); }
void CPU::CB_RES_1_H() { CB_RES(1, _registers.H); }
void CPU::CB_RES_1_L() { CB_RES(1, _registers.L); }
void CPU::CB_RES_1_A() { CB_RES(1, _registers.A); }
void CPU::CB_RES_2_B() { CB_RES(2, _registers.B); }
void CPU::CB_RES_2_C() { CB_RES(2, _registers.C); }
void CPU::CB_RES_2_D() { CB_RES(2, _registers.D); }
void CPU::CB_RES_2_E() { CB_RES(2, _registers.E); }
void CPU::CB_RES_2_H() { CB_RES(2, _registers.H); }
void CPU::CB_RES_2_L() { CB_RES(2, _registers.L); }
void CPU::CB_RES_2_A() { CB_RES(2, _registers.A); }
void CPU::CB_RES_3_B() { CB_RES(3, _registers.B); }
void CPU::CB_RES_3_C() { CB_RES(3, _registers.C); }
void CPU::CB_RES_3_D() { CB_RES(3, _registers.D); }
void CPU::CB_RES_3_E() { CB_RES(3, _registers.E); }
void CPU::CB_RES_3_H() { CB_RES(3, _registers.H); }
void CPU::CB_RES_3_L() { CB_RES(3, _registers.L); }
void CPU::CB_RES_3_A() { CB_RES(3, _registers.A); }
void CPU::CB_RES_4_B() { CB_RES(4, _registers.B); }
void CPU::CB_RES_4_C() { CB_RES(4, _registers.C); }
void CPU::CB_RES_4_D() { CB_RES(4, _registers.D); }
void CPU::CB_RES_4_E() { CB_RES(4, _registers.E); }
void CPU::CB_RES_4_H() { CB_RES(4, _registers.H); }
void CPU::CB_RES_4_L() { CB_RES(4, _registers.L); }
void CPU::CB_RES_4_A() { CB_RES(4, _registers.A); }
void CPU::CB_RES_5_B() { CB_RES(5, _registers.B); }
void CPU::CB_RES_5_C() { CB_RES(5, _registers.C); }
void CPU::CB_RES_5_D() { CB_RES(5, _registers.D); }
void CPU::CB_RES_5_E() { CB_RES(5, _registers.E); }
void CPU::CB_RES_5_H() { CB_RES(5, _registers.H); }
void CPU::CB_RES_5_L() { CB_RES(5, _registers.L); }
void CPU::CB_RES_5_A() { CB_RES(5, _registers.A); }
void CPU::CB_RES_6_B() { CB_RES(6, _registers.B); }
void CPU::CB_RES_6_C() { CB_RES(6, _registers.C); }
void CPU::CB_RES_6_D() { CB_RES(6, _registers.D); }
void CPU::CB_RES_6_E() { CB_RES(6, _registers.E); }
void CPU::CB_RES_6_H() { CB_RES(6, _registers.H); }
void CPU::CB_RES_6_L() { CB_RES(6, _registers.L); }
void CPU::CB_RES_6_A() { CB_RES(6, _registers.A); }
void CPU::CB_RES_7_B() { CB_RES(7, _registers.B); }
void CPU::CB_RES_7_C() { CB_RES(7, _registers.C); }
void CPU::CB_RES_7_D() { CB_RES(7, _registers.D); }
void CPU::CB_RES_7_E() { CB_RES(7, _registers.E); }
void CPU::CB_RES_7_H() { CB_RES(7, _registers.H); }
void CPU::CB_RES_7_L() { CB_RES(7, _registers.L); }
void CPU::CB_RES_7_A() { CB_RES(7, _registers.A); }
void CPU::CB_RES_0_MM()
{
	uint8 data = Read16(_registers.PC);
	Set_Bit(data, 0, FALSE);
	Write16(_registers.PC, data);
	_registers.tClock.Cycle(2, 16);
}
void CPU::CB_RES_1_MM()
{
	uint8 data = Read16(_registers.PC);
	Set_Bit(data, 1, FALSE);
	Write16(_registers.PC, data);
	_registers.tClock.Cycle(2, 16);
}
void CPU::CB_RES_2_MM()
{
	uint8 data = Read16(_registers.PC);
	Set_Bit(data, 2, FALSE);
	Write16(_registers.PC, data);
	_registers.tClock.Cycle(2, 16);
}
void CPU::CB_RES_3_MM()
{
	uint8 data = Read16(_registers.PC);
	Set_Bit(data, 3, FALSE);
	Write16(_registers.PC, data);
	_registers.tClock.Cycle(2, 16);
}
void CPU::CB_RES_4_MM()
{
	uint8 data = Read16(_registers.PC);
	Set_Bit(data, 4, FALSE);
	Write16(_registers.PC, data);
	_registers.tClock.Cycle(2, 16);
}
void CPU::CB_RES_5_MM()
{
	uint8 data = Read16(_registers.PC);
	Set_Bit(data, 5, FALSE);
	Write16(_registers.PC, data);
	_registers.tClock.Cycle(2, 16);
}
void CPU::CB_RES_6_MM()
{
	uint8 data = Read16(_registers.PC);
	Set_Bit(data, 6, FALSE);
	Write16(_registers.PC, data);
	_registers.tClock.Cycle(2, 16);
}
void CPU::CB_RES_7_MM()
{
	uint8 data = Read16(_registers.PC);
	Set_Bit(data, 7, FALSE);
	Write16(_registers.PC, data);
	_registers.tClock.Cycle(2, 16);
}
void CPU::CB_RES(uint8 bit, uint8 data)
{
	Set_Bit(data, bit, TRUE);
	_registers.tClock.Cycle(2, 8);
}
void CPU::CB_SET_0_B() { CB_SET(0, _registers.B); }
void CPU::CB_SET_0_C() { CB_SET(0, _registers.C); }
void CPU::CB_SET_0_D() { CB_SET(0, _registers.D); }
void CPU::CB_SET_0_E() { CB_SET(0, _registers.E); }
void CPU::CB_SET_0_H() { CB_SET(0, _registers.H); }
void CPU::CB_SET_0_L() { CB_SET(0, _registers.L); }
void CPU::CB_SET_0_A() { CB_SET(0, _registers.A); }
void CPU::CB_SET_1_B() { CB_SET(1, _registers.B); }
void CPU::CB_SET_1_C() { CB_SET(1, _registers.C); }
void CPU::CB_SET_1_D() { CB_SET(1, _registers.D); }
void CPU::CB_SET_1_E() { CB_SET(1, _registers.E); }
void CPU::CB_SET_1_H() { CB_SET(1, _registers.H); }
void CPU::CB_SET_1_L() { CB_SET(1, _registers.L); }
void CPU::CB_SET_1_A() { CB_SET(1, _registers.A); }
void CPU::CB_SET_2_B() { CB_SET(2, _registers.B); }
void CPU::CB_SET_2_C() { CB_SET(2, _registers.C); }
void CPU::CB_SET_2_D() { CB_SET(2, _registers.D); }
void CPU::CB_SET_2_E() { CB_SET(2, _registers.E); }
void CPU::CB_SET_2_H() { CB_SET(2, _registers.H); }
void CPU::CB_SET_2_L() { CB_SET(2, _registers.L); }
void CPU::CB_SET_2_A() { CB_SET(2, _registers.A); }
void CPU::CB_SET_3_B() { CB_SET(3, _registers.B); }
void CPU::CB_SET_3_C() { CB_SET(3, _registers.C); }
void CPU::CB_SET_3_D() { CB_SET(3, _registers.D); }
void CPU::CB_SET_3_E() { CB_SET(3, _registers.E); }
void CPU::CB_SET_3_H() { CB_SET(3, _registers.H); }
void CPU::CB_SET_3_L() { CB_SET(3, _registers.L); }
void CPU::CB_SET_3_A() { CB_SET(3, _registers.A); }
void CPU::CB_SET_4_B() { CB_SET(4, _registers.B); }
void CPU::CB_SET_4_C() { CB_SET(4, _registers.C); }
void CPU::CB_SET_4_D() { CB_SET(4, _registers.D); }
void CPU::CB_SET_4_E() { CB_SET(4, _registers.E); }
void CPU::CB_SET_4_H() { CB_SET(4, _registers.H); }
void CPU::CB_SET_4_L() { CB_SET(4, _registers.L); }
void CPU::CB_SET_4_A() { CB_SET(4, _registers.A); }
void CPU::CB_SET_5_B() { CB_SET(5, _registers.B); }
void CPU::CB_SET_5_C() { CB_SET(5, _registers.C); }
void CPU::CB_SET_5_D() { CB_SET(5, _registers.D); }
void CPU::CB_SET_5_E() { CB_SET(5, _registers.E); }
void CPU::CB_SET_5_H() { CB_SET(5, _registers.H); }
void CPU::CB_SET_5_L() { CB_SET(5, _registers.L); }
void CPU::CB_SET_5_A() { CB_SET(5, _registers.A); }
void CPU::CB_SET_6_B() { CB_SET(6, _registers.B); }
void CPU::CB_SET_6_C() { CB_SET(6, _registers.C); }
void CPU::CB_SET_6_D() { CB_SET(6, _registers.D); }
void CPU::CB_SET_6_E() { CB_SET(6, _registers.E); }
void CPU::CB_SET_6_H() { CB_SET(6, _registers.H); }
void CPU::CB_SET_6_L() { CB_SET(6, _registers.L); }
void CPU::CB_SET_6_A() { CB_SET(6, _registers.A); }
void CPU::CB_SET_7_B() { CB_SET(7, _registers.B); }
void CPU::CB_SET_7_C() { CB_SET(7, _registers.C); }
void CPU::CB_SET_7_D() { CB_SET(7, _registers.D); }
void CPU::CB_SET_7_E() { CB_SET(7, _registers.E); }
void CPU::CB_SET_7_H() { CB_SET(7, _registers.H); }
void CPU::CB_SET_7_L() { CB_SET(7, _registers.L); }
void CPU::CB_SET_7_A() { CB_SET(7, _registers.A); }

void CPU::CB_SET_0_MM()
{
	uint8 data = Read16(_registers.PC);
	Set_Bit(data, 0, TRUE);
	Write16(_registers.PC, data);
	_registers.tClock.Cycle(2, 16);
}

void CPU::CB_SET_1_MM()
{
	uint8 data = Read16(_registers.PC);
	Set_Bit(data, 1, TRUE);
	Write16(_registers.PC, data);
	_registers.tClock.Cycle(2, 16);
}

void CPU::CB_SET_2_MM()
{
	uint8 data = Read16(_registers.PC);
	Set_Bit(data, 2, TRUE);
	Write16(_registers.PC, data);
	_registers.tClock.Cycle(2, 16);
}

void CPU::CB_SET_3_MM()
{
	uint8 data = Read16(_registers.PC);
	Set_Bit(data, 3, TRUE);
	Write16(_registers.PC, data);
	_registers.tClock.Cycle(2, 16);
}

void CPU::CB_SET_4_MM()
{
	uint8 data = Read16(_registers.PC);
	Set_Bit(data, 4, TRUE);
	Write16(_registers.PC, data);
	_registers.tClock.Cycle(2, 16);
}

void CPU::CB_SET_5_MM()
{
	uint8 data = Read16(_registers.PC);
	Set_Bit(data, 5, TRUE);
	Write16(_registers.PC, data);
	_registers.tClock.Cycle(2, 16);
}

void CPU::CB_SET_6_MM()
{
	uint8 data = Read16(_registers.PC);
	Set_Bit(data, 6, TRUE);
	Write16(_registers.PC, data);
	_registers.tClock.Cycle(2, 16);
}

void CPU::CB_SET_7_MM()
{
	uint8 data = Read16(_registers.PC);
	Set_Bit(data, 7, TRUE);
	Write16(_registers.PC, data);
	_registers.tClock.Cycle(2, 16);
}

void CPU::CB_SET(uint8 bit, uint8 data)
{
	Set_Bit(data, bit, TRUE);
	_registers.tClock.Cycle(2, 8);
}

void CPU::CB_RLC8(uint8 & registerRef)
{
	BOOL bit7 = Get_Bit(registerRef, 7);
	registerRef = registerRef << 1;
	Set_Bit(registerRef, 0, bit7);
	Set_Bit(_registers.F, 7, (registerRef == 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, FALSE);
	Set_Bit(_registers.F, 4, bit7);

	_registers.tClock.Cycle(2, 8);
}

void CPU::CB_RLC_MM()
{
	uint8 registerRef = Read8(_registers.PC);

	BOOL bit7 = Get_Bit(registerRef, 7);
	registerRef = registerRef << 1;
	Set_Bit(registerRef, 0, bit7);
	Set_Bit(_registers.F, 7, (registerRef == 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, FALSE);
	Set_Bit(_registers.F, 4, bit7);

	Write8(_registers.PC, registerRef);
	_registers.tClock.Cycle(2, 16);
}

void CPU::CB_RRC8(uint8 & registerRef)
{
	BOOL bit0 = Get_Bit(registerRef, 0);
	registerRef = registerRef >> 1;
	Set_Bit(registerRef, 7, bit0);
	Set_Bit(_registers.F, 7, (registerRef == 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, FALSE);
	Set_Bit(_registers.F, 4, bit0);

	_registers.tClock.Cycle(2, 8);
}

void CPU::CB_RRC_MM()
{
	uint8 registerRef = Read8(_registers.PC);

	BOOL bit0 = Get_Bit(registerRef, 0);
	registerRef = registerRef >> 1;
	Set_Bit(registerRef, 7, bit0);
	Set_Bit(_registers.F, 7, (registerRef == 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, FALSE);
	Set_Bit(_registers.F, 4, bit0);

	Write8(_registers.PC, registerRef);
	_registers.tClock.Cycle(2, 16);
}

void CPU::CB_RL8(uint8 & registerRef)
{
	BOOL C = Get_Bit(_registers.F, 4);
	BOOL bit7 = Get_Bit(registerRef, 7);
	registerRef = registerRef << 1;
	Set_Bit(registerRef, 0, C);
	Set_Bit(_registers.F, 7, (registerRef == 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, FALSE);
	Set_Bit(_registers.F, 4, bit7);

	_registers.tClock.Cycle(2, 8);
}

void CPU::CB_RL_MM()
{
	uint8 registerRef = Read8(_registers.PC);

	BOOL C = Get_Bit(_registers.F, 4);
	BOOL bit7 = Get_Bit(registerRef, 7);
	registerRef = registerRef << 1;
	Set_Bit(registerRef, 0, C);
	Set_Bit(_registers.F, 7, (registerRef == 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, FALSE);
	Set_Bit(_registers.F, 4, bit7);

	Write8(_registers.PC, registerRef);
	_registers.tClock.Cycle(2, 16);
}

void CPU::CB_RR8(uint8 & registerRef)
{
	BOOL C = Get_Bit(_registers.F, 4);
	BOOL bit0 = Get_Bit(registerRef, 0);
	registerRef = registerRef >> 1;
	Set_Bit(registerRef, 7, C);
	Set_Bit(_registers.F, 7, (registerRef == 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, FALSE);
	Set_Bit(_registers.F, 4, bit0);

	_registers.tClock.Cycle(2, 8);
}

void CPU::CB_RR_MM()
{
	uint8 registerRef = Read8(_registers.PC);

	BOOL C = Get_Bit(_registers.F, 4);
	BOOL bit0 = Get_Bit(registerRef, 0);
	registerRef = registerRef >> 1;
	Set_Bit(registerRef, 7, C);
	Set_Bit(_registers.F, 7, (registerRef == 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, FALSE);
	Set_Bit(_registers.F, 4, bit0);

	Write8(_registers.PC, registerRef);
	_registers.tClock.Cycle(2, 16);
}

void CPU::CB_SLA8(uint8 & registerRef)
{
	BOOL bit7 = Get_Bit(registerRef, 7);
	registerRef = registerRef << 1;
	Set_Bit(_registers.F, 7, (registerRef == 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, FALSE);
	Set_Bit(_registers.F, 4, bit7);

	_registers.tClock.Cycle(2, 8);
}

void CPU::CB_SLA_MM()
{
	uint8 registerRef = Read8(_registers.PC);

	BOOL bit7 = Get_Bit(registerRef, 7);
	registerRef = registerRef << 1;
	Set_Bit(_registers.F, 7, (registerRef == 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, FALSE);
	Set_Bit(_registers.F, 4, bit7);

	Write8(_registers.PC, registerRef);
	_registers.tClock.Cycle(2, 16);
}

void CPU::CB_SRA8(uint8 & registerRef)
{
	BOOL bit0 = Get_Bit(registerRef, 0);
	BOOL bit7 = Get_Bit(registerRef, 7);
	registerRef = registerRef >> 1;
	Set_Bit(registerRef, 7, bit7);
	Set_Bit(_registers.F, 7, (registerRef == 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, FALSE);
	Set_Bit(_registers.F, 4, bit0);

	_registers.tClock.Cycle(2, 8);
}

void CPU::CB_SRA_MM()
{
	uint8 registerRef = Read8(_registers.PC);

	BOOL bit0 = Get_Bit(registerRef, 0);
	BOOL bit7 = Get_Bit(registerRef, 7);
	registerRef = registerRef >> 1;
	Set_Bit(registerRef, 7, bit7);
	Set_Bit(_registers.F, 7, (registerRef == 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, FALSE);
	Set_Bit(_registers.F, 4, bit0);

	Write8(_registers.PC, registerRef);
	_registers.tClock.Cycle(2, 16);
}

void CPU::CB_SWAP8(uint8 & registerRef)
{
	uint8 lower = registerRef >> 4;
	uint8 upper = registerRef << 4;
	uint8 val = lower;
	val |= upper;
	registerRef = val;
	Set_Bit(_registers.F, 7, (registerRef == 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, FALSE);
	Set_Bit(_registers.F, 4, FALSE);

	_registers.tClock.Cycle(2, 8);
}
void CPU::CB_SWAP_MM()
{
	uint8 registerRef = Read8(_registers.PC);

	uint8 lower = registerRef >> 4;
	uint8 upper = registerRef << 4;
	uint8 val = lower;
	val |= upper;
	registerRef = val;
	Set_Bit(_registers.F, 7, (registerRef == 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, FALSE);
	Set_Bit(_registers.F, 4, FALSE);

	Write8(_registers.PC, registerRef);
	_registers.tClock.Cycle(2, 16);
}

void CPU::CB_SRL8(uint8 & registerRef)
{
	BOOL bit0 = Get_Bit(registerRef, 0);
	registerRef = registerRef >> 1;
	Set_Bit(_registers.F, 7, (registerRef == 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, FALSE);
	Set_Bit(_registers.F, 4, bit0);

	_registers.tClock.Cycle(2, 8);
}

void CPU::CB_SRL_MM()
{
	uint8 registerRef = Read8(_registers.PC);

	BOOL bit0 = Get_Bit(registerRef, 0);
	registerRef = registerRef >> 1;
	Set_Bit(_registers.F, 7, (registerRef == 0));
	Set_Bit(_registers.F, 6, FALSE);
	Set_Bit(_registers.F, 5, FALSE);
	Set_Bit(_registers.F, 4, bit0);

	Write8(_registers.PC, registerRef);
	_registers.tClock.Cycle(2, 16);
}