#include "gol_win32.h"

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)
#define WIDTH      100
#define HEIGHT     50
#define CELL_SIZE  10
#define BASE_SPEED 100

struct Point {
	int x;
	int y;
};

bool board[WIDTH][HEIGHT];
bool paused = true;
bool grid = true;
int generation = 0;
double speedMultiplier = 1.0;
struct Point mousePoint;

HBRUSH colors[2];

const UINT IDT_GENERATION = 0;

static HICON icon;

void NextGeneration()
{
	bool newBoard[WIDTH][HEIGHT];

	for (int x = 0; x < WIDTH; x++) {
		for (int y = 0; y < HEIGHT; y++) {
			
			struct Point neighbors[] = {
				{ x - 1, y - 1 },
				{ x, y - 1 },
				{ x - 1, y },
				{ x + 1, y - 1 },
				{ x - 1, y + 1 },
				{ x + 1, y },
				{ x, y + 1 },
				{ x + 1, y + 1 }
			};
			
			int neighborCount = 0;
			for (int i = 0; i < static_cast<int>(sizeof(neighbors) / sizeof(neighbors[0])); i++) {
				if (
					neighbors[i].x >= 0 && neighbors[i].x < WIDTH &&
					neighbors[i].y >= 0 && neighbors[i].y < HEIGHT &&
					board[neighbors[i].x][neighbors[i].y] == 1
				) {
					neighborCount++;
				}
			}

			if (
				(board[x][y] == 1 && neighborCount == 2) ||
				neighborCount == 3
			) {
				newBoard[x][y] = 1;
			} else {
				newBoard[x][y] = 0;
			}

		}
	}

	memcpy(board, newBoard, sizeof(board));
	generation++;
}

RECT GameToScreenRect(int x, int y)
{
	RECT rect = {
		.left   = x * CELL_SIZE,
		.top    = y * CELL_SIZE,
		.right  = x * CELL_SIZE + CELL_SIZE,
		.bottom = y * CELL_SIZE + CELL_SIZE
	};
	return rect;
}

struct Point ScreenToGamePoint(int x, int y)
{
	x -= (x % CELL_SIZE);
	y -= (y % CELL_SIZE);
	x /= CELL_SIZE;
	y /= CELL_SIZE;
	struct Point point = { x, y };
	return point;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {

		case WM_CREATE: {
			// Get text metrics
			HDC hdc = GetDC(hwnd);
			TEXTMETRIC metrics;
			GetTextMetrics(hdc, &metrics);
			ReleaseDC(hwnd, hdc);

			// Calculate window size
			RECT rect;
			rect.left = 0;
			rect.top = 0;
			rect.right = WIDTH * CELL_SIZE;
			rect.bottom = HEIGHT * CELL_SIZE + metrics.tmHeight;
			AdjustWindowRect(&rect, WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, false);

			// Set the position
			SetWindowPos(
				hwnd, NULL, 0, 0,
				rect.right - rect.left, rect.bottom - rect.top,
				SWP_NOMOVE | SWP_NOZORDER
			);

			mousePoint = (struct Point){ -1, -1 };
		}

		case WM_TIMER: {
			NextGeneration();
			RECT rect;
			GetClientRect(hwnd, &rect);
			InvalidateRect(hwnd, &rect, false);
			return 0;
		}

		case WM_PAINT: {
			RECT updateRect;
			if (!GetUpdateRect(hwnd, &updateRect, false)) return 0;

			RECT clientRect;
			GetClientRect(hwnd, &clientRect);

			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			// Create memory DC for double buffering
			HDC hdcMem = CreateCompatibleDC(hdc);
			HBITMAP hbmMem = CreateCompatibleBitmap(hdc, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
			HDC hbmOld = reinterpret_cast<HDC>(SelectObject(hdcMem, hbmMem));

			struct Point updateStart = ScreenToGamePoint(updateRect.left, updateRect.top);
			struct Point updateEnd   = ScreenToGamePoint(updateRect.right, updateRect.bottom);

			// Draw cells
			for (int x = MIN(updateStart.x, 0); x < MAX(updateEnd.x, WIDTH); x++) {
				for (int y = MIN(updateStart.y, 0); y < MAX(updateEnd.y, HEIGHT); y++) {
					RECT rect = GameToScreenRect(x, y);
					FillRect(hdcMem, &rect, colors[board[x][y]]);
				}
			}

			// Draw grid
			if (grid) {
				HPEN grayPen = CreatePen(PS_SOLID, 0, RGB(128, 128, 128));
				SelectObject(hdcMem, grayPen);
				SelectObject(hdcMem, GetStockObject(NULL_BRUSH));

				Rectangle(hdcMem, 0, 0, WIDTH * CELL_SIZE, HEIGHT * CELL_SIZE);

				for (int x = 0; x < WIDTH; x++) {
					MoveToEx(hdcMem, x * CELL_SIZE, 0, NULL);
					LineTo(hdcMem, x * CELL_SIZE, HEIGHT * CELL_SIZE);
				}

				for (int y = 0; y < HEIGHT; y++) {
					MoveToEx(hdcMem, 0, y * CELL_SIZE, NULL);
					LineTo(hdcMem, WIDTH * CELL_SIZE, y * CELL_SIZE);
				}

			}
			
			// Draw status line
			if (updateRect.bottom >= HEIGHT * CELL_SIZE) {
				SetTextColor(hdcMem, RGB(255, 255, 255));
				SetBkColor(hdcMem, RGB(0, 0, 0));

				wchar_t statusLeft[64];
				StringCbPrintf(statusLeft, sizeof(statusLeft), L"Generation %d   (Speed x%g)", generation, speedMultiplier);
				TextOut(hdcMem, 0, HEIGHT * CELL_SIZE, statusLeft, wcslen(statusLeft));
				
				wchar_t statusRight[64];
				StringCbPrintf(statusRight, sizeof(statusRight), L"%d, %d", mousePoint.x, mousePoint.y);
				SetTextAlign(hdcMem, TA_RIGHT);
				TextOut(hdcMem, WIDTH * CELL_SIZE, HEIGHT * CELL_SIZE, statusRight, wcslen(statusRight));

				if (paused) {
					SetTextAlign(hdcMem, TA_CENTER);
					LPCWSTR pausedText = L"Paused";
					TextOut(hdcMem, (WIDTH * CELL_SIZE) / 2, HEIGHT * CELL_SIZE, pausedText, wcslen(pausedText));
				}
			}

			// Copy buffer and clean up
			BitBlt(hdc, clientRect.left, clientRect.top, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, hdcMem, clientRect.left, clientRect.top, SRCCOPY);
			SelectObject(hdcMem, hbmOld);
			DeleteObject(hbmMem);
			EndPaint(hwnd, &ps);
			return 0;
		}

		case WM_MOUSEMOVE: {
			struct Point point = ScreenToGamePoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			if (point.x >= 0 && point.x < WIDTH && point.y >= 0 && point.y < HEIGHT) {
				mousePoint = point;
				RECT statusRect;
				GetClientRect(hwnd, &statusRect);
				statusRect.top = HEIGHT * CELL_SIZE;
				InvalidateRect(hwnd, &statusRect, false);
			}
			return 0;
		}

		case WM_LBUTTONDOWN: {
			struct Point point = ScreenToGamePoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			if (point.x >= 0 && point.x < WIDTH && point.y >= 0 && point.y < HEIGHT) {
				board[point.x][point.y] = !board[point.x][point.y];
				RECT rect = GameToScreenRect(point.x, point.y);
				InvalidateRect(hwnd, &rect, false);
			}
			return 0;
		}

		case WM_KEYDOWN: {
			RECT statusRect, clientRect;
			GetClientRect(hwnd, &statusRect);
			GetClientRect(hwnd, &clientRect);
			statusRect.top = HEIGHT * CELL_SIZE;

			switch (wParam) {

				case VK_SPACE:
					if (paused) SetTimer(hwnd, IDT_GENERATION, BASE_SPEED / speedMultiplier, NULL);
					else KillTimer(hwnd, IDT_GENERATION);
					paused = !paused;
					InvalidateRect(hwnd, &statusRect, false);
					break;

				case VK_RIGHT:
					NextGeneration();
					InvalidateRect(hwnd, &clientRect, false);
					break;

				case VK_UP:
					speedMultiplier *= 2.0;
					if (!paused) {
						KillTimer(hwnd, IDT_GENERATION);
						SetTimer(hwnd, IDT_GENERATION, BASE_SPEED / speedMultiplier, NULL);
					}
					InvalidateRect(hwnd, &statusRect, false);
					break;

				case VK_DOWN:
					speedMultiplier /= 2.0;
					if (!paused) {
						KillTimer(hwnd, IDT_GENERATION);
						SetTimer(hwnd, IDT_GENERATION, BASE_SPEED / speedMultiplier, NULL);
					}
					InvalidateRect(hwnd, &statusRect, false);
					break;

				case 'R':
					if (MessageBox(hwnd, L"Are you sure you want to reset the game?", L"Reset", MB_YESNO | MB_ICONWARNING) == IDYES) {
						generation = 0;
						memset(board, 0, sizeof(board));
						paused = true;
						KillTimer(hwnd, IDT_GENERATION);
						InvalidateRect(hwnd, &clientRect, false);
					}
					break;

				case 'G':
					grid = !grid;
					InvalidateRect(hwnd, &clientRect, false);
					break;

			}
			return 0;
		}

		case WM_CLOSE: {
			DestroyWindow(hwnd);
			return 0;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		}
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

ATOM RegisterWndClass(HINSTANCE hInstance) {
  WNDCLASSEXW wcex;
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style          = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc    = WndProc;
  wcex.cbClsExtra     = 0;
  wcex.cbWndExtra     = 0;
  wcex.hInstance      = hInstance;
  wcex.hIcon          = icon;
  wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground  = colors[0];
  wcex.lpszMenuName   = nullptr; //MAKEINTRESOURCEW(IDC_GOL_WIN32);
  wcex.lpszClassName  = szWinClass;
  wcex.hIconSm        = icon;

  return RegisterClassExW(&wcex);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR szCmdLine, int iCmdShow)
{
	LPCWSTR title = L"Game of Life";

	colors[0] = static_cast<HBRUSH>(CreateSolidBrush(RGB_BLUE));
	colors[1] = static_cast<HBRUSH>(CreateSolidBrush(RGB_WHITE));

  constexpr bool standardicon = true;
	// Create icon
	BYTE iconAnd[32] = { 0 };
	BYTE iconXor[] = {
		0x00, 0x00, 0x00, 0x00,
		0x0F, 0xF0, 0x00, 0x00,
		0x0F, 0xF0, 0x00, 0x00,
		0x0F, 0xF0, 0x0F, 0xF0,
		0x0F, 0xF0, 0x0F, 0xF0,
		0x0F, 0xFF, 0xF0, 0x00,
		0x0F, 0xFF, 0xF0, 0x00,
		0x00, 0x00, 0x00, 0x00
	};
	icon = standardicon ? LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GOL_WIN32))
                      : CreateIcon(hInstance, 32, 8, 1, 1, iconAnd, iconXor);
	// Create window class
	RegisterWndClass(hInstance);

	// Create window
	HWND hwnd = CreateWindowExW(
		WS_EX_WINDOWEDGE, szWinClass, title,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
		nullptr, nullptr, hInstance, nullptr
	);

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
  return static_cast<int>(msg.wParam);
}

