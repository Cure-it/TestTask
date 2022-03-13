#include "UnitsVIsion.h"


using namespace visEval;

auto visionDistanceMatrix = DistanceMapping<2>();


int UnitsVision::gradient_search(position_t not_less_than, position_t not_greater_than) {
	auto vec_it1 = alternative2DMap.begin();
	int size = static_cast<int>(alternative2DMap.size());

	int step = size / 2;
	int index = 0;

	if (vec_it1[index].Info.position.p_x >= not_less_than.p_x && vec_it1[index].Info.position.p_x <= not_greater_than.p_x &&
		vec_it1[index].Info.position.p_y >= not_less_than.p_y && vec_it1[index].Info.position.p_y <= not_greater_than.p_y)
		return 0;

	/* Подходим к наибольшему индексу, при котором "y" текущего юнита ниже искомого */
	while (step) {
		if (index + step < size) {
			if (vec_it1[index + step].Info.position.p_y < not_less_than.p_y) {
				index += step;
			}
			else
				step /= 2;
		}
		else
			step /= 2;
	}

	/* Последовательно ищем первый подходящий объект */
	while (true) {
		index++;
		if (index < size) {
			if (vec_it1[index].Info.position.p_y > not_greater_than.p_y)
				return size;

			if ((vec_it1[index].Info.position.p_x >= not_less_than.p_x && vec_it1[index].Info.position.p_x <= not_greater_than.p_x) ||
				(vec_it1[index].Info.position.p_y > not_less_than.p_y && vec_it1[index].Info.position.p_x <= not_greater_than.p_x &&
					vec_it1[index].Info.position.p_x >= not_greater_than.p_x - (2 * visionDistance)))
				return index;
		}
		else
			return index;
	}
};


bool UnitsVision::check_vision_sector(visionVector_t sector_vector, const unitInfo& unit_info, position_t object_position)
{
	visionVector_t to_another_unit {
		static_cast<float>(object_position.p_y - unit_info.position.p_y),
		static_cast<float>(object_position.p_x - unit_info.position.p_x),
	};

	visionVector_t unit_vector_operating = unit_info.visionVector;
	float multiplicator;
	float unitMultiplicator;
	float sectorMultiplicator;

	if (unit_info.visionVector.v_cos == 0) {
		/* Зануляем синус вектора */
		multiplicator = sector_vector.v_sin / sector_vector.v_cos;
		sector_vector.v_sin = 0;
		unit_vector_operating.v_sin -= unit_vector_operating.v_cos * multiplicator;
		to_another_unit.v_sin -= to_another_unit.v_cos * multiplicator;

		/* Получаем множитель для вектора зрения */
		unitMultiplicator = to_another_unit.v_sin / unit_vector_operating.v_sin;

		/* Получаем множитель для вектора угла обзора  */
		sectorMultiplicator = (to_another_unit.v_cos - unit_vector_operating.v_cos * unitMultiplicator) / sector_vector.v_cos;
	}
	else {
		/* Зануляем синус вектора */
		multiplicator = unit_vector_operating.v_sin / unit_vector_operating.v_cos;
		unit_vector_operating.v_sin = 0;
		sector_vector.v_sin -= sector_vector.v_cos * multiplicator;
		to_another_unit.v_sin -= to_another_unit.v_cos * multiplicator;

		/* Получаем множитель для вектора зрения */
		sectorMultiplicator = to_another_unit.v_sin / sector_vector.v_sin;

		/* Получаем множитель для вектора угла обзора  */
		unitMultiplicator = (to_another_unit.v_cos - sector_vector.v_cos * sectorMultiplicator) / unit_vector_operating.v_cos;

		
	}

	return (!(sectorMultiplicator < -0.0001f) && !(unitMultiplicator < -0.0001f));
}


void UnitsVision::evaluateVision(std::map<id, unitInfo> units)
{
	if (visionSector <= 0 || visionDistance <= 0)
		return;
	/* Сначала заполняем вектор, потом упорядочиваем его. Или сразу заполнять упорядоченным? */

	alternative2DMap.resize(units.size());
	alternative2DMap.clear();
	for (const auto& it : units) {
		alternative2DMap.push_back({ it.first, it.second });
	}

	std::sort(alternative2DMap.begin(), alternative2DMap.end(), extendedUnitInfo::PositionCompare());

	for (auto& unit : alternative2DMap)
	{
		if (unit.Info.visionVector.v_cos == 0 && unit.Info.visionVector.v_sin == 0)
			throw - 1; // не указан радиус-вектор
		auto left = unit.Info.position.p_x - visionDistance;
		auto right = unit.Info.position.p_x + visionDistance;

		auto top = unit.Info.position.p_y + visionDistance;
		auto bottom = unit.Info.position.p_y - visionDistance;


		visionVector_t left_vision_sector_vector;
		visionVector_t right_vision_sector_vector;

		if (visionSector < 360) {
			/*
			* sin(a + b) = sin(a) * cos(b) + cos(a) * sin(b)
			* sin(a - b) = sin(a) * cos(b) - cos(a) * sin(b)
			* cos(a + b) = cos(a) * cos(b) - sin(a) * sin(b)
			* cos(a - b) = cos(a) * cos(b) + sin(a) * sin(b)
			*
			* left = sight + sector/2;
			* right = sight - sector/2
			*/
			left_vision_sector_vector.v_sin = unit.Info.visionVector.v_sin * cos_VisionSectorAngle + unit.Info.visionVector.v_cos * sin_VisionSectorAngle;
			left_vision_sector_vector.v_cos = unit.Info.visionVector.v_cos * cos_VisionSectorAngle - unit.Info.visionVector.v_sin * sin_VisionSectorAngle;

			right_vision_sector_vector.v_sin = unit.Info.visionVector.v_sin * cos_VisionSectorAngle - unit.Info.visionVector.v_cos * sin_VisionSectorAngle;
			right_vision_sector_vector.v_cos = unit.Info.visionVector.v_cos * cos_VisionSectorAngle + unit.Info.visionVector.v_sin * sin_VisionSectorAngle;
		}

		/* Проверить квадрат */
		/* Для упорядоченного вектора используем бинарный поиск */
		int y = bottom;
		int x = left;
		auto unit_check = alternative2DMap.at(gradient_search({ y, x } , {top, right}));

		while (unit_check.Info.position.p_y <= top || unit_check.Info.position.p_x <= right) {
			if (unit_check.unit_id != unit.unit_id) {
				auto test_vert = unit_check.Info.position.p_y - bottom;
				auto test_hor = unit_check.Info.position.p_x - left;
				if (visionDistanceMatrix[unit_check.Info.position.p_y - bottom][unit_check.Info.position.p_x - left]) {
					if (visionSector < 360)
					{
						/*
						* Здесь мы точно знаем, что другой юнит находится в дистанции зрения текущего юнита.
						* Наша задача - понять, входит ли он в сектор зрения. Чуть ранее мы отложили 3 вектора - направление взгляда, левый край сектора зрения и правый край.
						* Если объект входит, то вектор от другого юнита к текущему юниту может быть представлен как линейная комбинация вектора взгляда и правого или левого вектора
						* сектора.
						*
						* Для этого решаем систему линейных уравнений для радиус-векторов.
						*/
						int already_see = 0;


						if (check_vision_sector(left_vision_sector_vector, unit.Info, unit_check.Info.position)) { // если оба мультипликатора положительные - сектор обзора включает точку другого юнита
							unit.howManySeeUnits++;
							already_see++;
						}

						/* Если цель лежит ровно в направлении взгляда - надо избежать двойного учёта */
						if (!already_see) {
							if (check_vision_sector(right_vision_sector_vector, unit.Info, unit_check.Info.position)) { // если оба мультипликатора положительные - сектор обзора включает точку другого юнита
								unit.howManySeeUnits++;
								already_see++;
							}
						}


					}
					else
						unit.howManySeeUnits++;
				}
			}
				x = unit_check.Info.position.p_x + 1;
				if (x > right) {
					y = unit_check.Info.position.p_y + 1;
					if (!(y > top))
						x = left;
				}
				else
					y = unit_check.Info.position.p_y;

				int index = gradient_search({ y, x }, {top, right});
				if (index >= alternative2DMap.size())
					break;

				unit_check = alternative2DMap.at(index);
		}
	}
}


using id = int;
std::vector<std::pair<id, int>> UnitsVision::getVisibleUnitsAmount()
{
	std::vector<std::pair<id, int>> visAmount;
	visAmount.resize(alternative2DMap.size());
	visAmount.clear();

	for (const auto& it : alternative2DMap)
		visAmount.push_back({ it.unit_id, it.howManySeeUnits });

	return visAmount;
}











