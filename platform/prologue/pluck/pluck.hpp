#pragma once

#include "userosc.h"
#include "biquad.hpp"
#include "delayline.hpp"

#define DELAY_BUFFER_SIZE 4096 // 2048 is too small to fit the lowest octave, and it must be a power of 2 due to DelayLine implementation
static float delay_buffer[DELAY_BUFFER_SIZE];

struct Pluck {

  enum {
    k_flags_none    = 0,
    k_flag_attack    = 1<<1,
    k_flag_damping    = 1<<2,
    k_flag_attenuation  = 1<<3,
    k_flag_inharmonicity  = 1<<4,
    k_flag_drive = 1<<5,
    k_flag_reset    = 1<<6
  };
  
  struct Params {
    float    attack;
    float    damping;
    float    attenuation;
    float    inharmonicity;
    float    drive;
   
    Params(void) :
      attack(10.f),
      damping(.5f),
      attenuation(0.f),
      inharmonicity(0.f),
      drive(1.f)
    { }
  };
  
  struct State {
    uint32_t burst;
    float lfo, lfoz;
    uint32_t flags:8;
        
    State(void) :
      lfo(0.f),
      lfoz(0.f),
      flags(k_flags_none)
    {}
  };

  Pluck(void) {
    init();
  }

  void init(void) {
    state = State();
    params = Params();
    delay.setMemory(delay_buffer, DELAY_BUFFER_SIZE);
    impulse_filter.mCoeffs.setPoleLP(0.9f);
    postlpf.mCoeffs.setFOLP(osc_tanpif(0.45f));

  }
  
  inline void updatePitch(float w0) {
  }

  State       state;
  Params      params;
  dsp::BiQuad impulse_filter, postlpf;
  dsp::DelayLine delay;

};
