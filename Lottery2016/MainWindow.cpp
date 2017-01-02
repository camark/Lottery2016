#include "pch.h"
#include "MainWindow.h"
#include "Items.h"
#include "Resources\resource.h"
#include "Person.h"
#include "FlashImageScene.h"
#include "DxRes.h"

using D2D1::ColorF;
using namespace std;

const UINT ControlCombox = 1;
const UINT MenuLotteryStart = 2;
const UINT MenuLotteryLast() { return MenuLotteryStart + GetItems().size(); }
const UINT MenuStatusStart() { return MenuLotteryLast() + 1; };
const UINT MenuStatusLast() { return MenuStatusStart() + GetItems().size(); };
const UINT MenuStatusClear() { return MenuStatusLast() + 1; };

BEGIN_MESSAGE_MAP(MainWindow, CFrameWnd)
	ON_COMMAND_RANGE(MenuLotteryStart, MenuLotteryLast(), OnLottery)
	ON_COMMAND_RANGE(MenuStatusStart(), MenuStatusLast(), OnStatus)
	ON_COMMAND(MenuStatusClear(), OnLotteryClear)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_REGISTERED_MESSAGE(AFX_WM_DRAW2D, OnDraw2D)
	ON_REGISTERED_MESSAGE(AFX_WM_RECREATED2DRESOURCES, CreateDeviceResources)
	ON_WM_KEYUP()
END_MESSAGE_MAP()

MainWindow::MainWindow()
{
	Create(nullptr, L"Lottery 2016", WS_OVERLAPPEDWINDOW, { 100, 100, 800, 600 });
}

void MainWindow::Update()
{
	if (_scene)
	{
		_scene->Step();
	}
}

void MainWindow::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_SPACE)
	{
		auto completed = _scene->Toggle();
		if (completed)
		{
			SaveLuckyPersonIds(GetLotteryId(), _scene->GetSelectedPersonIds());
		}
	}
}

void MainWindow::OnSize(UINT nType, int cx, int cy)
{
	if (nType != SIZE_MINIMIZED && _dxRes)
	{
		_dxRes->CreateDeviceSizeResources(GetRenderTarget());
	}
}

void MainWindow::OnLottery(UINT id)
{
	auto itemId = id - MenuLotteryStart;
	auto item = GetItems()[itemId];
	auto unluckyIds = GetUnluckyPersonIds();
	auto needCount = item.Count;

	if (unluckyIds.empty())
	{
		MessageBoxW(L"��������");
		return;
	}

	if (needCount > (int)unluckyIds.size())
	{
		CString str;
		str.Format(L"ʣ������(%d)�����(%s)����Ҫ��(%d)��,�Ƿ����Ϊ��(%d)�˲�����?",
			unluckyIds.size(), item.Name, needCount, unluckyIds.size());
		if (MessageBoxW(str, nullptr, MB_YESNO) == IDNO)
		{
			return;
		}
		needCount = (int)unluckyIds.size();
	}

	auto luckyCount = GetLuckyPersonIds(itemId).size();
	if (luckyCount > 0)
	{
		CString str;
		str.Format(L"���� %d �˳��� %s, �Ƿ����?", luckyCount, item.Name);
		if (MessageBoxW(str, nullptr, MB_YESNO) == IDNO)
		{
			return;
		}
	}

	_menu.CheckMenuRadioItem(MenuLotteryStart, MenuLotteryLast(), id, MF_BYCOMMAND);

	_scene = make_unique<FlashImageScene>(needCount, unluckyIds);
}

void MainWindow::OnStatus(UINT id)
{
	auto itemId = id - MenuStatusStart();
	auto ids = GetLuckyPersonIds(itemId);
	if (ids.size() == 0)
	{
		CString str;
		str.Format(L"%s Ŀǰ���˻񽱡�", GetItems()[itemId].Name);
		MessageBoxW(str);
	}
	else
	{
		auto path = CreateLuckyStatusFile(itemId, ids);
		ShellExecuteW(GetSafeHwnd(), nullptr, path, nullptr, nullptr, SW_SHOW);
	}
}

int MainWindow::OnCreate(LPCREATESTRUCT cs)
{
	// setup menu
	_menu.LoadMenuW(IDR_MENU1);
	for (size_t i = 0; i < GetItems().size(); ++i)
	{
		auto item = GetItems()[i];
		auto lotteryMenu = _menu.GetSubMenu(1);
		auto statusMenu = _menu.GetSubMenu(2);

		CString str;
		str.Format(L"%s(&%d)", item.Name, i + 1);
		lotteryMenu->AppendMenuW(MF_STRING, MenuLotteryStart + i, str);
		statusMenu->AppendMenuW(MF_STRING, MenuStatusStart() + i, str);
	}
	_menu.GetSubMenu(1)->RemoveMenu(0, 0);
	_menu.GetSubMenu(2)->RemoveMenu(0, 0);
	_menu.GetSubMenu(2)->AppendMenuW(MF_SEPARATOR);
	_menu.GetSubMenu(2)->AppendMenuW(MF_STRING, MenuStatusClear(), L"���״̬(&C)");
	_menu.CheckMenuRadioItem(MenuLotteryStart, MenuLotteryLast(), MenuLotteryStart, MF_BYCOMMAND);
	_scene = make_unique<FlashImageScene>(GetItems()[0].Count, GetUnluckyPersonIds());
	SetMenu(&_menu);

	// d2d
	EnableD2DSupport();
	_dxRes = make_unique<DxRes>();
	CreateDeviceResources(0, (LPARAM)GetRenderTarget());
	_dxRes->CreateDeviceSizeResources(GetRenderTarget());

	return TRUE;
}

void MainWindow::OnLotteryClear()
{
	if (MessageBoxW(L"ȷ��Ҫ��ճ齱״̬��?", nullptr, MB_YESNO) == IDYES)
	{
		DeleteLuckyPersons();
		MessageBoxW(L"���г齱״̬�������");
	}
}

LRESULT MainWindow::OnDraw2D(WPARAM, LPARAM lparam)
{
	Update();

	auto target = (CHwndRenderTarget*)lparam;
	ASSERT_VALID(target);

	RECT rect;
	GetClientRect(&rect);
	CD2DRectF d2dRect{ (float)rect.left, (float)rect.top, (float)rect.right, (float)rect.bottom };

	target->DrawBitmap(_dxRes->LotteryBitmaps[GetLotteryId()], d2dRect);

	SYSTEMTIME st;
	GetLocalTime(&st);
	CString str;
	str.Format(L"%d", st.wMilliseconds);
	target->DrawTextW(str, d2dRect, _dxRes->Blue);

	_scene->Render(target, _dxRes.get());

	return TRUE;
}

LRESULT MainWindow::CreateDeviceResources(WPARAM, LPARAM lparam)
{
	_dxRes->CreateDeviceResources((CHwndRenderTarget*)lparam);
	return 0;
}

size_t MainWindow::GetLotteryId()
{
	for (size_t i = 0; i < GetItems().size(); ++i)
	{
		if (_menu.GetMenuState(UINT(i + MenuLotteryStart), MF_BYCOMMAND) & MF_CHECKED)
		{
			return i;
		}
	}
	return 0;
}
