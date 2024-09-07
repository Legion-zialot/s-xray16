#ifndef sh_atomicH
#define sh_atomicH
#pragma once

#include "xrCore/xr_resource.h"
#include "tss_def.h"

#pragma pack(push, 4)

//////////////////////////////////////////////////////////////////////////
// Atomic resources
//////////////////////////////////////////////////////////////////////////
struct ECORE_API SVS : public xr_resource_named
{
#if defined(USE_DX9)
    ID3DVertexShader* sh;
#else
#   error No graphics API selected or enabled!
#endif
    R_constant_table constants;
    SVS();
    ~SVS();
};
typedef resptr_core<SVS, resptr_base<SVS>> ref_vs;

//////////////////////////////////////////////////////////////////////////
struct ECORE_API SPS : public xr_resource_named
{
#if defined(USE_DX9)
    ID3DPixelShader* sh;
#else
#   error No graphics API selected or enabled!
#endif
    R_constant_table constants;
    ~SPS();
};
typedef resptr_core<SPS, resptr_base<SPS>> ref_ps;

//////////////////////////////////////////////////////////////////////////
struct ECORE_API SState : public xr_resource_flagged
{
    ID3DState* state;
    SimulatorStates state_code;
    SState() = default;
    ~SState();
};
typedef resptr_core<SState, resptr_base<SState>> ref_state;

//////////////////////////////////////////////////////////////////////////
struct ECORE_API SDeclaration : public xr_resource_flagged
{
#if defined(USE_DX9) //	Don't need it: use ID3DInputLayout instead
    //	which is per ( declaration, VS input layout) pair
    IDirect3DVertexDeclaration9* dcl;
#else
#   error No graphics API selected or enabled!
#endif

    //	Use this for DirectX10 to cache DX9 declaration for comparison purpose only
    xr_vector<VertexElement> dcl_code;
    SDeclaration() = default;
    ~SDeclaration();
};
typedef resptr_core<SDeclaration, resptr_base<SDeclaration>> ref_declaration;

#pragma pack(pop)
#endif // sh_atomicH
