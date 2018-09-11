#ifndef CPU_H
#define CPU_H
#pragma once

#include "Defines.h"
#include "MMU.h"
#include "Input.h"

class CPU
{
	typedef void (CPU::*opcode)(void);

	struct Clock {
		uint16 byte_call_cycles; 
		uint16 cpu_cycles;

		void Cycle(uint8 byteCalls, uint8 cycles) {
			byte_call_cycles = byteCalls;
			cpu_cycles = cycles;
		}
	};

	struct Registers {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		struct
		{
			uint8 A, F,	//16-bit Accumilator|Flags (Zero|Operation|Half-Carry|Carry|0|0|0|0)
				B, C,	//16-bit B|C
				D, E,	//16-bit D|E
				H, L	//16-bit H|L
				;
		};
#else
		struct
		{
			uint8 F, A,	//16-bit Accumilator|Flags (Zero|Operation|Half-Carry|Carry|0|0|0|0)
				C, B,	//16-bit B|C
				E, D,	//16-bit D|E
				L, H	//16-bit H|L
				;
		};
#endif

		/*
		uint16 AF = (uint16)(A << 8 & F);
		uint16 BC = (uint16)(B << 8 & C);
		uint16 DE = (uint16)(D << 8 & E);
		uint16 HL = (uint16)(H << 8 & L);
		*/
		uint16 AF() { return ((uint16)(A << 8) + F); }
		uint16 BC() { return ((uint16)(B << 8) + C); }
		uint16 DE() { return ((uint16)(D << 8) + E); }
		uint16 HL() { return ((uint16)(H << 8) + L); }
		void AF(uint16 newValue) { A = newValue >> 8; F = newValue & 0xFF; }
		void BC(uint16 newValue) { B = newValue >> 8; C = newValue & 0xFF; }
		void DE(uint16 newValue) { D = newValue >> 8; E = newValue & 0xFF; }
		void HL(uint16 newValue) { H = newValue >> 8; L = newValue & 0xFF; }

		uint16 PC,	//Program Counter
			SP		//Stack Pointer
		;

		Clock tClock; //temporary clock
	};

private:
	Clock		_clock;
	Registers	_registers;
	MMU* _mmu;
	BOOL _hasSetPC;
	opcode _opcodeTable[0xFF];
	opcode _cbOpcodeTable[0xFF];
	
public:
	
	void math8(BOOL method, BOOL C, uint16 dataChange, uint8 & dataRef, uint8 & f, BOOL & hCarried, BOOL & cCarried);
	CPU(MMU* mmu);
	~CPU();

	//sequence
	void Reset();
	uint16 GetCycles();
	uint16 ResetCycles();
	bool Run();

private:
#pragma region Custom Functions
	//base
	uint8 Read8(uint16 address);
	uint16 Read16(uint16 address);
	void Write8(uint16 address, uint8 value);
	void Write16(uint16 address, uint16 value);

	uint8 POP8_SP();
	uint16 POP16_SP();
	void PUSH8_SP(uint8 value);
	void PUSH16_SP(uint16 value);


	//flag un-/sets
	void Set_Z(BOOL value); //Zero Flag
	void Set_N(BOOL value); //Subtract Flag
	void Set_H(BOOL value); //Half Carry Flag
	void Set_C(BOOL value); //Carry Flag
#pragma endregion

	//Z80 opcodes
	void NOP();			//0x00, or unused
	void LD_BC_RR();	//0x01 LD BC,d16
	void LD_BC_A();		//0x02 LD (BC),A
	void INC_BC();		//0x03 INC BC
	void INC_B();		//0x04 INC B
	void DEC_B();		//0x05 DEC B
	void LD_B_R();		//0x06 LD B,d8
	void RL_CA();		//0x07
	void LD_RR_SP();	//0x08
	void ADD_HL_BC();	//0x09
	void LD_A_BC();		//0x0A
	void DEC_BC();		//0x0B
	void INC_C();		//0x0C
	void DEC_C();		//0x0D
	void LD_C_R();		//0x0E
	void RR_CA();		//0x0F

	void STOP();		//0x10
	void LD_DE_RR();	//0x11
	void LD_DE_A();		//0x12
	void INC_DE();		//0x13
	void INC_D();		//0x14
	void DEC_D();		//0x15
	void LD_D_R();		//0x16
	void RL_A();		//0x17
	void JR_R();		//0x18
	void ADD_HL_DE();	//0x19
	void LD_A_DE();		//0x1A
	void DEC_DE();		//0x1B
	void INC_E();		//0x1C
	void DEC_E();		//0x1D
	void LD_E_R();		//0x1E
	void RR_A();		//0x1F

	void JR_NZ_R();		//0x20
	void LD_HL_RR();	//0x21
	void LD_MM_PLUS_A();//0x22
	void INC_HL();		//0x23
	void INC_H();		//0x24
	void DEC_H();		//0x25
	void LD_H_R();		//0x26
	void DA_A();		//0x27
	void JR_Z_R();		//0x28
	void ADD_HL_HL();	//0x29
	void LD_A_MM_PLUS();//0x2A
	void DEC_HL();		//0x2B
	void INC_L();		//0x2C
	void DEC_L();		//0x2D
	void LD_L_R();		//0x2E
	void CPL();			//0x2F

	void JR_NC_R();		//0x30
	void LD_SP_RR();	//0x31
	void LD_MM_MIN_A();	//0x32
	void INC_SP();		//0x33
	void INC_MM();		//0x34
	void DEC_MM();		//0x35
	void LD_MM_R();		//0x36
	void SCF();		//0x37
	void JR_C_R();		//0x38
	void ADD_HL_SP();	//0x39
	void LD_A_MM_MIN();	//0x3A
	void DEC_SP();		//0x3B
	void INC_A();		//0x3C
	void DEC_A();		//0x3D
	void LD_A_R();		//0x3E
	void CCF();		//0x3F

	void LD_B_B();		//0x40
	void LD_B_C();		//0x41
	void LD_B_D();		//0x42
	void LD_B_E();		//0x43
	void LD_B_H();		//0x44
	void LD_B_L();		//0x45
	void LD_B_MM();		//0x46
	void LD_B_A();		//0x47
	void LD_C_B();		//0x48
	void LD_C_C();		//0x49
	void LD_C_D();		//0x4A
	void LD_C_E();		//0x4B
	void LD_C_H();		//0x4C
	void LD_C_L();		//0x4D
	void LD_C_MM();		//0x4E
	void LD_C_A();		//0x4F

	void LD_D_B();		//0x50
	void LD_D_C();		//0x51
	void LD_D_D();		//0x52
	void LD_D_E();		//0x53
	void LD_D_H();		//0x54
	void LD_D_L();		//0x55
	void LD_D_MM();		//0x56
	void LD_D_A();		//0x57
	void LD_E_B();		//0x58
	void LD_E_C();		//0x59
	void LD_E_D();		//0x5A
	void LD_E_E();		//0x5B
	void LD_E_H();		//0x5C
	void LD_E_L();		//0x5D
	void LD_E_MM();		//0x5E
	void LD_E_A();		//0x5F

	void LD_H_B();		//0x60
	void LD_H_C();		//0x61
	void LD_H_D();		//0x62
	void LD_H_E();		//0x63
	void LD_H_H();		//0x64
	void LD_H_L();		//0x65
	void LD_H_MM();		//0x66
	void LD_H_A();		//0x67
	void LD_L_B();		//0x68
	void LD_L_C();		//0x69
	void LD_L_D();		//0x6A
	void LD_L_E();		//0x6B
	void LD_L_H();		//0x6C
	void LD_L_L();		//0x6D
	void LD_L_MM();		//0x6E
	void LD_L_A();		//0x6F

	void LD_MM_B();		//0x70
	void LD_MM_C();		//0x71
	void LD_MM_D();		//0x72
	void LD_MM_E();		//0x73
	void LD_MM_H();		//0x74
	void LD_MM_L();		//0x75
	void HALT();		//0x76
	void LD_MM_A();		//0x77
	void LD_A_B();		//0x78
	void LD_A_C();		//0x79
	void LD_A_D();		//0x7A
	void LD_A_E();		//0x7B
	void LD_A_H();		//0x7C
	void LD_A_L();		//0x7D
	void LD_A_MM();		//0x7E
	void LD_A_A();		//0x7F
	
	
	void ADD_A_B();		//0x80
	void ADD_A_C();		//0x81
	void ADD_A_D();		//0x82
	void ADD_A_E();		//0x83
	void ADD_A_H();		//0x84
	void ADD_A_L();		//0x85
	void ADD_A_MM();	//0x86
	void ADD_A_A();		//0x87
	void ADD_A_M(uint8 value);	//0x80-0x87 (except '0x86')
	void ADC_A_B();		//0x88
	void ADC_A_C();		//0x89
	void ADC_A_D();		//0x8A
	void ADC_A_E();		//0x8B
	void ADC_A_H();		//0x8C
	void ADC_A_L();		//0x8D
	void ADC_A_MM();	//0x8E
	void ADC_A_A();		//0x8F
	void ADC_A_M(uint8 value);	//0x88-0x8F (except '0x8E')

	void SUB_A_B();		//0x90
	void SUB_A_C();		//0x91
	void SUB_A_D();		//0x92
	void SUB_A_E();		//0x93
	void SUB_A_H();		//0x94
	void SUB_A_L();		//0x95
	void SUB_A_MM();	//0x96
	void SUB_A_A();		//0x97
	void SUB_A_M(uint8 value);	//0x90-0x97 (except '0x96')
	void SBC_A_B();		//0x98
	void SBC_A_C();		//0x99
	void SBC_A_D();		//0x9A
	void SBC_A_E();		//0x9B
	void SBC_A_H();		//0x9C
	void SBC_A_L();		//0x9D
	void SBC_A_MM();	//0x9E
	void SBC_A_A();		//0x9F
	void SBC_A_M(uint8 value);	//0x98-0x9F (except '0x9E')

	void AND_B();		//0xA0
	void AND_C();		//0xA1
	void AND_D();		//0xA2
	void AND_E();		//0xA3
	void AND_H();		//0xA4
	void AND_L();		//0xA5
	void AND_MM();		//0xA6
	void AND_A();		//0xA7
	void AND_M(uint8 value);	//0xA0-0xA7 (except '0xA6')
	void XOR_B();		//0xA8
	void XOR_C();		//0xA9
	void XOR_D();		//0xAA
	void XOR_E();		//0xAB
	void XOR_H();		//0xAC
	void XOR_L();		//0xAD
	void XOR_MM();		//0xAE
	void XOR_A();		//0xAF
	void XOR_M(uint8 value);	//0xA8-0xAF (except '0xAE')

	
	void OR_B();		//0xB0
	void OR_C();		//0xB1
	void OR_D();		//0xB2
	void OR_E();		//0xB3
	void OR_H();		//0xB4
	void OR_L();		//0xB5
	void OR_MM();		//0xB6
	void OR_A();		//0xB7
	void OR_M(uint8 value);		//0xB0-0xB7 (except '0xB6')
	void CP_B();		//0xB8
	void CP_C();		//0xB9
	void CP_D();		//0xBA
	void CP_E();		//0xBB
	void CP_H();		//0xBC
	void CP_L();		//0xBD
	void CP_MM();		//0xBE
	void CP_A();		//0xBF
	void CP_M(uint8 value);		//0xB8-0xBF (except '0xBE')

	void RET_NZ();		//0xC0
	void POP_BC();		//0xC1
	void JP_NZ_RR();	//0xC2
	void JP_RR();		//0xC3
	void CALL_NZ_RR();	//0xC4
	void PUSH_BC();		//0xC5
	void ADD_A_R();		//0xC6
	void RST_00H();		//0xC7
	void RET_Z();		//0xC8
	void RET();			//0xC9
	void JP_Z_RR();		//0xCA
	void PREFIX_CB();	//0xCB
	void CALL_Z_RR();	//0xCC
	void CALL_RR();		//0xCD
	void ADC_A_R();		//0xCE
	void RST_08H();		//0xCF

	void RET_NC();		//0xD0
	void POP_DE();		//0xD1
	void JP_NC_RR();	//0xD2
	void CALL_NC_RR();	//0xD4
	void PUSH_DE();		//0xD5
	void SUB_R();		//0xD6
	void RST_10H();		//0xD7
	void RET_C();		//0xD8
	void RETI();		//0xD9
	void JP_C_RR();		//0xDA
	void CALL_C_RR();	//0xDC
	void SBC_A_R();		//0xDE
	void RST_18H();		//0xDF

	void LDH_R_A();		//0xE0
	void POP_HL();		//0xE1
	void LD_C_A2();		//0xE2
	void PUSH_HL();		//0xE5
	void AND_R();		//0xE6
	void RST_20H();		//0xE7
	void ADD_SP_R();	//0xE8
	void JP_MM();		//0xE9
	void LD_RR_A();		//0xEA
	void XOR_R();		//0xEE
	void RST_28H();		//0xEF

	void LDH_A_R();		//0xF0
	void POP_AF();		//0xF1
	void LD_A_C2();		//0xF2
	void DI();			//0xF3
	void PUSH_AF();		//0xF5
	void OR_R();		//0xF6
	void RST_30H();		//0xF7
	void LD_HL_SPandR();//0xF8
	void LD_SP_HL();	//0xF9
	void LD_A_RR();		//0xFA
	void EI();			//0xFB
	void CP_R();		//0xFE
	void RST_38H();		//0xFF

	void LD_M_M(uint8& address, uint8 value);
	void INC_M(uint8& address);
	void DEC_M(uint8& address);

	//CB
	void CB_RLC_B();	//0xCB00
	void CB_RLC_C();	//0xCB01
	void CB_RLC_D();	//0xCB02
	void CB_RLC_E();	//0xCB03
	void CB_RLC_H();	//0xCB04
	void CB_RLC_L();	//0xCB05
	void CB_RLC_MM();	//0xCB06
	void CB_RLC_A();	//0xCB07
	void CB_RRC_B();	//0xCB08
	void CB_RRC_C();	//0xCB09
	void CB_RRC_D();	//0xCB0A
	void CB_RRC_E();	//0xCB0B
	void CB_RRC_H();	//0xCB0C
	void CB_RRC_L();	//0xCB0D
	void CB_RRC_MM();	//0xCB0E
	void CB_RRC_A();	//0xCB0F

	void CB_RL_B();		//0xCB10
	void CB_RL_C();		//0xCB11
	void CB_RL_D();		//0xCB12
	void CB_RL_E();		//0xCB13
	void CB_RL_H();		//0xCB14
	void CB_RL_L();		//0xCB15
	void CB_RL_MM();	//0xCB16
	void CB_RL_A();		//0xCB17
	void CB_RR_B();		//0xCB18
	void CB_RR_C();		//0xCB19
	void CB_RR_D();		//0xCB1A
	void CB_RR_E();		//0xCB1B
	void CB_RR_H();		//0xCB1C
	void CB_RR_L();		//0xCB1D
	void CB_RR_MM();	//0xCB1E
	void CB_RR_A();		//0xCB1F

	void CB_SLA_B();	//0xCB20
	void CB_SLA_C();	//0xCB21
	void CB_SLA_D();	//0xCB22
	void CB_SLA_E();	//0xCB23
	void CB_SLA_H();	//0xCB24
	void CB_SLA_L();	//0xCB25
	void CB_SLA_MM();	//0xCB26
	void CB_SLA_A();	//0xCB27
	void CB_SRA_B();	//0xCB28
	void CB_SRA_C();	//0xCB29
	void CB_SRA_D();	//0xCB2A
	void CB_SRA_E();	//0xCB2B
	void CB_SRA_H();	//0xCB2C
	void CB_SRA_L();	//0xCB2D
	void CB_SRA_MM();	//0xCB2E
	void CB_SRA_A();	//0xCB2F

	void CB_SWAP_B();	//0xCB30
	void CB_SWAP_C();	//0xCB31
	void CB_SWAP_D();	//0xCB32
	void CB_SWAP_E();	//0xCB33
	void CB_SWAP_H();	//0xCB34
	void CB_SWAP_L();	//0xCB35
	void CB_SWAP_MM();	//0xCB36
	void CB_SWAP_A();	//0xCB37
	void CB_SRL_B();	//0xCB38
	void CB_SRL_C();	//0xCB39
	void CB_SRL_D();	//0xCB3A
	void CB_SRL_E();	//0xCB3B
	void CB_SRL_H();	//0xCB3C
	void CB_SRL_L();	//0xCB3D
	void CB_SRL_MM();	//0xCB3E
	void DMATransfer();
	void CB_SRL_A();	//0xCB3F

	void CB_BIT_0_B();	//0XCB40
	void CB_BIT_0_C();	//0XCB41
	void CB_BIT_0_D();	//0XCB42
	void CB_BIT_0_E();	//0XCB43
	void CB_BIT_0_H();	//0XCB44
	void CB_BIT_0_L();	//0XCB45
	void CB_BIT_0_A();	//0XCB47
	void CB_BIT_1_B();	//0XCB48
	void CB_BIT_1_C();	//0XCB49
	void CB_BIT_1_D();	//0XCB4A
	void CB_BIT_1_E();	//0XCB4B
	void CB_BIT_1_H();	//0XCB4C
	void CB_BIT_1_L();	//0XCB4D
	void CB_BIT_1_A();	//0XCB4F
	void CB_BIT_2_B();	//0XCB50
	void CB_BIT_2_C();	//0XCB51
	void CB_BIT_2_D();	//0XCB52
	void CB_BIT_2_E();	//0XCB53
	void CB_BIT_2_H();	//0XCB54
	void CB_BIT_2_L();	//0XCB55
	void CB_BIT_2_A();	//0XCB57
	void CB_BIT_3_B();	//0XCB58
	void CB_BIT_3_C();	//0XCB59
	void CB_BIT_3_D();	//0XCB5A
	void CB_BIT_3_E();	//0XCB5B
	void CB_BIT_3_H();	//0XCB5C
	void CB_BIT_3_L();	//0XCB5D
	void CB_BIT_3_A();	//0XCB5F
	void CB_BIT_4_B();	//0XCB60
	void CB_BIT_4_C();	//0XCB61
	void CB_BIT_4_D();	//0XCB62
	void CB_BIT_4_E();	//0XCB63
	void CB_BIT_4_H();	//0XCB64
	void CB_BIT_4_L();	//0XCB65
	void CB_BIT_4_A();	//0XCB67
	void CB_BIT_5_B();	//0XCB68
	void CB_BIT_5_C();	//0XCB69
	void CB_BIT_5_D();	//0XCB6A
	void CB_BIT_5_E();	//0XCB6B
	void CB_BIT_5_H();	//0XCB6C
	void CB_BIT_5_L();	//0XCB6D
	void CB_BIT_5_A();	//0XCB6F
	void CB_BIT_6_B();	//0XCB70
	void CB_BIT_6_C();	//0XCB71
	void CB_BIT_6_D();	//0XCB72
	void CB_BIT_6_E();	//0XCB73
	void CB_BIT_6_H();	//0XCB74
	void CB_BIT_6_L();	//0XCB75
	void CB_BIT_6_A();	//0XCB77
	void CB_BIT_7_B();	//0XCB78
	void CB_BIT_7_C();	//0XCB79
	void CB_BIT_7_D();	//0XCB7A
	void CB_BIT_7_E();	//0XCB7B
	void CB_BIT_7_H();	//0XCB7C
	void CB_BIT_7_L();	//0XCB7D
	void CB_BIT_7_A();	//0XCB7F

	void CB_BIT_0_MM();	//0xCB46
	void CB_BIT_1_MM();	//0xCB4E
	void CB_BIT_2_MM();	//0xCB56
	void CB_BIT_3_MM();	//0xCB5E
	void CB_BIT_4_MM();	//0xCB66
	void CB_BIT_5_MM();	//0xCB6E
	void CB_BIT_6_MM();	//0xCB76
	void CB_BIT_7_MM();	//0xCB7E

	void CB_BIT(uint8 bit, uint8 data);

	void CB_RES_0_B();	//0XCB80
	void CB_RES_0_C();	//0XCB81
	void CB_RES_0_D();	//0XCB82
	void CB_RES_0_E();	//0XCB83
	void CB_RES_0_H();	//0XCB84
	void CB_RES_0_L();	//0XCB85
	void CB_RES_0_A();	//0XCB87
	void CB_RES_1_B();	//0XCB88
	void CB_RES_1_C();	//0XCB89
	void CB_RES_1_D();	//0XCB8A
	void CB_RES_1_E();	//0XCB8B
	void CB_RES_1_H();	//0XCB8C
	void CB_RES_1_L();	//0XCB8D
	void CB_RES_1_A();	//0XCB8F
	void CB_RES_2_B();	//0XCB90
	void CB_RES_2_C();	//0XCB91
	void CB_RES_2_D();	//0XCB92
	void CB_RES_2_E();	//0XCB93
	void CB_RES_2_H();	//0XCB94
	void CB_RES_2_L();	//0XCB95
	void CB_RES_2_A();	//0XCB97
	void CB_RES_3_B();	//0XCB98
	void CB_RES_3_C();	//0XCB99
	void CB_RES_3_D();	//0XCB9A
	void CB_RES_3_E();	//0XCB9B
	void CB_RES_3_H();	//0XCB9C
	void CB_RES_3_L();	//0XCB9D
	void CB_RES_3_A();	//0XCB9F
	void CB_RES_4_B();	//0XCBA0
	void CB_RES_4_C();	//0XCBA1
	void CB_RES_4_D();	//0XCBA2
	void CB_RES_4_E();	//0XCBA3
	void CB_RES_4_H();	//0XCBA4
	void CB_RES_4_L();	//0XCBA5
	void CB_RES_4_A();	//0XCBA7
	void CB_RES_5_B();	//0XCBA8
	void CB_RES_5_C();	//0XCBA9
	void CB_RES_5_D();	//0XCBAA
	void CB_RES_5_E();	//0XCBAB
	void CB_RES_5_H();	//0XCBAC
	void CB_RES_5_L();	//0XCBAD
	void CB_RES_5_A();	//0XCBAF
	void CB_RES_6_B();	//0XCBB0
	void CB_RES_6_C();	//0XCBB1
	void CB_RES_6_D();	//0XCBB2
	void CB_RES_6_E();	//0XCBB3
	void CB_RES_6_H();	//0XCBB4
	void CB_RES_6_L();	//0XCBB5
	void CB_RES_6_A();	//0XCBB7
	void CB_RES_7_B();	//0XCBB8
	void CB_RES_7_C();	//0XCBB9
	void CB_RES_7_D();	//0XCBBA
	void CB_RES_7_E();	//0XCBBB
	void CB_RES_7_H();	//0XCBBC
	void CB_RES_7_L();	//0XCBBD
	void CB_RES_7_A();	//0XCBBF

	void CB_RES_0_MM();	//0xCB86
	void CB_RES_1_MM();	//0xCB8E
	void CB_RES_2_MM();	//0xCB96
	void CB_RES_3_MM();	//0xCB9E
	void CB_RES_4_MM();	//0xCBA6
	void CB_RES_5_MM();	//0xCBAE
	void CB_RES_6_MM();	//0xCBB6
	void CB_RES_7_MM();	//0xCBBE

	void CB_RES(uint8 bit, uint8 data);

	void CB_SET_0_B();	//0XCBC0
	void CB_SET_0_C();	//0XCBC1
	void CB_SET_0_D();	//0XCBC2
	void CB_SET_0_E();	//0XCBC3
	void CB_SET_0_H();	//0XCBC4
	void CB_SET_0_L();	//0XCBC5
	void CB_SET_0_A();	//0XCBC7
	void CB_SET_1_B();	//0XCBC8
	void CB_SET_1_C();	//0XCBC9
	void CB_SET_1_D();	//0XCBCA
	void CB_SET_1_E();	//0XCBCB
	void CB_SET_1_H();	//0XCBCC
	void CB_SET_1_L();	//0XCBCD
	void CB_SET_1_A();	//0XCBCF
	void CB_SET_2_B();	//0XCBD0
	void CB_SET_2_C();	//0XCBD1
	void CB_SET_2_D();	//0XCBD2
	void CB_SET_2_E();	//0XCBD3
	void CB_SET_2_H();	//0XCBD4
	void CB_SET_2_L();	//0XCBD5
	void CB_SET_2_A();	//0XCBD7
	void CB_SET_3_B();	//0XCBD8
	void CB_SET_3_C();	//0XCBD9
	void CB_SET_3_D();	//0XCBDA
	void CB_SET_3_E();	//0XCBDB
	void CB_SET_3_H();	//0XCBDC
	void CB_SET_3_L();	//0XCBDD
	void CB_SET_3_A();	//0XCBDF
	void CB_SET_4_B();	//0XCBE0
	void CB_SET_4_C();	//0XCBE1
	void CB_SET_4_D();	//0XCBE2
	void CB_SET_4_E();	//0XCBE3
	void CB_SET_4_H();	//0XCBE4
	void CB_SET_4_L();	//0XCBE5
	void CB_SET_4_A();	//0XCBE7
	void CB_SET_5_B();	//0XCBE8
	void CB_SET_5_C();	//0XCBE9
	void CB_SET_5_D();	//0XCBEA
	void CB_SET_5_E();	//0XCBEB
	void CB_SET_5_H();	//0XCBEC
	void CB_SET_5_L();	//0XCBED
	void CB_SET_5_A();	//0XCBEF
	void CB_SET_6_B();	//0XCBF0
	void CB_SET_6_C();	//0XCBF1
	void CB_SET_6_D();	//0XCBF2
	void CB_SET_6_E();	//0XCBF3
	void CB_SET_6_H();	//0XCBF4
	void CB_SET_6_L();	//0XCBF5
	void CB_SET_6_A();	//0XCBF7
	void CB_SET_7_B();	//0XCBF8
	void CB_SET_7_C();	//0XCBF9
	void CB_SET_7_D();	//0XCBFA
	void CB_SET_7_E();	//0XCBFB
	void CB_SET_7_H();	//0XCBFC
	void CB_SET_7_L();	//0XCBFD
	void CB_SET_7_A();	//0XCBFF

	void CB_SET_0_MM();	//0xCBC6
	void CB_SET_1_MM();	//0xCBCE
	void CB_SET_2_MM();	//0xCBD6
	void CB_SET_3_MM();	//0xCBDE
	void CB_SET_4_MM();	//0xCBE6
	void CB_SET_5_MM();	//0xCBEE
	void CB_SET_6_MM();	//0xCBF6
	void CB_SET_7_MM();	//0xCBFE

	void CB_SET(uint8 bit, uint8 data);

	void CB_RLC8(uint8& registerRef);		//0xCB00-0xCB07 (except '0xCB06')

	void CB_RRC8(uint8& registerRef);		//0xCB08-0xCB0F (except '0xCB0E')

	void CB_RL8(uint8& registerRef);		//0xCB10-0xCB17 (except '0xCB16')

	void CB_RR8(uint8& registerRef);		//0xCB18-0xCB1F (except '0xCB1E')

	void CB_SLA8(uint8& registerRef);		//0xCB20-0xCB27 (except '0xCB26')

	void CB_SRA8(uint8& registerRef);		//0xCB28-0xCB2F (except '0xCB2E')

	void CB_SWAP8(uint8& registerRef);		//0xCB30-0xCB37 (except '0xCB36')

	void CB_SRL8(uint8& registerRef);		//0xCB38-0xCB3F (except '0xCB3E')

	void CB_BIT8(uint8 bit, uint8& registerRef);	//0xCB40-0xCB7F (except '0xCB46|0xCB4E|0xCB56|0xCB5E|0xCB66|0xCB6E|0xCB76|0xCB7E')
	void CB_BIT16(uint8 bit, uint16& registerRef);	//0xCB46|0xCB4E|0xCB56|0xCB5E|0xCB66|0xCB6E|0xCB76|0xCB7E

	void CB_RES8(uint8 bit, uint8& registerRef);	//0xCB80-0xCBBF (except '0xCB86|0xCB8E|0xCB96|0xCB9E|0xCBA6|0xCBAE|0xCBB6|0xCBBE')
	void CB_RES16(uint8 bit, uint16& registerRef);	//0xCB86|0xCB8E|0xCB96|0xCB9E|0xCBA6|0xCBAE|0xCBB6|0xCBBE

	void CB_SET8(uint8 bit, uint8& registerRef);	//0xCBC0-0xCBFF (except '0xCBC6|0xCBCE|0xCBD6|0xCBDE|0xCBE6|0xCBEE|0xCBF6|0xCBFE')
	void CB_SET16(uint8 bit, uint16& registerRef);	//0xCBC6|0xCBCE|0xCBD6|0xCBDE|0xCBE6|0xCBEE|0xCBF6|0xCBFE
};

#endif // !CPU_H

