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


  HRESULT D3D9Cursor::SetHardwareCursor(UINT XHotSpot, UINT YHotSpot, const CursorBitmap& bitmap, uint32_t scale) {
    uint32_t cursorScale = scale == 0 ? 1u : scale;
    uint32_t outWidth  = HardwareCursorWidth  * cursorScale;
    uint32_t outHeight = HardwareCursorHeight * cursorScale;

    // Upscale color to ARGB32 buffer using nearest-neighbor
    std::vector<uint8_t> color(outWidth * outHeight * HardwareCursorFormatSize);
    for (uint32_t y = 0; y < outHeight; y++) {
      uint32_t srcY = y / cursorScale;
      const uint8_t* srcRow = &bitmap[srcY * HardwareCursorPitch];
      uint8_t* dstRow = &color[y * outWidth * HardwareCursorFormatSize];
      for (uint32_t x = 0; x < outWidth; x++) {
        uint32_t srcX = x / cursorScale;
        const uint8_t* srcPx = &srcRow[srcX * HardwareCursorFormatSize];
        uint8_t* dstPx = &dstRow[x * HardwareCursorFormatSize];
        dstPx[0] = srcPx[0];
        dstPx[1] = srcPx[1];
        dstPx[2] = srcPx[2];
        dstPx[3] = srcPx[3];
      }
    }

    // Create a fully opaque 1bpp mask of matching size
    uint32_t maskWordsPerRow = (outWidth + 31u) / 32u;
    std::vector<uint32_t> mask(maskWordsPerRow * outHeight, 0xFFFFFFFFu);

    ICONINFO info;
    info.fIcon    = FALSE;
    info.xHotspot = XHotSpot * cursorScale;
    info.yHotspot = YHotSpot * cursorScale;
    info.hbmMask  = ::CreateBitmap(outWidth, outHeight, 1, 1,  mask.data());
    info.hbmColor = ::CreateBitmap(outWidth, outHeight, 1, 32, color.data());

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
  }


  BOOL D3D9Cursor::ShowCursor(BOOL bShow) {
    return std::exchange(m_visible, bShow);
  }


  HRESULT D3D9Cursor::SetHardwareCursor(UINT XHotSpot, UINT YHotSpot, const CursorBitmap& bitmap, uint32_t scale) {
    return D3D_OK;
  }
#endif

}
