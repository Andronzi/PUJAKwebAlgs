#include "../Genetic.h"
#include <iostream>

Genetic::Genetic(Graph* graph) : graph(graph), bestWay(nullptr) {}

void Genetic::crossChromo(u16 firstChromo, u16 secondChromo, u16* splitPath) {
	u16 start = rand() % (graph->length);
	u16 end = rand() % (graph->length - start) + start + 1;

	while (start == end) {
		end = rand() % (graph->length - start) + start + 1;
	}

	if (end == graph->length) {
		--end;
	}

	for (int i = start; i < end; ++i) {
		splitPath[i - start] = graph->ways[firstChromo].path[i]; 
	}

	u16 index = end - start;
	bool flag = true;
	for (int i = 0; i < graph->length; ++i) {
		for (int j = 0; j < end - start; ++j) {
			if (splitPath[j] == graph->ways[secondChromo].path[i]) {
				flag = false;
				break;
			}
		}

		if (flag) {
			splitPath[index] = graph->ways[secondChromo].path[i];
			++index;
			++end;
		}

		flag = true;
	}
}

void Genetic::mutation(u16* splitPath) {
	u16 firstIndex = rand() % graph->length;
	u16 secondIndex = rand() % graph->length;

	while (secondIndex == firstIndex) {
		secondIndex = rand() % graph->length;
	}

	std::swap(splitPath[firstIndex], splitPath[secondIndex]);
}

void Genetic::reverseMutation(u16* splitPath)
{
	for (size_t i = 0; i < graph->length / 2; ++i) {
		std::swap(splitPath[i], splitPath[graph->length - i - 1]);
	}
}

void Genetic::generate() {
	std::vector<Graph::Way> nextGeneration;
	
	for (size_t i = 0; i < graph->length; ++i) {
		u16* splitPath = new u16[graph->length];

		u16 firstChromo = rand() % graph->length;
		u16 secondChromo = rand() % graph->length;

		while (secondChromo == firstChromo) {
			secondChromo = rand() % graph->length;
		}

		crossChromo(firstChromo, secondChromo, splitPath);
		
		if (rand() % 100 <= 5) {
			mutation(splitPath);
		}

		Graph::Way firstSplitWay{ splitPath };

		graph->countWeight(firstSplitWay);
		nextGeneration.push_back(firstSplitWay);
	}

	this->graph->ways = nextGeneration;
};

std::pair<std::vector<u16>, double>* Genetic::operator()()
{
	generate();
	Graph::Way& minWay = graph->getMinWay();
	std::pair<std::vector<u16>, double>* epochResult =
		new std::pair<std::vector<u16>, double>{ std::vector<u16>(graph->length + 1), minWay.weight };

	u16* p_path = epochResult->first.data();
	for (size_t i = 0; i < graph->length; ++i) {
		p_path[i] = minWay.path[i];
	}
	p_path[graph->length] = p_path[0];

	return epochResult;
}

void Genetic::printData() {
	for (int i = 0; i < graph->length; ++i) {
		for (int j = 0; j < graph->length; ++j) {
			std::cout << graph->ways[i].path[j] << ' ';
		}

		std::cout << std::endl;
	}
}