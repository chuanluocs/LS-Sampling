#include <signal.h>
#include "slstestcasesampler.h"

using namespace std;

void HandleInterrupt(int sig) 
{
	cout << "c" << endl;
	cout << "c caught signal... exiting" << endl;
	exit(-1);
}

void SetupSignalHandler() 
{
	signal(SIGTERM, HandleInterrupt);
	signal(SIGINT, HandleInterrupt);
	signal(SIGQUIT, HandleInterrupt);
	signal(SIGKILL, HandleInterrupt);
}

double ComputeCPUTime(clock_t start, clock_t stop)
{
	double run_time = (double)(stop - start) / CLOCKS_PER_SEC;
	return run_time;
}

struct Argument
{
	string input_cnf_path;
	string reduced_cnf_file_path;
	string output_testcase_path;
	int seed;
	int t_wise;
	int testcase_set_size; 
	int candidate_set_size;
	int use_weighted_sampling;
	int use_context_aware;
	int use_cnf_reduction;

	int is_cal_coverage;

	bool flag_input_cnf_path;
	bool flag_reduced_cnf_file_path;
	bool flag_output_testcase_path;
	bool flag_seed;
	bool flag_t_wise;
	bool flag_testcase_set_size; 
	bool flag_candidate_set_size;
	bool flag_use_weighted_sampling;
	bool flag_use_context_aware;
	bool flag_use_cnf_reduction;
};

bool ParseArgument(int argc, char **argv, Argument &argu)
{	
	argu.flag_input_cnf_path = false;
	argu.flag_reduced_cnf_file_path = false;
	argu.flag_output_testcase_path = false;
	argu.flag_seed = false;
	argu.flag_t_wise = false;
	argu.flag_testcase_set_size = false; 
	argu.flag_candidate_set_size = false;
	argu.flag_use_weighted_sampling = false;
	argu.flag_use_context_aware = false;
	argu.flag_use_cnf_reduction = false;

	argu.is_cal_coverage = 0;

	if (argc < 2)
	{
		return false;
	}

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-input_cnf_path") == 0)
		{
			i++;
			if(i>=argc) return false;
			argu.input_cnf_path = argv[i];
			argu.flag_input_cnf_path = true;
			continue;
		}
		else if (strcmp(argv[i], "-output_testcase_path") == 0)
		{
			i++;
			if(i>=argc) return false;
			argu.output_testcase_path = argv[i];
			argu.flag_output_testcase_path = true;
			continue;
		}
		else if (strcmp(argv[i], "-reduced_cnf_path") == 0)
		{
			i++;
			if(i>=argc) return false;
			argu.reduced_cnf_file_path = argv[i];
			argu.flag_reduced_cnf_file_path = true;
			continue;
		}
		else if(strcmp(argv[i], "-seed") == 0)
		{
			i++;
			if(i>=argc) return false;
			sscanf(argv[i], "%d", &argu.seed);
			argu.flag_seed = true;
			continue;
		}
		else if(strcmp(argv[i], "-t_wise") == 0)
		{
			i++;
			if(i>=argc) return false;
			sscanf(argv[i], "%d", &argu.t_wise);
			argu.flag_t_wise = true;
			continue;
		}
		else if(strcmp(argv[i], "-k") == 0)
		{
			i++;
			if(i>=argc) return false;
			sscanf(argv[i], "%d", &argu.testcase_set_size);
			argu.flag_testcase_set_size = true;
			continue;
		}
		else if(strcmp(argv[i], "-lambda") == 0)
		{
			i++;
			if(i>=argc) return false;
			sscanf(argv[i], "%d", &argu.candidate_set_size);
			argu.flag_candidate_set_size = true;
			continue;
		}
		else if(strcmp(argv[i], "-use_dynamic_updating_sampling_prob") == 0)
		{
			i++;
			if(i>=argc) return false;
			sscanf(argv[i], "%d", &argu.use_weighted_sampling);
			argu.flag_use_weighted_sampling = true;
			continue;
		}
		else if(strcmp(argv[i], "-use_diversity_aware_heuristic_search") == 0)
		{
			i++;
			if(i>=argc) return false;
			sscanf(argv[i], "%d", &argu.use_context_aware);
			argu.flag_use_context_aware = true;
			continue;
		}
		else if(strcmp(argv[i], "-use_formula_simplification") == 0)
		{
			i++;
			if(i>=argc) return false;
			sscanf(argv[i], "%d", &argu.use_cnf_reduction);
			argu.flag_use_cnf_reduction = true;
			continue;
		}
		else if(strcmp(argv[i], "-no_coverage") == 0)
		{
			argu.is_cal_coverage = 0;
			continue;
		}
		else
		{
			return false;
		}
	}

	int pos = argu.input_cnf_path.find_last_of( '/' );
    string cnf_file_name = argu.input_cnf_path.substr(pos + 1);
	cnf_file_name.replace(cnf_file_name.find(".cnf"), 4, "");

	if(argu.flag_input_cnf_path) return true;
	else return false;
}

int main(int argc, char **argv)
{
	clock_t start, stop;
	start = clock();

	SetupSignalHandler();

	Argument argu;

	if (!ParseArgument(argc, argv, argu))
	{
		cout << "c Argument Error!" << endl;
		return -1;
	}

	SLSTestcaseSampler sls_sampler(argu.input_cnf_path, argu.seed);
	if (argu.flag_t_wise)
		sls_sampler.SetTWise(argu.t_wise);
	if (argu.flag_testcase_set_size)
		sls_sampler.SetTestcaseSetSize(argu.testcase_set_size);
	if (argu.flag_candidate_set_size)
		sls_sampler.SetCandidateSetSize(argu.candidate_set_size);
	if (argu.flag_use_weighted_sampling)
		sls_sampler.SetWeightedSamplingMethod(argu.use_weighted_sampling);
	if (argu.flag_use_context_aware)
		sls_sampler.SetContextAwareMethod(argu.use_context_aware);
	if (argu.flag_use_cnf_reduction)
		sls_sampler.SetCNFReductionMethod(argu.use_cnf_reduction);
	if (argu.flag_reduced_cnf_file_path)
		sls_sampler.SetReducedCNFPath(argu.reduced_cnf_file_path);
	if (argu.flag_output_testcase_path)
		sls_sampler.SetTestcaseSetSavePath(argu.output_testcase_path);

	sls_sampler.GenerateTestCaseSet();

	if (argu.is_cal_coverage)
		if (argu.t_wise == 2 || argu.flag_t_wise == false)
			sls_sampler.Cal2TupleCoverage();
		else if (argu.t_wise == 3)
		{
			sls_sampler.Cal3TupleCoverage();
		}

	return 0;
}
