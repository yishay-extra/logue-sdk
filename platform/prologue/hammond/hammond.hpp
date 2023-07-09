#pragma once
#include "userosc.h"
#include "biquad.hpp"
#include "stops_presets.hpp"

#define H_SUB .5f
#define H_FIFTH 1.5f
#define H_2ND 2.f
#define H_3RD 3.f
#define H_4TH 4.f
#define H_5TH 5.f
#define H_6TH 6.f
#define H_8TH 8.f


struct Hammond {

  enum {
    k_flags_none        = 0,
    k_flag_organ_preset = 1<<0,
    k_flag_reset        = 1<<1
  };

  struct Params {
    float shape;
    float shiftshape;

    Params(void) :
      shape(0.f),
      shiftshape(0.f)
    { }
  };

  struct State {
    float    phi1, phi2, phi3, phi4, phi5, phi6, phi7, phi8, phi9;
    float    w01, w02, w03, w04, w05, w06, w07, w08, w09;
    float    a1, a2, a3, a4, a5, a6, a7, a8, a9;
    float    lfo, lfoz;
    float    dither;
    uint32_t flags: 8;

    State(void) :

      w01(440.f * H_SUB * k_samplerate_recipf),
      w02(440.f * k_samplerate_recipf),
      w03(440.f * H_FIFTH * k_samplerate_recipf),
      w04(440.f * H_2ND * k_samplerate_recipf),
      w05(440.f * H_3RD * k_samplerate_recipf),
      w06(440.f * H_4TH * k_samplerate_recipf),
      w07(440.f * H_5TH * k_samplerate_recipf),
      w08(440.f * H_6TH * k_samplerate_recipf),
      w09(440.f * H_8TH * k_samplerate_recipf),
      a1(0.f),
      a2(0.f),
      a3(0.f),
      a4(0.f),
      a5(0.f),
      a6(0.f),
      a7(0.f),
      a8(0.f),
      a9(0.f),
      dither(0.f),
      lfo(0.f),
      lfoz(0.f),
      flags(k_flags_none)
    {
      reset();
    }
    
    inline void reset(void)
    {
      phi1 = phi2 = phi3 = phi4 = phi5 = phi6 = phi7 = phi8 = phi9 = 0.f;
      lfo = lfoz;
    }
  };

  Hammond(void) {
    init();
  }

  void init(void) {
    state = State();
    params = Params();
    prelpf.mCoeffs.setPoleLP(0.8f);
    postlpf.mCoeffs.setFOLP(osc_tanpif(0.45f));
  }

  inline void updatePitch(float w0) {
    state.w01 = w0 * H_SUB;
    state.w02 = w0;
    state.w03 = w0 * H_FIFTH;
    state.w04 = w0 * H_2ND;
    state.w05 = w0 * H_3RD;
    state.w06 = w0 * H_4TH;
    state.w07 = w0 * H_5TH;
    state.w08 = w0 * H_6TH;
    state.w09 = w0 * H_8TH;
  }

  inline void updateAmplitudes(void) {

    float *amps;
    if (params.shape >= 0.0 && params.shape <= 0.1) {
      amps = POP;
    }
    else if (params.shape > 0.1 && params.shape <= 0.2) {
      amps = FLUTE;
    }
    else if (params.shape > 0.2 && params.shape <= 0.3) {
      amps = OBOE;
    }
    else if (params.shape > 0.3 && params.shape <= 0.4) {
      amps = DIAPSON;
    }
    else if (params.shape > 0.4 && params.shape <= 0.5) {
      amps = CELLO;
    }
    else if (params.shape > 0.5 && params.shape <= 0.6) {
      amps = STRING;
    }
    else if (params.shape > 0.6 && params.shape <= 0.7) {
      amps = VOXHUMANA;
    }
    else if (params.shape > 0.7 && params.shape <= 0.8) {
      amps = HORN;
    }
    else if (params.shape > 0.8 && params.shape <= 0.9) {
      amps = TIBIA;
    } 
    else {
      amps = JAZZ;
    }

    float total_amplitude = 0.f;   
    for (int i = 0; i < 9; i++) {     
        total_amplitude += *(amps+i);    
    }    

    const float scale = 5.f;
    state.a1 = *amps / total_amplitude;
    state.a2 = *(amps+1) / total_amplitude;
    state.a3 = *(amps+2) / total_amplitude;
    state.a4 = *(amps+3) / total_amplitude;
    state.a5 = *(amps+4) / total_amplitude;
    state.a6 = *(amps+5) / total_amplitude;
    state.a7 = *(amps+6) / total_amplitude;
    state.a8 = *(amps+7) / total_amplitude;
    state.a9 = *(amps+8) / total_amplitude;
  }

  State       state;
  Params      params;
  dsp::BiQuad prelpf, postlpf;

};
