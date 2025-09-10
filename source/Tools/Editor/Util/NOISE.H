#ifndef NOISE_H
#define NOISE_H

#include <vec3d.h>

inline float Dblrand (float min, float max)
{
  return (max-min) * (float(rand())/RAND_MAX) + min;
}

inline float Scurve (float val)
{
    return (val*val*(3.0f - 2.0f*val));  
}

class Noise
{
private:
        class HashTable
        {
        private:
                int32               value[256];
        public:
                                    HashTable (int32 seed);

                int                 operator() (int32 i1, int32 i2, int32 i3) const
                                    {
                                        return value[value[value[i1%256]^(i2%256)]^(i3%256)];
                                    }
        };

        class VecTable
        {
        public:
                jeFloat             x[256], y[256], z[256], o[256];

                                    VecTable (int32 seed);
        };

static  const HashTable             hTable;
static  const VecTable              vTable;
static  const jeVec3d                bigvect;

public:
        jeFloat                     NoiseAt (const jeVec3d &) const;
        jeVec3d                     DNoiseAt (const jeVec3d &) const;

        jeFloat                     fBm (jeVec3d, jeFloat, jeFloat, uint32) const;
        jeVec3d                     DfBm (jeVec3d, jeFloat, jeFloat, uint32) const;

        jeFloat                     Turbulence (jeVec3d, jeFloat, jeFloat, uint32) const;
};

class Noise2
{
private:
        class HashTable
        {
        private:
                uint32              value[256];
        public:
                                    HashTable (int32 seed);

                int                 operator() (int32 i1, int32 i2) const
                                    { return value[value[i1%256]^(i2%256)]; }
        };

        class VecTable
        {
        public:
                jeFloat             x[256], y[256], o[256];

                                    VecTable (int32 seed);
        };

static  const HashTable             hTable;
static  const VecTable              vTable;

public:
        jeFloat                     NoiseAt (jeFloat, jeFloat) const;

        jeFloat                     fBm (jeFloat, jeFloat, jeFloat, jeFloat, uint32) const;

        jeFloat                     Turbulence (jeFloat, jeFloat, jeFloat, jeFloat, uint32) const;
};

#endif
