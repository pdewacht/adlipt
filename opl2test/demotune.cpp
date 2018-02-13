/**
 * This is a demonstration sketch for the OPL2 Audio Board. It demonstrates playing a little tune on 3 channels using
 * a piano from the MIDI instrument defenitions.
 *
 * Code by Maarten Janssen (maarten@cheerful.nl) 2016-04-13
 * Most recent version of the library can be found at my GitHub: https://github.com/DhrBaksteen/ArduinoOPL2
 */

/**
 * Hacked for a OPL2LPT test program by pdewacht@gmail.com.
 */

#include "OPL2.h"
#include "midi_instruments.h"
#include "timer.h"

void parseTune(struct Tune *tune);
void parseNote(struct Tune *tune);
int parseDuration(struct Tune *tune);
int parseNumber(struct Tune *tune);

const byte noteDefs[21] = {
  NOTE_A, NOTE_A - 1, NOTE_A + 1,
  NOTE_B, NOTE_B - 1, NOTE_B + 1,
  NOTE_C, 0/*FIXME*/, NOTE_C + 1,
  NOTE_D, NOTE_D - 1, NOTE_D + 1,
  NOTE_E, NOTE_E - 1, NOTE_E + 1,
  NOTE_F, NOTE_F - 1, NOTE_F + 1,
  NOTE_G, NOTE_G - 1, NOTE_G + 1
};

unsigned tempo;

struct Tune {
  const char *data;
  int channel;
  int octave;
  int noteDuration;
  int noteLength;
  unsigned long nextNoteTime;
  unsigned long releaseTime;
  int index;
};

const char *tuneData[3] = {
  "t150m200o5l8egredgrdcerc<b>er<ba>a<a>agdefefedr4.regredgrdcerc<b>er<ba>a<a>agdedcr4.c<g>cea>cr<ag>cr<gfarfearedgrdcfrc<bagab>cdfegredgrdcerc<b>er<ba>a<a>agdedcr4.cro3c2",
  "m85o3l8crer<br>dr<ar>cr<grbrfr>cr<grbr>crer<gb>dgcrer<br>dr<ar>cr<grbrfr>cr<grbr>ceger4.rfrafergedrfdcrec<br>d<bar>c<agrgd<gr4.o4crer<br>dr<ar>cr<grbrfr>cr<grbr>cege",
  "m85o3l8r4gr4.gr4.er4.err4fr4.gr4.gr4.grr4gr4.er4.er4.frr4gr4>ccr4ccr4<aarraar4ggr4ffr4.ro4gab>dr4.r<gr4.gr4.err4er4.fr4.g"
};


OPL2 opl2;
struct Tune music[3];


void music_setup() {
  tempo = 120;

  // Initialize 3 channels of the tune.
  for (int i = 0; i < 3; i ++) {
    struct Tune channel;
    channel.data = tuneData[i];
    channel.channel = i;
    channel.octave = 4;
    channel.noteDuration = 4;
    channel.noteLength = 85;
    channel.releaseTime = 0;
    channel.nextNoteTime = timer_get();
    channel.index = 0;
    music[i] = channel;
  }

  // Setup channels 0, 1 and 2.
  opl2.setInstrument(0, PIANO1);
  opl2.setBlock     (0, 5);
  opl2.setInstrument(1, PIANO1);
  opl2.setBlock     (1, 4);
  opl2.setInstrument(2, PIANO1);
  opl2.setBlock     (2, 4);

  timer_setup(100);
}

void music_shutdown() {
  timer_shutdown();
  for (int i = 0; i < 3; i ++) {
    if (opl2.getKeyOn(music[i].channel)) {
      opl2.setKeyOn(music[i].channel, false);
    }
  }
}

void music_loop() {
  bool busy = false;
  for (int i = 0; i < 3; i ++) {
    if (timer_get() >= music[i].releaseTime && opl2.getKeyOn(music[i].channel)) {
      opl2.setKeyOn(music[i].channel, false);
    }
    if (timer_get() >= music[i].nextNoteTime && music[i].data[music[i].index] != 0) {
      parseTune(&music[i]);
    }
    if (music[i].data[music[i].index] != 0 || timer_get() < music[i].nextNoteTime) {
      busy = true;
    }
  }

  if (!busy) {
    for (int i = 0; i < 3; i ++) {
      music[i].index = 0;
      music[i].nextNoteTime = timer_get() + 500;
    }
  }

  hlt();
}


void parseTune(struct Tune *tune) {
  while (tune->data[tune->index] != 0) {

    // Decrease octave if greater than 1.
    if (tune->data[tune->index] == '<' && tune->octave > 1) {
      tune->octave --;

    // Increase octave if less than 7.
    } else if (tune->data[tune->index] == '>' && tune->octave < 7) {
      tune->octave ++;

    // Set octave.
    } else if (tune->data[tune->index] == 'o' && tune->data[tune->index + 1] >= '1' && tune->data[tune->index + 1] <= '7') {
      tune->octave = tune->data[++ tune->index] - 48;

    // Set default note duration.
    } else if (tune->data[tune->index] == 'l') {
      tune->index ++;
      int duration = parseNumber(tune);
      if (duration != 0) tune->noteDuration = duration;

    // Set note length in percent.
    } else if (tune->data[tune->index] == 'm') {
      tune->index ++;
      tune->noteLength = parseNumber(tune);

    // Set song tempo.
    } else if (tune->data[tune->index] == 't') {
      tune->index ++;
      tempo = parseNumber(tune);

    // Pause.
    } else if (tune->data[tune->index] == 'p' || tune->data[tune->index] == 'r') {
      tune->index ++;
      tune->nextNoteTime = timer_get() + parseDuration(tune);
      break;

    // Next character is a note A..G so play it.
    } else if (tune->data[tune->index] >= 'a' && tune->data[tune->index] <= 'g') {
      parseNote(tune);
      break;
    }

    tune->index ++;
  }
}


void parseNote(struct Tune *tune) {
  // Get index of note in base frequenct table.
  unsigned char note = (tune->data[tune->index ++] - 97) * 3;
  if (tune->data[tune->index] == '-') {
    note ++;
    tune->index ++;
  } else if (tune->data[tune->index] == '+') {
    note += 2;
    tune->index ++;
  }

  // Get duration, set delay and play note.
  int duration = parseDuration(tune);
  tune->nextNoteTime = timer_get() + duration;
  tune->releaseTime = timer_get() + ((long)duration * tune->noteLength / 100);

  opl2.setKeyOn(tune->channel, false);
  opl2.setFrequency(tune->channel, opl2.getNoteFrequency(tune->channel, tune->octave, noteDefs[note]));
  opl2.setKeyOn(tune->channel, true);
}


int parseDuration(struct Tune *tune) {
  unsigned char duration = parseNumber(tune);
  if (duration == 0) {
    duration = tune->noteDuration;
  }

  // See whether we need to double the duration
  unsigned char base;
  if (tune->data[tune->index + 1] == '.') {
    ++ tune->index;
    base = 6;
  } else {
    base = 4;
  }

  // Calculate note duration in timer ticks (0.01s)
  int ticks = 6000U * base / duration / tempo;
  return ticks;
}


int parseNumber(struct Tune *tune) {
  int number = 0;
  if (tune->data[tune->index] != 0 && tune->data[tune->index] >= '0' && tune->data[tune->index] <= '9') {
    while (tune->data[tune->index] != 0 && tune->data[tune->index] >= '0' && tune->data[tune->index] <= '9') {
      number = number * 10 + (tune->data[tune->index ++] - 48);
    }
    tune->index --;
  }
  return number;
}

