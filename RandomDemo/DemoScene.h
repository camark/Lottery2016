#pragma once

#include "Scene.h"

class RandomDemoScene : public Scene
{
public:
	// ͨ�� Scene �̳�
	virtual void CreateDeviceResources(CHwndRenderTarget *) override;
	virtual void CreateDeviceSizeResources(CHwndRenderTarget *) override;
	virtual void Render(CHwndRenderTarget *) override;
	virtual void Update() override;
	static std::unique_ptr<RandomDemoScene> Make(int type);
	
private:
	virtual UINT GetNextRand() = 0;
	std::vector<UINT> _points;
	
	// dxres
	CD2DSolidColorBrush * _brush;
};

class RandDemoScene : public RandomDemoScene
{
	// ͨ�� DemoScene �̳�
	virtual UINT GetNextRand() override;
};

class RdDemoScene : public RandomDemoScene
{
	// ͨ�� DemoScene �̳�
	virtual UINT GetNextRand() override;
	std::random_device _rd;
};

class Mt19937DemoScene : public RandomDemoScene
{
	// ͨ�� DemoScene �̳�
	virtual UINT GetNextRand() override;
	std::mt19937 _mt{ std::random_device()() };
};

class RanluxDemoScene : public RandomDemoScene
{
	// ͨ�� DemoScene �̳�
	virtual UINT GetNextRand() override;
	std::ranlux48 _rd{ std::random_device()() };
};

class MinstdRandDemoScene : public RandomDemoScene
{
	// ͨ�� DemoScene �̳�
	virtual UINT GetNextRand() override;
	std::minstd_rand _rd{ std::random_device()() };
};

class CSDemoScene : public RandomDemoScene
{
	// ͨ�� DemoScene �̳�
	virtual UINT GetNextRand() override;
};