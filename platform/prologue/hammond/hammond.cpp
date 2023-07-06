#include "userosc.h"

#define H_SUB .5f
#define H_FIFTH 1.5f
#define H_2ND 2.f
#define H_3RD 3.f
#define H_4TH 4.f
#define H_5TH 5.f
#define H_6TH 6.f
#define H_8TH 8.f

float INIT[9] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
float POP[9] = {1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f};
float JAZZ[9] = {1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f};


enum {
  k_flags_none    = 0,
  k_flag_organ_preset = 1<<0,
  k_flag_reset    = 1<<1
};

typedef struct Params {
  float shape, shiftshape;
} Params;

typedef struct State {
  float    phi1, phi2, phi3, phi4, phi5, phi6, phi7, phi8, phi9;
  float    w01, w02, w03, w04, w05, w06, w07, w08, w09;
  float    a1, a2, a3, a4, a5, a6, a7, a8, a9;
  float    lfo, lfoz;
  uint32_t flags:8;
} State;

static Params p;
static State s;

inline void updatePitch(float w0) {
  s.w01 = w0 * H_SUB;
  s.w02 = w0;
  s.w03 = w0 * H_FIFTH;
  s.w04 = w0 * H_2ND;
  s.w05 = w0 * H_3RD;
  s.w06 = w0 * H_4TH;
  s.w07 = w0 * H_5TH;
  s.w08 = w0 * H_6TH;
  s.w09 = w0 * H_8TH;
}

inline void updateAmplitudes(void) {

  float *amps;
  if (p.shape >= 0 && p.shape <= 10) {
    amps = POP;
  }
  else {
    amps = JAZZ;
  }

  float total_amplitude = 0.f;
   //Calculate length of array arr    
  // int length = sizeof(*amps)/sizeof(*amps[0]);    
  for (int i = 0; i < 9; i++) {     
      total_amplitude += *(amps+i);    
  }    

  const float scale = 5.f;
  s.a1 = *amps / total_amplitude;
  s.a2 = *(amps+1) / total_amplitude;
  s.a3 = *(amps+2) / total_amplitude;
  s.a4 = *(amps+3) / total_amplitude;
  s.a5 = *(amps+4) / total_amplitude;
  s.a6 = *(amps+5) / total_amplitude;
  s.a7 = *(amps+6) / total_amplitude;
  s.a8 = *(amps+7) / total_amplitude;
  s.a9 = *(amps+8) / total_amplitude;

}

inline void resetPhase(void) {
//  s.phi1 = s.phi2 = s.phi3 = s.phi4 = s.phi5 = s.phi6 = s.phi7 = s.phi8 = 0.f;
  s.lfo = s.lfoz;
}

void OSC_INIT(uint32_t platform, uint32_t api)
{
  updateAmplitudes();
  updatePitch(440.f);
}

void OSC_CYCLE(const user_osc_param_t * const params,
               int32_t *yn,
               const uint32_t frames)
{
  // Handle events.
  {
    const uint32_t flags = s.flags;
    s.flags = k_flags_none;
    
    updatePitch(osc_w0f_for_note((params->pitch)>>8, params->pitch & 0xFF));
    
    if (flags & k_flag_organ_preset)
      updateAmplitudes();
    if (flags & k_flag_reset)
      resetPhase();
    
    s.lfo = q31_to_f32(params->shape_lfo);
  }
  
  // Temporaries.
  float phi1 = s.phi1;
  float phi2 = s.phi2;
  float phi3 = s.phi3;
  float phi4 = s.phi4;
  float phi5 = s.phi5;
  float phi6 = s.phi6;
  float phi7 = s.phi7;
  float phi8 = s.phi8;
  float phi9 = s.phi9;

  const float shape = p.shape;
  const float drive = p.shiftshape;

  float lfoz = s.lfoz;
  const float lfo_inc = (s.lfo - lfoz) / frames;
  
  q31_t * __restrict y = (q31_t *)yn;
  const q31_t * y_e = y + frames;
  
  for (; y != y_e; ) {

    float sig;

    sig = s.a1 * osc_sinf(phi1) +
        + s.a2 * osc_sinf(phi2) +
        + s.a3 * osc_sinf(phi3) +
        + s.a4 * osc_sinf(phi4) +
        + s.a5 * osc_sinf(phi5) +
        + s.a6 * osc_sinf(phi6) +
        + s.a7 * osc_sinf(phi7) +
        + s.a8 * osc_sinf(phi8) +
        + s.a9 * osc_sinf(phi9);

    //sig = clip1m1f(sig);
    const float main_sig = osc_softclipf(0.05f, drive * sig);    
    
    *(y++) = f32_to_q31(main_sig);
    
    phi1 += s.w01;
    phi1 -= (uint32_t)phi1;
    phi2 += s.w02;
    phi2 -= (uint32_t)phi2;
    phi3 += s.w03;
    phi3 -= (uint32_t)phi3;
    phi4 += s.w04;
    phi4 -= (uint32_t)phi4;
    phi5 += s.w05;
    phi5 -= (uint32_t)phi5;
    phi6 += s.w06;
    phi6 -= (uint32_t)phi6;
    phi7 += s.w07;
    phi7 -= (uint32_t)phi7;
    phi8 += s.w08;
    phi8 -= (uint32_t)phi8;
    phi9 += s.w09;
    phi9 -= (uint32_t)phi9;
    lfoz += lfo_inc;
  }
  
  s.phi1 = phi1;
  s.phi2 = phi2;
  s.phi3 = phi3;
  s.phi4 = phi4;
  s.phi5 = phi5;
  s.phi6 = phi6;
  s.phi7 = phi7;
  s.phi8 = phi8;
  s.phi9 = phi9;
  s.lfoz = lfoz;
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
  s.flags |= k_flag_reset;
}

void OSC_NOTEOFF(const user_osc_param_t * const params)
{
  (void)params;
}

void OSC_PARAM(uint16_t index, uint16_t value)
{ 
  switch (index) {
  case k_user_osc_param_id1:
    break;
    
  case k_user_osc_param_id2:
    break;
    
  case k_user_osc_param_id3:
    break;
    
  case k_user_osc_param_id4:
    break;
    
  case k_user_osc_param_id5:
    break;
    
  case k_user_osc_param_id6:
    break;
    
  case k_user_osc_param_shape:
    p.shape = param_val_to_f32(value);
    s.flags |= k_flag_organ_preset;
    break;
    
  case k_user_osc_param_shiftshape:
    p.shiftshape = 1.f + param_val_to_f32(value); 
    break;
    
  default:
    break;
  }
}
