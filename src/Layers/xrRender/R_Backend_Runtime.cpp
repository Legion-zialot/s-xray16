#include "stdafx.h"
#pragma hdrstop

#if defined(USE_DX9)
#include <DirectXMath.h>
#endif

void CBackend::OnFrameEnd()
{
    if (!GEnv.isDedicatedServer)
    {
#if defined(USE_DX9)
        for (u32 stage = 0; stage < HW.Caps.raster.dwStages; stage++)
            CHK_DX(HW.pDevice->SetTexture(0, nullptr));
        CHK_DX(HW.pDevice->SetStreamSource(0, nullptr, 0, 0));
        CHK_DX(HW.pDevice->SetIndices(nullptr));
        CHK_DX(HW.pDevice->SetVertexShader(nullptr));
        CHK_DX(HW.pDevice->SetPixelShader(nullptr));
#endif
        Invalidate();
    }
}

void CBackend::OnFrameBegin()
{
    if (!GEnv.isDedicatedServer)
    {
        PGO(Msg("PGO:*****frame[%d]*****", Device.dwFrame));

#ifndef USE_DX9
        Invalidate();
        // DX9 sets base rt and base zb by default
        set_FB(HW.pFB);
        set_RT(RImplementation.Target->get_base_rt());
        set_ZB(RImplementation.Target->get_base_zb());
#endif

        ZeroMemory(&stat, sizeof(stat));
        set_Stencil(FALSE);
    }
}

void CBackend::Invalidate()
{
    pRT[0] = 0;
    pRT[1] = 0;
    pRT[2] = 0;
    pRT[3] = 0;
    pZB = 0;

    decl = nullptr;
    vb = 0;
    ib = 0;
    vb_stride = 0;

    state = nullptr;
    ps = 0;
    vs = 0;
    DX11_ONLY(gs = NULL);
    ctable = nullptr;

    T = nullptr;
    M = nullptr;
    C = nullptr;

    stencil_enable = u32(-1);
    stencil_func = u32(-1);
    stencil_ref = u32(-1);
    stencil_mask = u32(-1);
    stencil_writemask = u32(-1);
    stencil_fail = u32(-1);
    stencil_pass = u32(-1);
    stencil_zfail = u32(-1);
    cull_mode = u32(-1);
    fill_mode = u32(-1);
    z_enable = u32(-1);
    z_func = u32(-1);
    alpha_ref = u32(-1);
    colorwrite_mask = u32(-1);

    // Since constant buffers are unmapped (for DirecX 10)
    // transform setting handlers should be unmapped too.
    xforms.unmap();

    for (u32 ps_it = 0; ps_it < CTexture::mtMaxPixelShaderTextures;)
        textures_ps[ps_it++] = nullptr;
    for (u32 vs_it = 0; vs_it < CTexture::mtMaxVertexShaderTextures;)
        textures_vs[vs_it++] = nullptr;
    for (auto& matrix : matrices)
        matrix = nullptr;
}

void CBackend::set_ClipPlanes(u32 _enable, Fplane* _planes /*=NULL */, u32 count /* =0*/)
{
#if defined(USE_DX9)
    if (0 == HW.Caps.geometry.dwClipPlanes)
        return;
    if (!_enable)
    {
        CHK_DX(HW.pDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, FALSE));
        return;
    }

    // Enable and setup planes
    VERIFY(_planes && count);
    if (count > HW.Caps.geometry.dwClipPlanes)
        count = HW.Caps.geometry.dwClipPlanes;

    using namespace DirectX;

    const XMMATRIX transform = XMLoadFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&Device.mFullTransform));
    XMMATRIX worldToClipMatrixIT = XMMatrixInverse(nullptr, transform);
    worldToClipMatrixIT = XMMatrixTranspose(worldToClipMatrixIT);

    for (u32 it = 0; it < count; it++)
    {
        Fplane& P = _planes[it];
        XMFLOAT4 planeClip;
        XMVECTOR planeWorld = XMPlaneNormalize(XMVectorSet(-P.n.x, -P.n.y, -P.n.z, -P.d));
        XMStoreFloat4(&planeClip, XMPlaneTransform(planeWorld, worldToClipMatrixIT));
        CHK_DX(HW.pDevice->SetClipPlane(it, reinterpret_cast<float*>(&planeClip)));
    }

    // Enable them
    u32 e_mask = (1 << count) - 1;
    CHK_DX(HW.pDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, e_mask));
#else
#   error No graphics API selected or enabled!
#endif //USE_DX9
}

#ifndef DEDICATED_SREVER
void CBackend::set_ClipPlanes(u32 _enable, Fmatrix* _xform /*=NULL */, u32 fmask /* =0xff */)
{
    if (0 == HW.Caps.geometry.dwClipPlanes)
        return;
    if (!_enable)
    {
#if defined(USE_DX9)
        CHK_DX(HW.pDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, FALSE));
#else
#   error No graphics API selected or enabled!
#endif
        return;
    }
    VERIFY(_xform && fmask);
    CFrustum F;
    F.CreateFromMatrix(*_xform, fmask);
    set_ClipPlanes(_enable, F.planes, F.p_count);
}

void CBackend::set_Textures(STextureList* _T)
{
    // TODO: expose T invalidation method
    //if (T == _T) // disabled due to cases when the set of resources the same, but different srv is need to be bind
    //    return;
    T = _T;
    // If resources weren't set at all we should clear from resource #0.
    int _last_ps = -1;
    int _last_vs = -1;
    STextureList::iterator _it = _T->begin();
    STextureList::iterator _end = _T->end();

    for (; _it != _end; ++_it)
    {
        std::pair<u32, ref_texture>& loader = *_it;
        u32 load_id = loader.first;
        CTexture* load_surf = loader.second._get();
        //if (load_id < 256) {
        if (load_id < CTexture::rstVertex)
        {
            // Set up pixel shader resources
            VERIFY(load_id < CTexture::mtMaxPixelShaderTextures);
            // ordinary pixel surface
            if ((int)load_id > _last_ps)
                _last_ps = load_id;
            if (textures_ps[load_id] != load_surf || (load_surf && (load_surf->last_slice != load_surf->curr_slice)))
            {
                textures_ps[load_id] = load_surf;
                stat.textures++;

                if (load_surf)
                {
                    PGO(Msg("PGO:tex%d:%s", load_id, load_surf->cName.c_str()));
                    load_surf->bind(*this, load_id);
                    //load_surf->Apply(load_id);
                    load_surf->last_slice = load_surf->curr_slice;
                }
            }
        }
        else
        {
            // Set up pixel shader resources
            VERIFY(load_id < CTexture::rstVertex + CTexture::mtMaxVertexShaderTextures);

            // vertex only //d-map or vertex
            u32 load_id_remapped = load_id - CTexture::rstVertex;
            if ((int)load_id_remapped > _last_vs)
                _last_vs = load_id_remapped;
            if (textures_vs[load_id_remapped] != load_surf)
            {
                textures_vs[load_id_remapped] = load_surf;
                stat.textures++;

                if (load_surf)
                {
                    PGO(Msg("PGO:tex%d:%s", load_id, load_surf->cName.c_str()));
                    load_surf->bind(*this, load_id);
                    //load_surf->Apply(load_id);
                }
            }
        }
    }

    // clear remaining stages (PS)
    for (++_last_ps; _last_ps < CTexture::mtMaxPixelShaderTextures; _last_ps++)
    {
        if (!textures_ps[_last_ps])
            continue;

        textures_ps[_last_ps] = nullptr;
#if defined(USE_DX9)
        CHK_DX(HW.pDevice->SetTexture(_last_ps, NULL));
#else
#   error No graphics API selected or enabled!
#endif
    }
    // clear remaining stages (VS)
    for (++_last_vs; _last_vs < CTexture::mtMaxVertexShaderTextures; _last_vs++)
    {
        if (!textures_vs[_last_vs])
            continue;

        textures_vs[_last_vs] = nullptr;
#if defined(USE_DX9)
        CHK_DX(HW.pDevice->SetTexture(_last_vs + CTexture::rstVertex, NULL));
#else
#   error No graphics API selected or enabled!
#endif
    }

}
#else

void CBackend::set_ClipPlanes(u32 _enable, Fmatrix* _xform /*=NULL */, u32 fmask /* =0xff */) {}
void CBackend::set_Textures(STextureList* _T) {}

#endif // DEDICATED SERVER

void CBackend::SetupStates()
{
    set_CullMode(CULL_CCW);
#if defined(USE_DX9)
    for (u32 i = 0; i < HW.Caps.raster.dwStages; i++)
    {
        CHK_DX(HW.pDevice->SetSamplerState(i, D3DSAMP_MAXANISOTROPY, ps_r__tf_Anisotropic));
        CHK_DX(HW.pDevice->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, *(u32*)&ps_r__tf_Mipbias));
        CHK_DX(HW.pDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
        CHK_DX(HW.pDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
        CHK_DX(HW.pDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));
    }
    CHK_DX(HW.pDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE));
    CHK_DX(HW.pDevice->SetRenderState(D3DRS_COLORVERTEX, TRUE));
    CHK_DX(HW.pDevice->SetRenderState(D3DRS_ZENABLE, TRUE));
    CHK_DX(HW.pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD));
    CHK_DX(HW.pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER));
    CHK_DX(HW.pDevice->SetRenderState(D3DRS_LOCALVIEWER, TRUE));
    CHK_DX(HW.pDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL));
    CHK_DX(HW.pDevice->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL));
    CHK_DX(HW.pDevice->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL));
    CHK_DX(HW.pDevice->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_COLOR1));
    CHK_DX(HW.pDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE));
    CHK_DX(HW.pDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE));

    Fmaterial mat
    {
        /*.diffuse  =*/ { 1, 1, 1, 1 },
        /*.ambient  =*/ { 1, 1, 1, 1 },
        /*.emissive =*/ { 0, 0, 0, 0 },
        /*.specular =*/ { 1, 1, 1, 1 },
        /*.power    =*/ 15.f
    };
    CHK_DX(HW.pDevice->SetMaterial(reinterpret_cast<D3DMATERIAL9*>(&mat)));

    if (psDeviceFlags.test(rsWireframe))
        CHK_DX(HW.pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME));
    else
        CHK_DX(HW.pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID));
    // ******************** Fog parameters
    CHK_DX(HW.pDevice->SetRenderState(D3DRS_FOGCOLOR, 0));
    CHK_DX(HW.pDevice->SetRenderState(D3DRS_RANGEFOGENABLE, FALSE));
    if (HW.Caps.bTableFog)
    {
        CHK_DX(HW.pDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR));
        CHK_DX(HW.pDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE));
    }
    else
    {
        CHK_DX(HW.pDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE));
        CHK_DX(HW.pDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR));
    }
#else
#   error No graphics API selected or enabled!
#endif
}


// Device dependance
void CBackend::OnDeviceCreate()
{
    // Debug Draw
    InitializeDebugDraw();

    // invalidate caching
    Invalidate();
}

void CBackend::OnDeviceDestroy()
{
    // Debug Draw
    DestroyDebugDraw();
}

#include "LightTrack.h"
#include "xrEngine/IRenderable.h"

void CBackend::apply_lmaterial()
{
    R_constant* C = get_c(c_sbase)._get(); // get sampler
    if (!C)
        return;

    VERIFY(RC_dest_sampler == C->destination);
#if defined(USE_DX9)
    VERIFY(RC_sampler == C->type);
#else
#   error No graphics API selected or enabled!
#endif

    CTexture* T = get_ActiveTexture(u32(C->samp.index));
    VERIFY(T);
    float mtl = T->m_material;
#ifdef DEBUG
    if (ps_r2_ls_flags.test(R2FLAG_GLOBALMATERIAL))
        mtl = ps_r2_gmaterial;
#endif
    hemi.set_material(o_hemi, o_sun, 0, (mtl + .5f) / 4.f);
    hemi.set_pos_faces(o_hemi_cube[CROS_impl::CUBE_FACE_POS_X],
                                o_hemi_cube[CROS_impl::CUBE_FACE_POS_Y],
                                o_hemi_cube[CROS_impl::CUBE_FACE_POS_Z]);
    hemi.set_neg_faces(o_hemi_cube[CROS_impl::CUBE_FACE_NEG_X],
                                o_hemi_cube[CROS_impl::CUBE_FACE_NEG_Y],
                                o_hemi_cube[CROS_impl::CUBE_FACE_NEG_Z]);
}
