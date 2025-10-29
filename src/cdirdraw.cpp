///////////////////////////////////////////////////////////
// CDIRDRAW.CPP -- Implementation file for the CDirDraw
//                 class.
///////////////////////////////////////////////////////////

#include "cdirdraw.h"
extern int backgroundcolour;
///////////////////////////////////////////////////////////
// CDirDraw::CDirDraw()
//
// This is the class's constructor.
///////////////////////////////////////////////////////////
CDirDraw::CDirDraw(HWND hWnd)
{
    // Initialize class data members.
    m_hWindow = hWnd;
    m_pDirectDrawObj = NULL;
    m_pDirectDrawPal = NULL;
    m_pPrimarySurface = NULL;
    m_pBackBuffer = NULL;
    m_currentSurfaceNum = 0;

    for (UINT x = 0; x < 101; ++x)
    {
        m_surfacePointers[x] = NULL;
        m_surfaceDibPointers[x] = NULL;
        m_colorKeySet[x] = FALSE;
    }

    // Create a DirectDraw object.
    HRESULT result =
        DirectDrawCreate(NULL, &m_pDirectDrawObj, NULL);

    // If the DirectDraw object is created successfully...
    if (result == DD_OK)
    {
        // Create surfaces, set screen resolution, etc.
        BOOL success = InitDirectDraw();

        // If all initialization tasks are not
        // successful, release all DirectDraw objects.
        if (!success)
        {
            ReleaseAll();
            m_pDirectDrawObj = NULL;
        }
    }
    else
        m_pDirectDrawObj = NULL;
}

///////////////////////////////////////////////////////////
// CDirDraw::~CDirDraw()
//
// This is the class's destructor.
///////////////////////////////////////////////////////////
CDirDraw::~CDirDraw()
{
    if (m_pDirectDrawObj != NULL)
    {
        m_pDirectDrawObj->FlipToGDISurface();
        ReleaseAll();
    }
}

///////////////////////////////////////////////////////////
// CDirDraw::InitDirectDraw()
//
// This function initializes the program's DirectDraw
// objects, including the main object and surfaces. The
// function creates a primary surface with a single
// back buffer.
///////////////////////////////////////////////////////////
BOOL CDirDraw::InitDirectDraw()
{
    //    Set the Direct Draw object to exclusive mode.
    HRESULT result = m_pDirectDrawObj->
        SetCooperativeLevel(m_hWindow,
            DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);


    if (result != DD_OK)
        return FALSE;

    // Set the display mode to 640x480 with 256 colors.


    result = m_pDirectDrawObj->SetDisplayMode(1024, 768, 8);


    //   result = m_pDirectDrawObj->SetDisplayMode(1920,1080, 8);

    //	result = m_pDirectDrawObj->SetDisplayMode(1440, 900, 8);
    if (result != DD_OK)
        return FALSE;

    // Create the primary and back-buffer surfaces.
    BOOL success = CreateMainSurfaces();
    if (!success)
        return FALSE;

    // Clear the back buffer.
    success = ClearSurface(BACKBUFFERNUM);
    if (!success)
        return FALSE;

    // If we got this far, everything went OK!
    return TRUE;
}

///////////////////////////////////////////////////////////
// CDirDraw::ReleaseAll()
//
// This member function releases all the DirectDraw objects
// created in the class.
///////////////////////////////////////////////////////////
void CDirDraw::ReleaseAll()
{
    // Release the DirectDrawPalette object.
    if (m_pDirectDrawPal != NULL)
        m_pDirectDrawPal->Release();

    // Release the primary DirectDrawSurface object.
    if (m_pPrimarySurface != NULL)
        m_pPrimarySurface->Release();

    // Release all off-screen DirectDrawSurface objects.
    for (UINT x = 1; x <= m_currentSurfaceNum; ++x)
        m_surfacePointers[x]->Release();

    // Release the DirectDraw object.
    if (m_pDirectDrawObj != NULL)
        m_pDirectDrawObj->Release();
}

///////////////////////////////////////////////////////////
// CDirDraw::SetPalette()
//
// This member function creates a DirectDrawPalette object
// from the color table found in the CDib object passed as
// the function's parameter. After creating the palette
// object, this function sets the palette for the primary
// DirectDrawSurface object.
///////////////////////////////////////////////////////////
BOOL CDirDraw::SetPalette(CDib* pDib)
{
    BOOL success = TRUE;
    PALETTEENTRY palEntries[256];
    LPRGBQUAD pColorTable = pDib->GetDibRGBTablePtr();

    // If a palette already exists, release it.
    if (m_pDirectDrawPal != NULL)
        m_pDirectDrawPal->Release();

    // Copy the bitmap's color table
    // into the palette entries array.
    for (int x = 0; x < 256; ++x)
    {
        palEntries[x].peRed = pColorTable[x].rgbRed;
        palEntries[x].peGreen = pColorTable[x].rgbGreen;
        palEntries[x].peBlue = pColorTable[x].rgbBlue;
    }

    // Create a DirectDrawPalette object.
    HRESULT result = m_pDirectDrawObj->CreatePalette(DDPCAPS_8BIT,
        palEntries, &m_pDirectDrawPal, NULL);

    // If the palette is created unsuccessfully,
    // make the palette's pointer NULL.
    if (result != DD_OK)
    {
        m_pDirectDrawPal = NULL;
        success = FALSE;
    }
    else
    {
        result = m_pPrimarySurface->SetPalette(m_pDirectDrawPal);
        if (result != DD_OK)
            success = FALSE;
    }

    return success;
}

///////////////////////////////////////////////////////////
// CDirDraw::CreateMainSurfaces()
//
// This function creates a primary DirectDrawSurface object
// with a single back buffer.
///////////////////////////////////////////////////////////
BOOL CDirDraw::CreateMainSurfaces()
{
    // Initialize the surface description structure
    // for a primary surface with one back buffer.
    DDSURFACEDESC ddSurfaceDesc;
    memset(&ddSurfaceDesc, 0, sizeof(DDSURFACEDESC));
    ddSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
    ddSurfaceDesc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
        DDSCAPS_FLIP | DDSCAPS_COMPLEX;
    ddSurfaceDesc.dwBackBufferCount = 1;

    // Create the primary surface.
    HRESULT result = m_pDirectDrawObj->
        CreateSurface(&ddSurfaceDesc, &m_pPrimarySurface, NULL);

    if (result != DD_OK)
        return FALSE;

    // Retrieve a pointer to the back buffer.
    DDSCAPS ddsCaps;
    ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
    result = m_pPrimarySurface->
        GetAttachedSurface(&ddsCaps, &m_pBackBuffer);

    if (result != DD_OK)
        return FALSE;

    return TRUE;
}

///////////////////////////////////////////////////////////
// CDirDraw::ClearSurface()
//
// This function blacks out a DirectDraw surface.
///////////////////////////////////////////////////////////
BOOL CDirDraw::ClearSurface(UINT surfaceNum)
{
    // Check that the surface number is valid.
    if (surfaceNum > m_currentSurfaceNum)
        return FALSE;

    LPDIRECTDRAWSURFACE pSurface;
    BOOL success = FALSE;

    // Get a pointer to the requested surface.
    if (surfaceNum == BACKBUFFERNUM)
        pSurface = m_pBackBuffer;
    else
        pSurface = m_surfacePointers[surfaceNum];

    // Initialize the surface description structure.
    DDSURFACEDESC ddSurfaceDesc;
    memset(&ddSurfaceDesc, 0, sizeof(DDSURFACEDESC));
    ddSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);

    HRESULT result;
    BOOL exitLoop = FALSE;
    do
    {
        // Attempt to lock the surface.
        result = pSurface->Lock(NULL, &ddSurfaceDesc,
            DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

        // If the lock failed due to the loss
        // of the surface, restore the surface
        // and try again.
        if (result == DDERR_SURFACELOST)
        {
            m_pPrimarySurface->Restore();
            RestoreOffScreenSurfaces();
        }
        else
            exitLoop = TRUE;
    } while (!exitLoop);
    while (!exitLoop);

    // If the surface locked okay...
    if (result == DD_OK)
    {
        UINT surfaceWidth = ddSurfaceDesc.lPitch;
        UINT surfaceHeight = ddSurfaceDesc.dwHeight;

        // Clear the buffer's area in memory.
        char* buf = (char*)ddSurfaceDesc.lpSurface;
        memset(buf, backgroundcolour, surfaceWidth * surfaceHeight);

        // Unlock the back buffer.
        pSurface->Unlock(ddSurfaceDesc.lpSurface);
        success = TRUE;
    }

    return success;
}

///////////////////////////////////////////////////////////
// CDirDraw::FlipSurfaces()
//
// This function swaps the primary and back-buffer
// surface memory.
///////////////////////////////////////////////////////////
BOOL CDirDraw::FlipSurfaces()
{
    HRESULT result;

    // Keep trying to flip until Direct Draw is done drawing.
    BOOL exitLoop = FALSE;
    do
    {
        // Attempt to flip the surface.
        result = m_pPrimarySurface->Flip(NULL, DDFLIP_WAIT);

        // If the flip failed due to the loss
        // of the surface, restore the surface
        // and try again.
        if (result == DDERR_SURFACELOST)
        {
            m_pPrimarySurface->Restore();
            RestoreOffScreenSurfaces();
        }
        else
            exitLoop = TRUE;
    } while (!exitLoop);

    if (result != DD_OK)
        return FALSE;

    return TRUE;
}

///////////////////////////////////////////////////////////
// CDirDraw::CopyDibToSurface()
//
// This function copies a bitmap image to a DirectDraw
// surface.
///////////////////////////////////////////////////////////
BOOL CDirDraw::CopyDibToSurface(UINT surfaceNum,
    CPoint* dstPoint, CDib* pDib, CRect* srcRect)
{
    // Check for a valid surface number.
    if (surfaceNum > m_currentSurfaceNum)
        return FALSE;

    // Remember which DIB is stored in the
    // requested surface.
    m_surfaceDibPointers[surfaceNum] = pDib;

    // Get a pointer to the requested surface.
    LPDIRECTDRAWSURFACE pSurface;
    if (surfaceNum == BACKBUFFERNUM)
        pSurface = m_pBackBuffer;
    else
        pSurface = m_surfacePointers[surfaceNum];

    // Initialize the DDSURFACEDESC structure.
    DDSURFACEDESC ddSurfaceDesc;
    memset(&ddSurfaceDesc, 0, sizeof(DDSURFACEDESC));
    ddSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);

    // Lock the destination surface.
    HRESULT result;
    BOOL exitLoop = FALSE;
    do
    {
        // Attempt to lock the surface.
        result = pSurface->Lock(NULL, &ddSurfaceDesc,
            DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

        // If the lock failed due to the loss
        // of the surface, restore the surface
        // and try again.
        if (result == DDERR_SURFACELOST)
        {
            m_pPrimarySurface->Restore();
            RestoreOffScreenSurfaces();
        }
        else
            exitLoop = TRUE;
    } while (!exitLoop);

    if (result != DD_OK)
        return FALSE;

    // Get the surface width.
    UINT surfaceWidth = ddSurfaceDesc.dwWidth;

    // Get a pointer to the surface's memory, adjusting
    // for the requested destination point.
    char* pSurfaceBits = (char*)ddSurfaceDesc.lpSurface +
        dstPoint->x + dstPoint->y * ddSurfaceDesc.lPitch;

    // Get the bitmap's width and height.
    UINT dibWidth = pDib->GetDibWidth();
    UINT dibHeight = pDib->GetDibHeight();

    // Get the width and height of the source rectangle.
    UINT srcHeight = (UINT)srcRect->Height();
    UINT srcWidth = (UINT)srcRect->Width();

    // Get a pointer to the bitmap's image, adjusting
    // for bottom-up and for the source rectangle.
    char* pDibBits = (((char*)pDib->GetDibBitsPtr()) +
        (dibHeight - 1) * dibWidth) -
        srcRect->top * dibWidth + srcRect->left;

    // Copy the source rectangle to the
    // destination point.
    for (UINT x = 0; x < srcHeight; ++x)
    {
        // Copy one line.
        memcpy(pSurfaceBits, pDibBits, srcWidth);

        // Calculate pointers to the next line.
        pSurfaceBits += ddSurfaceDesc.lPitch;
        pDibBits -= dibWidth;
    }

    // Unlock the destination surface.
    pSurface->Unlock(NULL);

    return TRUE;
}

///////////////////////////////////////////////////////////
// CDirDraw::CreateOffScreenSurface()
//
// This member function creates plain off-screen surfaces.
// The class can accommodate up to 100 such surfaces.
///////////////////////////////////////////////////////////
UINT CDirDraw::CreateOffScreenSurface(UINT width, UINT height)
{
    // Make sure there's room for another surface.
    if (m_currentSurfaceNum == 100)
        return 0;

    // Calculate the next available surface number.
    ++m_currentSurfaceNum;

    // Initialize the DDSURFACEDESC structure.
    DDSURFACEDESC ddSurfaceDesc;
    memset(&ddSurfaceDesc, 0, sizeof(DDSURFACEDESC));
    ddSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
    ddSurfaceDesc.dwFlags = DDSD_CAPS |
        DDSD_HEIGHT | DDSD_WIDTH;
    ddSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    ddSurfaceDesc.dwWidth = width;
    ddSurfaceDesc.dwHeight = height;

    // Create the new surface.
    HRESULT result = m_pDirectDrawObj->
        CreateSurface(&ddSurfaceDesc,
            &m_surfacePointers[m_currentSurfaceNum], NULL);

    if (result != DD_OK)
        return 0;

    return m_currentSurfaceNum;
}

///////////////////////////////////////////////////////////
// CDirDraw::SetTransparentColors()
//
// This member function sets transparent colors for direct
// draw surfaces. The transparent colors will not be copied
// when blitting images from the surface. That is, the
// colors defined in this function are source color keys.
///////////////////////////////////////////////////////////
BOOL CDirDraw::SetTransparentColors(UINT surfaceNum,
    UINT lowColor, UINT hiColor)
{
    // Check for a valid surface number.
    if (surfaceNum > m_currentSurfaceNum)
        return FALSE;

    // Get a pointer to the requested surface.
    LPDIRECTDRAWSURFACE pSurface;
    if (surfaceNum == BACKBUFFERNUM)
        pSurface = m_pBackBuffer;
    else
        pSurface = m_surfacePointers[surfaceNum];

    // Set the color key for the transparent colors.
    DDCOLORKEY ddColorKey;
    ddColorKey.dwColorSpaceLowValue = lowColor;
    ddColorKey.dwColorSpaceHighValue = hiColor;
    HRESULT result = pSurface->
        SetColorKey(DDCKEY_SRCBLT, &ddColorKey);

    if (result != DD_OK)
        return FALSE;

    // Remember that this surface has set a color key.
    m_colorKeySet[surfaceNum] = TRUE;

    return TRUE;
}

///////////////////////////////////////////////////////////
// CDirDraw::DirectDrawOK()
//
// This member function returns TRUE if the DirectDraw
// object was created and initialized okay. Otherwise, it
// returns FALSE.
///////////////////////////////////////////////////////////
BOOL CDirDraw::DirectDrawOK()
{
    if (m_pDirectDrawObj == NULL)
        return FALSE;

    return TRUE;
}

///////////////////////////////////////////////////////////
// CDirDraw::RestoreOffScreenSurfaces()
//
// This member function restores all lost off-screen
// surfaces, the pointers to which are stored in the
// m_surfacePointers[] array.
///////////////////////////////////////////////////////////
void CDirDraw::RestoreOffScreenSurfaces()
{
    // If there are no surfaces to
    // restore, return immediately.
    if (m_currentSurfaceNum == 0)
        return;

    // Restore all off-screen surfaces.
    for (UINT x = 1; x <= m_currentSurfaceNum; ++x)
        m_surfacePointers[x]->Restore();

    // Repaint all off-screen surfaces.
    RepaintOffScreenSurfaces();
}

///////////////////////////////////////////////////////////
// CDirDraw::BlitImage()
//
// This member function blits all or part of a bitmap from
// a DirectDraw off-screen surface to the back-buffer
// surface.
///////////////////////////////////////////////////////////
BOOL CDirDraw::BlitImage(CPoint* point, UINT surfaceNum, CRect* rect)
{
    // Check for a valid surface number.
    if ((surfaceNum > m_currentSurfaceNum) || (surfaceNum == 0))
        return FALSE;

    // Get a pointer to the requested surface.
    LPDIRECTDRAWSURFACE pSourceSurface =
        m_surfacePointers[surfaceNum];

    // Determine whether the requested surface
    // has set a color key.
    DWORD transFlag = DDBLTFAST_NOCOLORKEY;
    if (m_colorKeySet[surfaceNum])
        transFlag = DDBLTFAST_SRCCOLORKEY;

    HRESULT result;
    BOOL exitLoop = FALSE;
    do
    {
        // Attempt to perform the blit.
        result = m_pBackBuffer->BltFast(point->x, point->y,
            pSourceSurface, rect, transFlag | DDBLTFAST_WAIT);

        // If the blit failed due to the loss
        // of the surfaces, restore the surfaces
        // and try again.
        if (result == DDERR_SURFACELOST)
        {
            m_pPrimarySurface->Restore();
            RestoreOffScreenSurfaces();
        }
        else
            exitLoop = TRUE;
    } while (!exitLoop);

    if (result != DD_OK)
        return FALSE;

    return TRUE;
}

///////////////////////////////////////////////////////////
// CDirDraw::RepaintOffScreenSurfaces()
//
// This member function restores the graphical contents
// of off-screen surfaces after they've been lost. If a
// program needs to repaint its off-screen surfaces, this
// virtual function should be overridden in a class derived
// from CDirDraw.
///////////////////////////////////////////////////////////
void CDirDraw::RepaintOffScreenSurfaces()
{
}


