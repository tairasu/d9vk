#include "d3d9_cursor.h"
#include "d3d9_util.h"

#include <utility>
#include <vector>

namespace dxvk {

#ifdef _WIN32
  void D3D9Cursor::UpdateCursor(int X, int Y) {
    POINT currentPos = { };
    if (::GetCursorPos(&currentPos) && currentPos == POINT{ X, Y })
        return;

    ::SetCursorPos(X, Y);
  }


  BOOL D3D9Cursor::ShowCursor(BOOL bShow) {
    if (likely(m_hCursor != nullptr))
      ::SetCursor(bShow ? m_hCursor : nullptr);
    else
      Logger::debug("D3D9Cursor::ShowCursor: Software cursor not implemented.");
    
    return std::exchange(m_visible, bShow);
  }


  HRESULT D3D9Cursor::SetHardwareCursor(UINT XHotSpot, UINT YHotSpot, const uint8_t* bitmap, UINT width, UINT height) {
    std::vector<DWORD> mask(height, ~0u);

    ICONINFO info;
    info.fIcon    = FALSE;
    info.xHotspot = XHotSpot;
    info.yHotspot = YHotSpot;
    info.hbmMask  = ::CreateBitmap(width, height, 1, 1,  mask.data());
    info.hbmColor = ::CreateBitmap(width, height, 1, 32, bitmap);

    if (m_hCursor != nullptr)
      ::DestroyCursor(m_hCursor);

    m_hCursor = ::CreateIconIndirect(&info);

    ::DeleteObject(info.hbmMask);
    ::DeleteObject(info.hbmColor);

    ShowCursor(m_visible);

    return D3D_OK;
  }
#else
  void D3D9Cursor::UpdateCursor(int X, int Y) {
    Logger::warn("D3D9Cursor::UpdateCursor: Not supported on current platform.");
  }


  BOOL D3D9Cursor::ShowCursor(BOOL bShow) {
    Logger::warn("D3D9Cursor::ShowCursor: Not supported on current platform.");
    return std::exchange(m_visible, bShow);
  }


  HRESULT D3D9Cursor::SetHardwareCursor(UINT XHotSpot, UINT YHotSpot, const uint8_t* bitmap, UINT width, UINT height) {
    Logger::warn("D3D9Cursor::SetHardwareCursor: Not supported on current platform.");

    return D3D_OK;
  }
#endif

}
