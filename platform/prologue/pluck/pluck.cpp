#include "userosc.h"
#include "delayline.hpp"
#include "biquad.hpp"
#include "pluck.hpp"

static Pluck s_pluck;

void OSC_INIT(uint32_t platform, uint32_t api)
{
  (void)platform;
  (void)api;
}

void OSC_CYCLE(const user_osc_param_t * const params,
               int32_t *yn,
               const uint32_t frames)
{
  Pluck::State &s = s_pluck.state;
  const Pluck::Params &p = s_pluck.params;

  // Handle events.
  {
    const uint32_t flags = s.flags;
    s.flags = Pluck::k_flags_none;
    
    if (flags & Pluck::k_flag_reset) {
      s.delay.clear();
      s.burst = 48.f*s.attack; // milliseconds at 48khz
    }
    
    s.lfo = q31_to_f32(params->shape_lfo);
  }
  
  dsp::BiQuad &impulse_filter = s.impulse_filter;
  dsp::DelayLine &delay = s.delay;

  const float attenuation = s.attenuation;
  const float length = clipminmaxf(2.f, 1.f / osc_w0f_for_note((params->pitch)>>8, params->pitch & 0xFF), DELAY_BUFFER_SIZE);

  const float drive = s.drive;

  uint32_t burst = s.burst;

  float lfoz = s.lfoz;
  const float lfo_inc = (s.lfo - lfoz) / frames;
  
  q31_t * __restrict y = (q31_t *)yn;
  const q31_t * y_e = y + frames;
  float lastSig = 0;
  float impulse = 0;
  for (; y != y_e; ) {
    float sig = delay.readFrac(length);

    // low-pass filter for damping
    const float damping = clipminmaxf(.000001f, s.damping + lfoz, .999999f);
    sig = (1.f - attenuation) * (sig*damping + lastSig*(1.f - damping));

    if (burst>0) {
      burst--;
      sig += impulse_filter.process_fo(osc_white());
    }

    // TODO: all-pass filter for inharmonicity

    delay.write(osc_softclipf(0.05f, sig));

    sig = osc_softclipf(0.05f, sig * drive);

    *(y++) = f32_to_q31(sig);

    lastSig = sig; 

    lfoz += lfo_inc;
  }
  
  s.burst = burst;
  s.lfoz = lfoz;
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
  s_pluck.state.flags |= Pluck::k_flag_reset;
}

void OSC_NOTEOFF(const user_osc_param_t * const params)
{
  (void)params;
}

void OSC_PARAM(uint16_t index, uint16_t value)
{ 
  Pluck::State &s = s_pluck.state;
  Pluck::Params &p = s_pluck.params;
  
  switch (index) {
  case k_user_osc_param_id1:
    {
      const float x = value*.01f*.6f + .1f;
      s.attenuation = x*x*x; // 0.01 to 0.343 with more resolution in the lower values
      s.flags |= Pluck::k_flag_attenuation;
    }
    break;
  case k_user_osc_param_id2:
    {
      s.drive = 1.f + value*.01f; // 1 to 2
      s.flags |= Pluck::k_flag_drive;
    }
    break;
  case k_user_osc_param_id3:
  case k_user_osc_param_id4:
  case k_user_osc_param_id5:
  case k_user_osc_param_id6:
    break;
    
  case k_user_osc_param_shape:
    {
      s.damping = 1.f - clipminmaxf(.0000001f, param_val_to_f32(value), .999999f); // 1 to 0
      s.flags |= Pluck::k_flag_damping;
    }
    break;
    
  case k_user_osc_param_shiftshape:
    {
      const float x = 1.f - param_val_to_f32(value);
      s.impulse_filter.mCoeffs.setPoleLP(clipminmaxf(.0000001f, 1.f - x*x*x, .999999f)); // more resolution near 1
      s.flags |= Pluck::k_flag_attack;
    }
    break;
 
  default:
    break;
  }
}
