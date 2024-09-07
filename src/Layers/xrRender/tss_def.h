#ifndef TSS_DEF_H
#define TSS_DEF_H

#pragma once

class SimulatorStates
{
private:
    struct State
    {
        u32 type; // 0=RS, 1=TSS
        u32 v1, v2, v3;

        void set_RS(u32 a, u32 b)
        {
            type = 0;
            v1 = a;
            v2 = b;
            v3 = 0;
        }

        void set_TSS(u32 a, u32 b, u32 c)
        {
            type = 1;
            v1 = a;
            v2 = b;
            v3 = c;
        }

        void set_SAMP(u32 a, u32 b, u32 c)
        {
            type = 2;
            v1 = a;
            v2 = b;
            v3 = c;
        }
    };

private:
    xr_vector<State> States;

public:
    SimulatorStates() = default;

    void set_RS(u32 a, u32 b);
    void set_TSS(u32 a, u32 b, u32 c);
    void set_SAMP(u32 a, u32 b, u32 c);
    BOOL equal(SimulatorStates& S);
    void clear();
    void record(ID3DState*& state);
};
#endif
