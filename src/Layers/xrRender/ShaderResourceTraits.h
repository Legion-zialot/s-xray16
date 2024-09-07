#pragma once

#include "ResourceManager.h"
#ifdef USE_DX9
#   include "Layers/xrRenderDX9/dx9shader_utils.h"
#endif

template <typename T>
struct ShaderTypeTraits;

template <>
struct ShaderTypeTraits<SVS>
{
    using MapType = CResourceManager::map_VS;

#if defined(USE_DX9)
    using LinkageType = void*;
    using HWShaderType = ID3DVertexShader*;
    using BufferType = DWORD const*;
    using ResultType = HRESULT;
#endif

    static inline const char* GetShaderExt() { return ".vs"; }

    static inline const char* GetCompilationTarget()
    {
        return HW.Caps.geometry_profile;
    }

    static void GetCompilationTarget(const char*& target, const char*& entry, const char* data)
    {
        entry = "main";

        if (strstr(data, "main_vs_1_1"))
        {
            target = "vs_1_1";
            entry = "main_vs_1_1";
        }
        else if (strstr(data, "main_vs_2_0"))
        {
            target = "vs_2_0";
            entry = "main_vs_2_0";
        }
#ifdef USE_DX9 // For DX10+ we always should use SM4.0 or higher
        else
#endif
        {
            target = GetCompilationTarget();
        }
    }

    static inline ResultType CreateHWShader(BufferType buffer, size_t size, HWShaderType& sh,
        LinkageType linkage = nullptr, pcstr name = nullptr)
    {
        ResultType res{};

#if defined(USE_DX9)
        res = HW.pDevice->CreateVertexShader(buffer, &sh);
        UNUSED(linkage, name);
#else
#   error No graphics API selected or enabled!
#endif

        return res;
    }

    static inline u32 GetShaderDest() { return RC_dest_vertex; }
};

template <>
struct ShaderTypeTraits<SPS>
{
    using MapType = CResourceManager::map_PS;

#if defined(USE_DX9)
    using LinkageType = void*;
#endif
    using HWShaderType = ID3DPixelShader*;
    using BufferType = DWORD const*;
    using ResultType = HRESULT;

    static inline const char* GetShaderExt() { return ".ps"; }

    static inline const char* GetCompilationTarget()
    {
        return HW.Caps.raster_profile;
    }

    static void GetCompilationTarget(const char*& target, const char*& entry, const char* data)
    {
        entry = "main";
        if (strstr(data, "main_ps_1_1"))
        {
            target = "ps_1_1";
            entry = "main_ps_1_1";
        }
        else if (strstr(data, "main_ps_1_2"))
        {
            target = "ps_1_2";
            entry = "main_ps_1_2";
        }
        else if (strstr(data, "main_ps_1_3"))
        {
            target = "ps_1_3";
            entry = "main_ps_1_3";
        }
        else if (strstr(data, "main_ps_1_4"))
        {
            target = "ps_1_4";
            entry = "main_ps_1_4";
        }
        else if (strstr(data, "main_ps_2_0"))
        {
            target = "ps_2_0";
            entry = "main_ps_2_0";
        }
#ifdef USE_DX9 // For DX10+ we always should use SM4.0 or higher
        else
#endif
        {
            target = GetCompilationTarget();
        }
    }

    static inline ResultType CreateHWShader(BufferType buffer, size_t size, HWShaderType& sh,
        LinkageType linkage = nullptr, pcstr name = nullptr)
    {
        ResultType res{};

#if defined(USE_DX9)
        res = HW.pDevice->CreatePixelShader(buffer, &sh);
        UNUSED(linkage, name);
#else
#       error No graphics API selected or enabled!
#endif

        return res;
    }

    static inline u32 GetShaderDest() { return RC_dest_pixel; }
};

template <>
inline CResourceManager::map_PS& CResourceManager::GetShaderMap()
{
    return m_ps;
}

template <>
inline CResourceManager::map_VS& CResourceManager::GetShaderMap()
{
    return m_vs;
}

template <typename T>
T* CResourceManager::CreateShader(cpcstr name, pcstr filename /*= nullptr*/, u32 flags /*= 0*/)
{
    typename ShaderTypeTraits<T>::MapType& sh_map = GetShaderMap<typename ShaderTypeTraits<T>::MapType>();
    pstr N = pstr(name);
    auto iterator = sh_map.find(N);

    if (iterator != sh_map.end())
        return iterator->second;
    else
    {
        T* sh = xr_new<T>();

        sh->dwFlags |= xr_resource_flagged::RF_REGISTERED;
        sh_map.emplace(sh->set_name(name), sh);
        if (0 == xr_stricmp(name, "null"))
        {
            sh->sh = 0;
            return sh;
        }

        // Remove ( and everything after it
        string_path shName;
        {
            if (filename == nullptr)
                filename = name;

            pcstr pchr = strchr(filename, '(');
            ptrdiff_t size = pchr ? pchr - filename : xr_strlen(filename);
            strncpy(shName, filename, size);
            shName[size] = 0;
        }

        // Open file
        string_path cname;
        strconcat(cname, RImplementation.getShaderPath(), shName,
            ShaderTypeTraits<T>::GetShaderExt());
        FS.update_path(cname, "$game_shaders$", cname);

        // Try to open
        IReader* file = FS.r_open(cname);

        // Here we can fallback to "stub_default"
        bool fallback = m_shader_fallback_allowed;
        if (!file && fallback)
        {
        fallback:
            fallback = false;

            string_path tmp;
            strconcat(tmp, "stub_default", ShaderTypeTraits<T>::GetShaderExt());

            Msg("CreateShader: %s is missing. Replacing it with %s", cname, tmp);
            strconcat(cname, RImplementation.getShaderPath(), tmp);
            FS.update_path(cname, "$game_shaders$", cname);
            file = FS.r_open(cname);
        }
        R_ASSERT3(file, "Shader file doesnt exist", cname);

        // Duplicate and zero-terminate
        const auto size = file->length();
        char* const data = (pstr)xr_alloca(size + 1);
        CopyMemory(data, file->pointer(), size);
        data[size] = 0;

        // Select target
        pcstr c_target, c_entry;
        ShaderTypeTraits<T>::GetCompilationTarget(c_target, c_entry, data);

#if defined(USE_D3DX)
#   ifdef NDEBUG
        flags |= D3DXSHADER_PACKMATRIX_ROWMAJOR;
#   else
        flags |= D3DXSHADER_PACKMATRIX_ROWMAJOR | (xrDebug::DebuggerIsPresent() ? D3DXSHADER_DEBUG : 0);
#   endif
#elif defined(USE_DX9)
#   ifdef NDEBUG
        flags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_OPTIMIZATION_LEVEL3;
#   else
        flags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_DEBUG;
#   endif
#endif

        // Compile
        HRESULT const _hr = RImplementation.shader_compile(name, file, c_entry, c_target, flags, (void*&)sh);

        FS.r_close(file);

        VERIFY(SUCCEEDED(_hr));

        if (FAILED(_hr) && fallback)
            goto fallback;

        CHECK_OR_EXIT(!FAILED(_hr), "Your video card doesn't meet game requirements.\n\nTry to lower game settings.");

        return sh;
    }
}

template <typename T>
bool CResourceManager::DestroyShader(const T* sh)
{
    if (0 == (sh->dwFlags & xr_resource_flagged::RF_REGISTERED))
        return false;

    typename ShaderTypeTraits<T>::MapType& sh_map = GetShaderMap<typename ShaderTypeTraits<T>::MapType>();

    pstr N = pstr(*sh->cName);
    auto iterator = sh_map.find(N);

    if (iterator != sh_map.end())
    {
        sh_map.erase(iterator);
        return true;
    }

    Msg("! ERROR: Failed to find compiled shader '%s'", sh->cName.c_str());
    return false;
}
