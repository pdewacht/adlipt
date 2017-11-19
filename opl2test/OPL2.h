#ifndef OPL2_LIB_H_
#define OPL2_LIB_H_

	#ifndef TRUE
	#define TRUE 1
	#endif

	#ifndef FALSE
	#define FALSE 0
	#endif

	// Operator definitions.
	#define OPERATOR1 false
	#define OPERATOR2 true
	#define MODULATOR false
	#define CARRIER   true

	// Synthesis type definitions.
	#define FREQ_MODULATION false
	#define ADDITIVE_SYNTH  true

	// Drum sounds.
	#define DRUM_BASS   0x10
	#define DRUM_SNARE  0x08
	#define DRUM_TOM    0x04
	#define DRUM_CYMBAL 0x02
	#define DRUM_HI_HAT 0x01

	// Note to frequency mapping.
	#define NOTE_C   0
	#define NOTE_CS  1
	#define NOTE_D   2
	#define NOTE_DS  3
	#define NOTE_E   4
	#define NOTE_F   5
	#define NOTE_FS  6
	#define NOTE_G   7
	#define NOTE_GS  8
	#define NOTE_A   9
	#define NOTE_AS 10
	#define NOTE_B  11


	#include <stdint.h>
	typedef uint8_t byte;
	#define min(a, b) ((a) < (b) ? (a) : (b))
	#define max(a, b) ((a) > (b) ? (a) : (b))
	#define PROGMEM


	class OPL2 {
		public:
			OPL2();
			void init(short lpt_base);
			void reset();
			void write(byte, byte);

			short getNoteFrequency(byte, byte, byte);
			byte getRegister(byte);
			bool getWaveFormSelect();
			bool getTremolo(byte, bool);
			bool getVibrato(byte, bool);
			bool getMaintainSustain(byte, bool);
			bool getEnvelopeScaling(byte, bool);
			byte getMultiplier(byte, bool);
			byte getScalingLevel(byte, bool);
			byte getVolume(byte, bool);
			byte getAttack(byte, bool);
			byte getDecay(byte, bool);
			byte getSustain(byte, bool);
			byte getRelease(byte, bool);
			short getFrequency(byte);
			byte getBlock(byte);
			bool getKeyOn(byte);
			byte getFeedback(byte);
			bool getSynthMode(byte);
			bool getDeepTremolo();
			bool getDeepVibrato();
			bool getPercussion();
			byte getDrums();
			byte getWaveForm(byte, bool);

			void setInstrument(byte, const unsigned char*);
			byte setRegister(byte, byte);
			byte setWaveFormSelect(bool);
			byte setTremolo(byte, bool, bool);
			byte setVibrato(byte, bool, bool);
			byte setMaintainSustain(byte, bool, bool);
			byte setEnvelopeScaling(byte, bool, bool);
			byte setMultiplier(byte, bool, byte);
			byte setScalingLevel(byte, bool, byte);
			byte setVolume(byte, bool, byte);
			byte setAttack(byte, bool, byte);
			byte setDecay(byte, bool, byte);
			byte setSustain(byte, bool, byte);
			byte setRelease(byte, bool, byte);
			byte setFrequency(byte, short);
			byte setBlock(byte, byte);
			byte setKeyOn(byte, bool);
			byte setFeedback(byte, byte);
			byte setSynthMode(byte, bool);
			byte setDeepTremolo(bool);
			byte setDeepVibrato(bool);
			byte setPercussion(bool);
			byte setDrums(bool, bool, bool, bool, bool);
			byte setWaveForm(byte, bool, byte);

		private:
			const static unsigned fIntervals[8];
			const static unsigned notes[12];
			const static byte offset[2][9];
			const static byte drumOffset[6];
			const static byte instrumentBaseRegs[11];
                        short lpt_base;
			byte oplRegisters[256];
			byte getRegisterOffset(byte, bool);
	};
#endif
