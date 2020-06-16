

#pragma once


class WaveForm
{
public:
                WaveForm(int nb, uint16_t *s)
                {
                    _nbSamples=nb;
                    _samples=s;
                }
                /**
                 * \brief search a rampup in the waveform. PointA is at 10% of max, point B is at target of max
                 * @param target
                 * @param delta
                 * @param overflow
                 * @param underflow
                 * @param a
                 * @param b
                 * @return 
                 */
                bool searchRampUp(float  target, int &delta,bool &overflow, bool &underflow,int &a, int &b);

protected:
    int         _nbSamples;
    uint16_t    *_samples;
    
};