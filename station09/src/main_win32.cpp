// STATION 09: ECHO  --  Windows entry point (no external libraries: Win32 + GDI + waveOut)
// NOTE: game headers are included BEFORE <windows.h> on purpose, so that Windows' macros
// (near, far, S_OK ...) can never collide with identifiers inside the game code.
#include "game_render.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>

static S9::Game* gGame = nullptr;
static S9::Input gIn;
static HWND gWnd = nullptr;
static bool gQuit = false, gActive = true, gFullscreen = false, gResetMouse = true, gCursorHidden = false;
static RECT gSavedRect;
static LONG gSavedStyle = 0;

// ---------------- audio: 4 x 512-frame buffers, refilled by a worker thread ----------------
static const int AB_FRAMES = 512;
static const int AB_COUNT = 4;
static HWAVEOUT gWave = nullptr;
static WAVEHDR gHdr[AB_COUNT];
static short gABuf[AB_COUNT][AB_FRAMES * 2];
static volatile bool gAudioRun = false;
static HANDLE gAudioThread = nullptr;

static DWORD WINAPI audioProc(LPVOID) {
    while (gAudioRun) {
        bool did = false;
        for (int i = 0; i < AB_COUNT; i++) {
            if (gHdr[i].dwFlags & WHDR_DONE) {
                S9::audio.mix(gABuf[i], AB_FRAMES);
                waveOutWrite(gWave, &gHdr[i], sizeof(WAVEHDR));
                did = true;
            }
        }
        if (!did) Sleep(3);
    }
    return 0;
}

static void startAudio() {
    WAVEFORMATEX wf;
    ZeroMemory(&wf, sizeof(wf));
    wf.wFormatTag = WAVE_FORMAT_PCM;
    wf.nChannels = 2;
    wf.nSamplesPerSec = S9::SR;
    wf.wBitsPerSample = 16;
    wf.nBlockAlign = (WORD)(wf.nChannels * wf.wBitsPerSample / 8);
    wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
    if (waveOutOpen(&gWave, WAVE_MAPPER, &wf, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) { gWave = nullptr; return; }
    for (int i = 0; i < AB_COUNT; i++) {
        ZeroMemory(&gHdr[i], sizeof(WAVEHDR));
        gHdr[i].lpData = (LPSTR)gABuf[i];
        gHdr[i].dwBufferLength = AB_FRAMES * 2 * sizeof(short);
        waveOutPrepareHeader(gWave, &gHdr[i], sizeof(WAVEHDR));
        S9::audio.mix(gABuf[i], AB_FRAMES);
        waveOutWrite(gWave, &gHdr[i], sizeof(WAVEHDR));
    }
    gAudioRun = true;
    gAudioThread = CreateThread(nullptr, 0, audioProc, nullptr, 0, nullptr);
    if (gAudioThread) SetThreadPriority(gAudioThread, THREAD_PRIORITY_ABOVE_NORMAL);
}

static void stopAudio() {
    gAudioRun = false;
    if (gAudioThread) { WaitForSingleObject(gAudioThread, 500); CloseHandle(gAudioThread); gAudioThread = nullptr; }
    if (gWave) {
        waveOutReset(gWave);
        for (int i = 0; i < AB_COUNT; i++) waveOutUnprepareHeader(gWave, &gHdr[i], sizeof(WAVEHDR));
        waveOutClose(gWave);
        gWave = nullptr;
    }
}

// ---------------- window ----------------
static void setCursorHidden(bool hide) {
    if (hide && !gCursorHidden) { ShowCursor(FALSE); gCursorHidden = true; }
    if (!hide && gCursorHidden) { ShowCursor(TRUE); gCursorHidden = false; }
}

static void toggleFullscreen() {
    if (!gFullscreen) {
        gSavedStyle = GetWindowLong(gWnd, GWL_STYLE);
        GetWindowRect(gWnd, &gSavedRect);
        MONITORINFO mi;
        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        GetMonitorInfo(MonitorFromWindow(gWnd, MONITOR_DEFAULTTONEAREST), &mi);
        SetWindowLong(gWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowPos(gWnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        gFullscreen = true;
    } else {
        SetWindowLong(gWnd, GWL_STYLE, gSavedStyle);
        SetWindowPos(gWnd, HWND_NOTOPMOST, gSavedRect.left, gSavedRect.top, gSavedRect.right - gSavedRect.left,
                     gSavedRect.bottom - gSavedRect.top, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        gFullscreen = false;
    }
    gResetMouse = true;
}

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    switch (m) {
        case WM_CLOSE: gQuit = true; return 0;
        case WM_DESTROY: PostQuitMessage(0); return 0;
        case WM_ACTIVATE:
            gActive = LOWORD(w) != WA_INACTIVE;
            if (!gActive) { memset(gIn.key, 0, sizeof(gIn.key)); memset(gIn.mouse, 0, sizeof(gIn.mouse)); }
            gResetMouse = true;
            return 0;
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN: {
            int k = (int)(w & 255);
            if (m == WM_SYSKEYDOWN && k == VK_F4) return DefWindowProc(h, m, w, l);
            if (k == VK_F11) { if (!(l & (1 << 30))) toggleFullscreen(); return 0; }
            gIn.key[k] = true;
            return 0;
        }
        case WM_SYSKEYUP:
        case WM_KEYUP: gIn.key[(int)(w & 255)] = false; return 0;
        case WM_LBUTTONDOWN: gIn.mouse[0] = true; return 0;
        case WM_LBUTTONUP: gIn.mouse[0] = false; return 0;
        case WM_RBUTTONDOWN: gIn.mouse[1] = true; return 0;
        case WM_RBUTTONUP: gIn.mouse[1] = false; return 0;
        case WM_SETCURSOR:
            if (LOWORD(l) == HTCLIENT && gActive) { SetCursor(nullptr); return TRUE; }
            break;
        case WM_ERASEBKGND: return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(h, &ps);
            EndPaint(h, &ps);
            return 0;
        }
    }
    return DefWindowProc(h, m, w, l);
}

static void present(HDC dc) {
    RECT rc;
    GetClientRect(gWnd, &rc);
    int cw = rc.right - rc.left, ch = rc.bottom - rc.top;
    if (cw <= 0 || ch <= 0) return;
    // keep 4:3, integer-ish scaling with nearest neighbour so the pixels stay crunchy
    float sc = (float)cw / S9::SW;
    if ((float)ch / S9::SH < sc) sc = (float)ch / S9::SH;
    int dw = (int)(S9::SW * sc), dh = (int)(S9::SH * sc);
    int dx = (cw - dw) / 2, dy = (ch - dh) / 2;
    if (dx > 0 || dy > 0) {
        PatBlt(dc, 0, 0, cw, dy, BLACKNESS);
        PatBlt(dc, 0, dy + dh, cw, ch - dy - dh, BLACKNESS);
        PatBlt(dc, 0, dy, dx, dh, BLACKNESS);
        PatBlt(dc, dx + dw, dy, cw - dx - dw, dh, BLACKNESS);
    }
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = S9::SW;
    bmi.bmiHeader.biHeight = -S9::SH;  // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    SetStretchBltMode(dc, COLORONCOLOR);
    StretchDIBits(dc, dx, dy, dw, dh, 0, 0, S9::SW, S9::SH, S9::gfx.out, &bmi, DIB_RGB_COLORS, SRCCOPY);
}

static void showLoading(HDC dc, const char* msg) {
    RECT rc;
    GetClientRect(gWnd, &rc);
    FillRect(dc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
    SetTextColor(dc, RGB(170, 40, 40));
    SetBkMode(dc, TRANSPARENT);
    DrawTextA(dc, msg, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
    // crisp scaling on high-DPI displays
    typedef BOOL(WINAPI * SetDpiAwareFn)(void);
    HMODULE user32 = GetModuleHandleA("user32.dll");
    if (user32) {
        SetDpiAwareFn fn = (SetDpiAwareFn)GetProcAddress(user32, "SetProcessDPIAware");
        if (fn) fn();
    }
    timeBeginPeriod(1);

    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = "Station09Class";
    RegisterClassA(&wc);

    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int scale = (screenH - 120) / S9::SH;
    if (scale < 2) scale = 2;
    if (scale > 4) scale = 4;
    RECT r = {0, 0, S9::SW * scale, S9::SH * scale};
    DWORD style = WS_OVERLAPPEDWINDOW;
    AdjustWindowRect(&r, style, FALSE);
    gWnd = CreateWindowA("Station09Class", "STATION 09: ECHO", style | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                         r.right - r.left, r.bottom - r.top, nullptr, nullptr, hInst, nullptr);
    if (!gWnd) return 1;
    HDC dc = GetDC(gWnd);
    showLoading(dc, "STATION 09: ECHO\r\n\r\nLoading... (synthesizing voices)");
    MSG msg;
    while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageA(&msg); }

    gGame = new S9::Game();
    gGame->init();
    startAudio();

    LARGE_INTEGER freq, last, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&last);

    while (!gQuit && !gGame->quit) {
        while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) gQuit = true;
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        QueryPerformanceCounter(&now);
        float dt = (float)((double)(now.QuadPart - last.QuadPart) / (double)freq.QuadPart);
        last = now;
        if (dt > 0.1f) dt = 0.1f;

        // mouse look: re-centre the cursor every frame and use the delta
        gIn.mdx = gIn.mdy = 0;
        if (gActive) {
            setCursorHidden(true);
            RECT rc;
            GetClientRect(gWnd, &rc);
            POINT c = {(rc.right - rc.left) / 2, (rc.bottom - rc.top) / 2};
            ClientToScreen(gWnd, &c);
            POINT p;
            GetCursorPos(&p);
            if (gResetMouse) gResetMouse = false;
            else { gIn.mdx = (float)(p.x - c.x); gIn.mdy = (float)(p.y - c.y); }
            SetCursorPos(c.x, c.y);
        } else {
            setCursorHidden(false);
            if (gGame->mode == S9::M_PLAY) gGame->mode = S9::M_PAUSE;  // auto-pause when you alt-tab
        }

        gGame->update(dt, gIn);
        gGame->render();
        present(dc);

        // ~60 fps limiter
        for (;;) {
            LARGE_INTEGER t2;
            QueryPerformanceCounter(&t2);
            double ms = (double)(t2.QuadPart - last.QuadPart) * 1000.0 / (double)freq.QuadPart;
            if (ms >= 16.0) break;
            Sleep(1);
        }
    }
    stopAudio();
    setCursorHidden(false);
    ReleaseDC(gWnd, dc);
    timeEndPeriod(1);
    return 0;
}
