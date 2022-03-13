#pragma once
/*
* �������� ��������� ������ ������� �� 2 ������: ��������� ���������������� ������� � ��������� ��������� ���������.
* 
* ��������� ������� ���������� ��������� �������� �� ������� ����� ������ ������. ���� ������ ���������� ������ ������ ������� � ����� �������. �����������, �� ��������� �����������
* ���������� (x ��� y) � ��� �� ������� ������� ������� � ���� "������" � "�������" �������� ��������� ����������.
* 
* --- ��� ������ ����� �������, ��� ������ ������� ������ ����� (������-������ ������ (0,1)). ���������� ������� - (x0, y0). ������������ ��������� ������ - �. ---
* 
* ��������� ��������� �������������� ���������: -A < (x-x0) < A => -A < (y-y0) < A => ((x-x0)^2+(y-y0)^2)^(1/2) < A. 
* �.�. ������� ���� ���������� ����� ��������� - ��������,��� ������ ������ � ������� �� ��������� 2�, � ����� �����������, ��� ������ ������ � ����. 
* ����� ����, ��� ������ ����� � ����, �� ����� ����� ��������� � ����������� (x->x0) � (y-y0) �� ����� 2D ����� � ������� ����, ���� �����. 
* �����������, ��� ����� ������ � ����� ������ ����� 90 �������� �� ����� �������� �� ���������� y �������: y = y0+A. ���� ���� ������ 90 ��������: y = y0+A, x = x0+-A
* 
* 
* �� ���� ����� ���� ������������� ������, ��� ����� ��������� ������ � ���������. 
* 
* ���� �� �������� ��������������� ������ std::vector<position, visionVector>, ��:
* 1) ����� ������ "� ���" � �������� ������� � ������ �� ������� ��������� O(2(n^2)) - n^2 �������� �� ������ ��������� + n^2 �������� �� ��������� ������.
* 2) �������� �� 2D ����� ����� ������� O(n^2) - n ������� ��������� ������� + n^2 ������� �����, ��������������� ���� ��������� ��������.
* 
* ���� �� ���������� std::set ��� std::map (�� multiset/multimap, �.�. ��� ����� � ����� ���� �� ����� ������������� �� �������� ������), ��:
* �� ������, � �������, n*log(n) �� ����������� ������-������� ������ + A*log(n) �� ����� ��������� ����� ������ ������ + C �� �������� �� ������ ���� � ������� �
* �������� ��������� ������� �������.
* ����� ��������� ������� O(n*log(n)). 
* 
* ����� ���� ������� �������
* ���������� ������ ����� ����������� ��������������, ������� ����� �������� ��������� ������ �� �������� ��������� ��������� ��� ���������� ������, ��������, <100. 
* �������, ��� �������� � ����������� �������� ������� ����� ��������: 
*	-- ������������� ����������� ��������� ����������� ���������� ������; 
*	-- ���������� ����������������� ������: ���� ������ ������ ������, �� ������ �� ���� ������, �������� ���������� ������� ������ � ������ ������ - ������ ����� 
* ���������� � 1000 ������. ���� �� ������ ������ ������, �� �������� ������ � ������� ���������� ������ ��������� �� ����� � ��� �� ������ - ������, � ����������� �� ����������
* ���������, ����� ���������� � 10-50 ������.
* 
* 
*/

#include <map>
#include <cmath>
#include <vector>
#include <algorithm>
#include <array>

namespace visEval {
	using id = int;

	typedef struct {
	int p_y;
	int p_x;
} position_t;

	typedef struct {
	float v_sin;
	float v_cos;
} visionVector_t;

	struct unitInfo {
public:
	position_t position;
	visionVector_t visionVector;

	explicit unitInfo() = default;
	explicit unitInfo(position_t pos, visionVector_t visionVec) :
		position(pos), visionVector(visionVec)
	{}
};



	constexpr float visionSector = 90;
	constexpr int visionDistance = 2;

	const float Pi = 3.14159265f;
	const float visionSector_radians = visionSector / (180 / Pi);
	const float sin_VisionSectorAngle = sin(visionSector_radians / 2);
	const float cos_VisionSectorAngle = cos(visionSector_radians / 2);


	template <int Distance>
	constexpr auto DistanceMapping()
{
	std::array<std::array<bool, 2 * visionDistance + 1>, 2 * visionDistance + 1> Matrix{};

	for (int i = 0; i < 2 * visionDistance + 1; i++) {
		for (int j = 0; j < 2 * visionDistance + 1; j++) {
			/*
			* ���� �������� ������� sqrt � pow �� constexpr ��������, �� ��� ������� VisionMapping ������ constexpr
			*/
			if (sqrt(pow(i - Distance, 2) + pow(j - Distance, 2)) <= visionDistance) {
				Matrix[j][i] = true;
			}
			else
				Matrix[j][i] = false;
		}
	}
	return Matrix;
}


/*
* ������������������ ��������:
* 1. �������� ������� ������ �������.
* 2. ������� ���� ������� � �������, ������ �� ������ ������� � ���� ������ �������.
* 3. ���� ������ - ��������� �������� ������ ������ � �������, ���������� �� �� ����� � ������ �������, �������������� ������ ������.
* 
* ������ std::map ������������ ������������� std::vector - ��� ���������� ��������� � ����, ������� ����������� ������ � ���������� ���-���������
* 
*/
	class UnitsVision
	{
	private:
		struct extendedUnitInfo {
		id unit_id;
		unitInfo Info;
		int howManySeeUnits = 0;

		struct PositionCompare {
			bool operator()(const extendedUnitInfo& lhs, const extendedUnitInfo& rhs) const {
				if (lhs.Info.position.p_y == rhs.Info.position.p_y)
					return lhs.Info.position.p_x < rhs.Info.position.p_x;
				else
					return lhs.Info.position.p_y < rhs.Info.position.p_y;
			}
		};
	} ;

		std::vector<extendedUnitInfo> alternative2DMap;


		int gradient_search(position_t not_less_than, position_t not_greater_than);

		bool check_vision_sector(visionVector_t sector_vector, const unitInfo& unit_info, position_t object_position);

		UnitsVision(UnitsVision&) = delete;
		UnitsVision(UnitsVision&&) = delete;
	public:

		UnitsVision() = default;

		void evaluateVision(std::map<id, unitInfo> units);

		std::vector<std::pair<id, int>> getVisibleUnitsAmount();
	};

}




