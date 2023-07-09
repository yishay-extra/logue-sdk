#include "userosc.h"
#include "hammond.hpp"

static Hammond s_hammond;

void OSC_INIT(uint32_t platform, uint32_t api)
{
  (void)platform;
  (void)api;
  s_hammond.updateAmplitudes();
  s_hammond.updatePitch(440.f);
}

void OSC_CYCLE(const user_osc_param_t * const params,
               int32_t *yn,
               const uint32_t frames)
{

  Hammond::State &s = s_hammond.state;
  const Hammond::Params &p = s_hammond.params;

  // Handle events.
  {
    const uint32_t flags = s.flags;
    s.flags = Hammond::k_flags_none;
    
    s_hammond.updatePitch(osc_w0f_for_note((params->pitch)>>8, params->pitch & 0xFF));
    
    if (flags & Hammond::k_flag_organ_preset)
      s_hammond.updateAmplitudes();
    if (flags & Hammond::k_flag_reset)
      s.reset();
    
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

  dsp::BiQuad &prelpf = s_hammond.prelpf;
  dsp::BiQuad &postlpf = s_hammond.postlpf;
  
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

    sig = osc_softclipf(0.05f, drive * sig);   

    // sig = prelpf.process_fo(sig);
    // sig += s.dither * osc_white();
    sig = postlpf.process_fo(sig);
    sig = osc_softclipf(0.125f, sig); 
    
    *(y++) = f32_to_q31(sig);
    
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
  s_hammond.state.flags |= Hammond::k_flag_reset;
}

void OSC_NOTEOFF(const user_osc_param_t * const params)
{
  (void)params;
}

void OSC_PARAM(uint16_t index, uint16_t value)
{ 
  Hammond::Params &p = s_hammond.params;
  Hammond::State &s = s_hammond.state;

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
    s.flags |= Hammond::k_flag_organ_preset;
    break;
    
  case k_user_osc_param_shiftshape:
    p.shiftshape = 1.f + param_val_to_f32(value); 
    break;
    
  default:
    break;
  }
}
