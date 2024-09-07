#include "stdafx.h"
#include "r__sync_point.h"

#include "QueryHelper.h"

#if defined(USE_DX9)
void R_sync_point::Create()
{
    for (u32 i = 0; i < HW.Caps.iGPUNum; ++i)
        R_CHK(CreateQuery((ID3DQuery**)&q_sync_point[i], D3D_QUERY_EVENT));
    // Prevent error on first get data
    CHK_DX(EndQuery((ID3DQuery*)q_sync_point[0]));
}

void R_sync_point::Destroy()
{
    for (u32 i = 0; i < HW.Caps.iGPUNum; ++i)
        R_CHK(ReleaseQuery((ID3DQuery*)q_sync_point[i]));
}

bool R_sync_point::Wait(u32 wait_sleep, u64 timeout)
{
    CTimer T;
    T.Start();
    BOOL result = FALSE;
    HRESULT hr = S_FALSE;
    while ((hr = GetData((ID3DQuery*)q_sync_point[q_sync_count], &result, sizeof(result))) == S_FALSE)
    {
        if (!SwitchToThread())
            Sleep(wait_sleep);
        if (T.GetElapsed_ms() > timeout)
        {
            result = FALSE;
            break;
        }
    }
    return result;
}

void R_sync_point::End()
{
    q_sync_count = (q_sync_count + 1) % HW.Caps.iGPUNum;
    CHK_DX(EndQuery((ID3DQuery*)q_sync_point[q_sync_count]));
}
#else
#   error No graphics API selected or enabled!
#endif
