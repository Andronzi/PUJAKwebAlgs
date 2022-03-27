#include <algorithm>
#include <fstream>
#include "../LearningClient.h"
#include "../tools.h"

using namespace vectorExtention;

LearningClient::LearningClient(Dataset& dataset, NeuralNetwork& net, size_t workersCount)
	: dataset(dataset), workers(workersCount), regions(workersCount),
	tasksDone(0), layCount(net.dims.size()), 
	resultSum(layCount - 1, layCount), run(false), terminateFlag(false)
{
	for (size_t i = 1; i < layCount; ++i) {
		resultSum.weightsGradient[i - 1] = Matrix<double>(net.dims[i], net.dims[i - 1]);
		resultSum.biasesGradient[i] = std::vector<double>(net.dims[i]);
	}
	
	size_t regionSize = dataset.size() / workersCount,
		remnants = dataset.size() % workersCount;

	regions[0].first = dataset.begin();
	regions[0].second = dataset.begin() + regionSize;
	for (size_t i = 1; i < workersCount; ++i)
	{
		regions[i].first = regions[i - 1].second;
		regions[i].second = regions[i].first + regionSize;
		if (remnants) {
			++regions[i].second;
			--remnants;
		}
	}

	is_workerFinished.assign(workersCount, true);
	for (size_t i = 0; i < workersCount; ++i) {
		workers[i] = new std::thread(workerRuntime, this, std::ref(regions[i]), std::ref(net));
	}
}

LearningClient::~LearningClient() {
	run = false;
	terminateFlag = true;
	awakeWorkers.notify_all();
	for (std::thread* pthr : workers) {
		if (pthr) if (pthr->joinable()) pthr->join();
		delete pthr;
	}
}

void LearningClient::workerFinishReport(LearningClient* client, const  std::thread::id& workerId) 
{
	std::unique_lock<std::mutex> lock(client->callbackMutex);
	for (size_t i = 0; i < client->workers.size(); ++i) {
		if (client->workers[i]->get_id() == workerId) {
			client->is_workerFinished[i] = true;
			break;
		}
	}
	for (size_t i = 0; i < client->workers.size(); ++i) {
		if (!client->is_workerFinished[i]) return;
	}
	client->workDone.notify_one();
}

void LearningClient::workerRuntime(LearningClient* client, Region& region, 
	NeuralNetwork& net)
{
	NeuralNetwork _net(net);
	std::mutex _mutex;
	std::unique_lock<std::mutex> selfLock(_mutex);

	while (!client->terminateFlag)
	{
		while (region.first == region.second || !client->run) {
			LearningClient::workerFinishReport(client, std::this_thread::get_id());
			client->awakeWorkers.wait(selfLock);
			if (client->terminateFlag) return;
		}

		while (client->run && region.first != region.second)
		{
			NeuralNetwork::backPropagation_Result* result = _net.backPropagation(*region.first);
			{
				std::unique_lock<std::mutex> lock(client->outputMutex);
				for (size_t i = 0; i < _net.dims.size() - 1; ++i) {
					client->resultSum.weightsGradient[i] += result->weightsGradient[i];
					client->resultSum.biasesGradient[i] += result->biasesGradient[i];
				}
				++client->tasksDone;
			}
			delete result;
			++region.first;
		}
	}
}

bool LearningClient::isDone() { 
	return std::find(is_workerFinished.begin(), 
		is_workerFinished.end(), false) == is_workerFinished.end();
}

void LearningClient::launch() {
	if (!isDone()) throw "Calculation haven't finished yet; use abort() or await() first";

	tasksDone = 0;
	for (size_t i = 1; i < layCount; ++i) {
		resultSum.weightsGradient[i - 1].forEach(
			[](double& el) { el = 0; });
		std::for_each(resultSum.biasesGradient[i].begin(), resultSum.biasesGradient[i].end(),
			[](double& el) { el = 0; });
	}
	is_workerFinished.assign(is_workerFinished.size(), false);
	run = true;
	awakeWorkers.notify_all();
}

void LearningClient::doDescent(Matrix<double>* weights, std::vector<double>* biases, 
	double descentCoef) 
{
	if (!isDone()) throw "Calculation haven't finished yet; use abort() or await() first";

	if (!tasksDone) throw "Invalid gradient; do calculations fist";

	for (size_t l = 0; l < layCount - 1; ++l) {
		resultSum.weightsGradient[l].forEach(
			[count = tasksDone, coef = descentCoef](double& el) { el /= count * coef; });

		std::for_each(resultSum.biasesGradient[l + 1].begin(), resultSum.biasesGradient[l + 1].end(),
			[count = tasksDone, coef = descentCoef](double& el) { el /= count * coef; });

		weights[l] -= resultSum.weightsGradient[l];
		biases[l + 1] -= resultSum.biasesGradient[l + 1];

		/*resultSum.weightsGradient[l].console_log();
		std::cout << "\n=================================================\n";*/
	}

	tasksDone = 0;
}

void LearningClient::abort() { run = false; }

void LearningClient::await() {
	std::mutex _mutex;
	std::unique_lock<std::mutex> selfLock(_mutex);
	if (!isDone()) workDone.wait(selfLock);
}

void LearningClient::assignDataSet(const std::vector<NeuralNetwork::Package>& dataset) {
	run = false;
	std::mutex _mutex;
	std::unique_lock<std::mutex> selfLock(_mutex);
	workDone.wait(selfLock);
	this->dataset = dataset;
}






uint8_t buf4[4];

int readInt32(std::ifstream& stream) {
	for (int i = 3; i >= 0; --i)
		stream >> buf4[i];
	return *(int*)&buf4;
}

void normalize_MultiThread(std::vector<uint8_t>& buffer, size_t imgSize,
	Dataset& output, size_t workersCount)
{
	std::thread* workers = new std::thread[workersCount];

	auto runtime = [&output, imgSize, &buffer](size_t imgInd, size_t endInd) {
		uint8_t* byteRegion = buffer.data() + imgInd * imgSize;
		for (; imgInd < endInd; ++imgInd) {
			std::vector<double>& cur = output[imgInd].data;
			cur = std::vector<double>(imgSize);

			for (size_t i = 0; i < imgSize; ++i) {
				cur[i] = (double)*(byteRegion++) / 255;
			}
		}
	};

	size_t regionSize = output.size() / workersCount,
		remnants = output.size() % workersCount;

	size_t ind1, ind2 = 0;
	for (size_t i = 0; i < workersCount; ++i)
	{
		ind1 = ind2;
		ind2 += regionSize;
		
		if (remnants) { ++ind2; --remnants; }

		workers[i] = std::thread(runtime, ind1, ind2);
	}

	for (size_t i = 0; i < workersCount; ++i) {
		workers[i].join();
	}
	delete[] workers;
}

Dataset download_MNIST_Dataset_MultiThread(const std::string& pathToImg, 
	const std::string& pathTolabel, size_t workersCount)
{
	std::ifstream istream(pathToImg, std::ios::in | std::ios::binary);
	std::ifstream lstream(pathTolabel, std::ios::in | std::ios::binary);

	// skip useless data
	{
		istream.read((char*)buf4, 4);
		lstream.read((char*)buf4, 4);
		lstream.read((char*)buf4, 4);
	}

	int datasetSize = readInt32(istream),
		row = readInt32(istream), col = readInt32(istream),
		imgSize = row * col;

	Dataset dataset(datasetSize);
	std::vector<uint8_t> buffer(imgSize * datasetSize);

	istream.read((char*)buffer.data(), buffer.size());
	normalize_MultiThread(buffer, imgSize, dataset, workersCount);

	for (size_t i = 0; i < datasetSize; ++i)
	{
		lstream >> dataset[i].label;
		NeuralNetwork::Package& package = dataset[i];
	}
	return dataset;
}