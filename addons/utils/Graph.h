#pragma once
#include <malloc.h>
#include <cstdint>
#include <stdlib.h>
#include <vector>

typedef uint16_t u16;

class Graph
{
public:
	struct Way {
		u16* path;
		double weight;
	};

	u16 length;
	double** matrix;

	std::vector<Way> ways;

	Graph(u16 length, double** matrix);
	~Graph();

	Way& getMinWay();

	/*u16 getMinWeight(double** matrix);
	u16 getMaxWeightIndex(double** matrix);
	int32_t getMaxWeightIndex(double** matrix, u16* way);*/
	void countWeight(Way& way);

	//set random way on Graph
	void setRandomWay(Way& way);
};