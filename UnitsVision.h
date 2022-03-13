#pragma once
/*
* Проверка видимости всегда состоит из 2 этапов: выделение просматриваемого сектора и выделение дальности видимости.
* 
* Выделение сектора проводится бросанием векторов из крайних точек конуса зрения. Цель должна находиться правее левого вектора и левее правого. Практически, мы фиксируем определённую
* координату (x или y) и для неё считаем границы сектора в виде "левого" и "правого" значения остальной координаты.
* 
* --- Для модели будем считать, что объект смотрит строго вверх (радиус-вектор зрения (0,1)). Координаты объекта - (x0, y0). Максимальная дальность зрения - А. ---
* 
* Выделение дальности осуществляется проверкой: -A < (x-x0) < A => -A < (y-y0) < A => ((x-x0)^2+(y-y0)^2)^(1/2) < A. 
* Т.е. сначала идут вычисления малой сложности - проверка,что объект входит в квадрат со сторонами 2А, а потом проверяется, что объект входит в круг. 
* После того, как объект вошёл в круг, мы можем смело двигаться в направлении (x->x0) и (y-y0) по нашей 2D сетке и считать всех, кого найдём. 
* Практически, для нашей модели и углов зрения менее 90 градусов мы можем отложить по координате y границу: y = y0+A. Если угол больше 90 градусов: y = y0+A, x = x0+-A
* 
* 
* На этом этапе надо принципиально решить, как будет проходить работа с объектами. 
* 
* Если мы получаем неупорядоченный вектор std::vector<position, visionVector>, то:
* 1) Решая задачу "в лоб" и проверяя каждого с каждым мы получим сложность O(2(n^2)) - n^2 проверок на сектор видимости + n^2 проверок на дальность зрения.
* 2) Двигаясь по 2D сетке можно ожидать O(n^2) - n поисков граничных условий + n^2 поисков узлов, удовлетворяющих этим граничным условиям.
* 
* Если мы используем std::set или std::map (не multiset/multimap, т.к. два юнита в одном узле не могут располагаться по условиям задачи), то:
* Мы тратим, в среднем, n*log(n) на составление красно-чёрного дерева + A*log(n) на поиск граничных узлов внутри дерева + C на движение от одного узла к другому и
* проверку граничных условий сектора.
* Итого сложность порядка O(n*log(n)). 
* 
* Здесь надо сделать ремарку
* Построение дерева имеет константный мультипликатор, который может запросто перекрыть выгоду от снижения сложности алгоритма для количества юнитов, например, <100. 
* Поэтому, для хорошего и взвешенного принятия решения нужно понимать: 
*	-- распределение вероятности встретить определённое количество юнитов; 
*	-- количество переиспользований дерева: если классу выдали вектор, он сделал из него дерево, посчитао количество видимых юнитов и удалил дерево - выгода может 
* начинаться с 1000 юнитов. Если же классу выдали вектор, он построил дерево и считает количество юнитов постоянно на одном и том же дереве - выгода, в зависимости от количества
* подсчётов, может начинаться с 10-50 юнитов.
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
			* Если заменить функции sqrt и pow их constexpr версиями, то вся функция VisionMapping станет constexpr
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
* Последовательность действий:
* 1. Выделяем квадрат вокруг объекта.
* 2. Обходим этот квадрат и смотрим, входят ли другие объекты в круг вокруг объекта.
* 3. Если входят - выпускаем обратный зрению вектор и смотрим, пересекает ли он левый и правый вектора, ограничивающие сектор обзора.
* 
* Вместо std::map используется сортированный std::vector - для уменьшения аллокаций в куче, большей собранности данных и увеличения кэш-попаданий
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




