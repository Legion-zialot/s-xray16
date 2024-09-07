#ifndef SH_TEXTURE_H
#define SH_TEXTURE_H
#pragma once

#include "xrCore/xr_resource.h"

class CAviPlayerCustom;
class ENGINE_API CTheoraSurface;

class ECORE_API CTexture : public xr_resource_named
{
public:
#if defined(USE_DX9)
    enum MaxTextures
    {
        mtMaxPixelShaderTextures = 16,
        mtMaxVertexShaderTextures = 4,
        mtMaxCombinedShaderTextures =
        mtMaxPixelShaderTextures
        + mtMaxVertexShaderTextures
    };
#else
#   error No graphics API selected or enabled!
#endif

#if defined(USE_DX9)
    //	Since DX11 allows up to 128 unique textures,
    //	distance between enum values should be at leas 128
    enum ResourceShaderType //	Don't change this since it's hardware-dependent
    {
        rstPixel = 0,
        // Default texture offset
        rstVertex = D3DVERTEXTEXTURESAMPLER0,
        rstGeometry = rstVertex + 256,
        rstHull = rstGeometry + 256,
        rstDomain = rstHull + 256,
        rstCompute = rstDomain + 256,
        rstInvalid = rstCompute + 256
    };
#else
#   error No graphics API selected or enabled!
#endif

public:
    void apply_load(CBackend& cmd_list, u32 stage);
    void apply_theora(CBackend& cmd_list, u32 stage);
    void apply_avi(CBackend& cmd_list, u32 stage) const;
    void apply_seq(CBackend& cmd_list, u32 stage);
    void apply_normal(CBackend& cmd_list, u32 stage) const;

    void set_slice(int slice);

    void Preload();
    void Load();
    void PostLoad();
    void Unload();
    // void Apply(u32 dwStage);

#if defined(USE_DX9)
    void surface_set(ID3DBaseTexture* surf);
    [[nodiscard]] ID3DBaseTexture* surface_get() const;
#else
#   error No graphics API selected or enabled!
#endif

    [[nodiscard]] BOOL isUser() const
    {
        return flags.bUser;
    }

    u32 get_Width()
    {
        desc_enshure();
        return m_width;
    }

    u32 get_Height()
    {
        desc_enshure();
        return m_height;
    }

    void video_Sync(u32 _time) { m_play_time = _time; }
    void video_Play(BOOL looped, u32 _time = 0xFFFFFFFF);
    void video_Pause(BOOL state) const;
    void video_Stop() const;
    [[nodiscard]] BOOL video_IsPlaying() const;

    CTexture();
    virtual ~CTexture();

private:
    [[nodiscard]] BOOL desc_valid() const
    {
        return pSurface == desc_cache;
    }

    void desc_enshure()
    {
        if (!desc_valid())
            desc_update();
    }

    void desc_update();

    //	Class data
public: //	Public class members (must be encapsulated further)
    struct
    {
        u32 bLoaded : 1;
        u32 bUser : 1;
        u32 seqCycles : 1;
        u32 MemoryUsage : 28;
    } flags;

    fastdelegate::FastDelegate2<CBackend&,u32> bind;

    CAviPlayerCustom* pAVI;
    CTheoraSurface* pTheora;
    float m_material;
    shared_str m_bumpmap;

    union
    {
        u32 m_play_time; // sync theora time
        u32 seqMSPF; // Sequence data milliseconds per frame
    };

    int curr_slice{ -1 };
    int last_slice{ -1 };

private:
#if defined(USE_DX9)
    ID3DBaseTexture* pSurface{};
    ID3DBaseTexture* pTempSurface{};
    // Sequence data
    xr_vector<ID3DBaseTexture*> seqDATA;

    // Description
    u32 m_width;
    u32 m_height;
    ID3DBaseTexture* desc_cache;
    D3D_TEXTURE2D_DESC desc;
#else
#   error No graphics API selected or enabled!
#endif

};

struct resptrcode_texture : public resptr_base<CTexture>
{
    void create(LPCSTR _name);
    void destroy() { _set(nullptr); }
    shared_str bump_get() { return _get()->m_bumpmap; }
    bool bump_exist() { return 0 != bump_get().size(); }
};

typedef resptr_core<CTexture, resptrcode_texture> ref_texture;

#endif
