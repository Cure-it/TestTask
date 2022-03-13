#pragma once
#include "UnitsVision.h"

/*
* 0) ƒл€ реализации вычислений выбран подсчЄт на CPU, а не GPU. ¬о-первых, GPU используетс€ дл€ рендера графики и ему будет чем зан€тьс€ и без этих вычислений.
* ¬о-вторых, https://habr.com/ru/company/dbtc/blog/498374/ - автор показывает, что дл€ небольших и несложных вычислений вопросы загрузки-выгрузки данных в пам€ть GPU 
* могут существенно увеличить врем€ выполнени€. — ограничем в "не более 10000 юнитов" эта задача видитс€ небольшой.
* 
* 
* 1) ѕозици€ юнита определ€етс€ парой <int, int>. ѕочему целочисленные? ѕотому что вещественные типы так или иначе надо будет приводить к сетке. –азмерность сетки может составл€ть 
* 0.1, 0.01 и т.д., в зависимости от целей обсчЄта. “ип float и размерность сетки можно использовать тогда, когда 2D поле представл€ет собой значительную область и нам нужна гибкость
* выражени€ чисел через float дл€ покрыти€ всей области.
* ¬ данном случае € считаю, что 2^32 точек хватит дл€ полного покрыти€ нужной нам области. 
* 
* 2) ¬ектор зрени€ описываетс€ парой <float, float>. ѕочему не double хорошо описывает стать€: https://habr.com/ru/post/331814/
* ќсновные тезисы: 1 - меньше занимает места в оперативке и в кеше -> больше кэш-попаданий и меньше обращений к оперативке через PCIe. 2 - иногда процессорам проще обрабатывать
* вычислени€ с float (4 байта), чем с double (8 байт).
*/


using namespace visEval;

class UnitsVisionTest
{
private:

	using id = int;

	static int get_next_num ()
	{
		static int i = 0;
		return ++i;
	}

	std::map<id, unitInfo> m_unitsHandmade = {
		{get_next_num(), unitInfo({1,1}, {0.f, 1.f})},
		{get_next_num(), unitInfo({1,2}, {1.f, 0.f})},
		{get_next_num(), unitInfo({-1,-1}, {0.707f, 0.707f})},
		{get_next_num(), unitInfo({1,3}, {1.f, 0.f})},
		{get_next_num(), unitInfo({2,3}, {1.f, 0.f})},
		{get_next_num(), unitInfo({-2,-1}, {1.f, 0.f})},
		{get_next_num(), unitInfo({-2,-2}, {1.f, 0.f})},
		{get_next_num(), unitInfo({-2,0}, {1.f, 0.f})},
		{get_next_num(), unitInfo({-1,-2}, {1.f, 0.f})},
		{get_next_num(), unitInfo({-1,0}, {1.f, 0.f})},
		{get_next_num(), unitInfo({-1,1}, {1.f, 0.f})},
		{get_next_num(), unitInfo({0,2}, {1.f, 0.f})},
		{get_next_num(), unitInfo({0,4}, {1.f, 0.f})},
		{get_next_num(), unitInfo({2,4}, {1.f, 0.f})},
		{get_next_num(), unitInfo({2,1}, {1.f, 0.f})},
	};

public:
	UnitsVisionTest()
	{}
	
	const std::map<id, unitInfo>& operator() (void)
	{
		return m_unitsHandmade;
	}


};

