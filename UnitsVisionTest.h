#pragma once
#include "UnitsVision.h"
#include <unordered_set>
#include <random>
/*
* 0) ��� ���������� ���������� ������ ������� �� CPU, � �� GPU. ��-������, GPU ������������ ��� ������� ������� � ��� ����� ��� �������� � ��� ���� ����������.
* ��-������, https://habr.com/ru/company/dbtc/blog/498374/ - ����� ����������, ��� ��� ��������� � ��������� ���������� ������� ��������-�������� ������ � ������ GPU 
* ����� ����������� ��������� ����� ����������. � ��������� � "�� ����� 10000 ������" ��� ������ ������� ���������.
* 
* 
* 1) ������� ����� ������������ ����� <int, int>. ������ �������������? ������ ��� ������������ ���� ��� ��� ����� ���� ����� ��������� � �����. ����������� ����� ����� ���������� 
* 0.1, 0.01 � �.�., � ����������� �� ����� �������. ��� float � ����������� ����� ����� ������������ �����, ����� 2D ���� ������������ ����� ������������ ������� � ��� ����� ��������
* ��������� ����� ����� float ��� �������� ���� �������.
* � ������ ������ � ������, ��� 2^32 ����� ������ ��� ������� �������� ������ ��� �������. 
* 
* 2) ������ ������ ����������� ����� <float, float>. ������ �� double ������ ��������� ������: https://habr.com/ru/post/331814/
* �������� ������: 1 - ������ �������� ����� � ���������� � � ���� -> ������ ���-��������� � ������ ��������� � ���������� ����� PCIe. 2 - ������ ����������� ����� ������������
* ���������� � float (4 �����), ��� � double (8 ����).
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


	std::map<id, unitInfo> getRandomGeneration(int amount)
	{
		struct KeyHash {
			std::size_t operator()(const position_t& key) const
			{
				return key.p_x ^ (key.p_y << 1);
			}
		};

		struct KeyEqual {
			bool operator()(const position_t& lhs, const position_t& rhs) const
			{
				return (lhs.p_x == rhs.p_x && lhs.p_y == rhs.p_y);
			}
		};
		
		std::unordered_set<position_t, KeyHash, KeyEqual> positions;

		std::random_device randomizer;
		std::mt19937 gen(randomizer()); 
		std::uniform_int_distribution<> distrib(-5000, 5000);
		
		while (positions.size() < amount)
			positions.insert({ distrib(gen), distrib(gen) });

		std::uniform_real_distribution<> real_distrub(0.f, 1.f);
		std::map<id, unitInfo> test;
		float sinus = 0;
		for (auto& it : positions) {
			sinus = static_cast<float>(real_distrub(gen));
			test.insert({ get_next_num(), unitInfo(it, { sinus, static_cast<float>(sqrt(1 - pow(sinus,2))) } ) } );
		}

		return test;
	}


};

