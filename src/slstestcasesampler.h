#include "../include/cadical.hpp"
#include "pboccsatsolver.h"
#include <random>

class SLSTestcaseSampler
{
public:
    SLSTestcaseSampler(std::string cnf_file_path, int seed);
    ~SLSTestcaseSampler();
     
    void SetDefaultPara();
    void SetTestcaseSetSize(int testcase_set_size);
    void SetCandidateSetSize(int candidate_set_size);
    void SetTWise(int t_wise);
    void SetWeightedSamplingMethod(bool use_weighted_sampling);
    void SetContextAwareMethod(bool use_context_aware);
    void SetCNFReductionMethod(bool use_cnf_reduction);
    inline void SetReducedCNFPath(std::string reduced_cnf_path) { flag_reduced_cnf_as_temp_ = false; reduced_cnf_file_path_ = reduced_cnf_path; }
    inline void SetTestcaseSetSavePath(std::string testcase_set_path) { testcase_set_save_path_ = testcase_set_path; }
    
    void GenerateInitTestcase();
    void GenerateTestcase();
    std::vector<int> GetSatTestcaseWithGivenInitSolution(const std::vector<int>& init_solution);
    std::vector<int> GetWeightedSampleInitSolution();
    void GenerateCandidateTestcaseSet();
    int SelectTestcaseFromCandidateSet();
    int SelectTestcaseFromCandidateSetByTupleNum();
    int GetHammingDistance(const std::vector<int>& vec_1, const std::vector<int>& vec_2);

    long long Get2TupleMapIndex(long i, long v_i, long j, long v_j);
    long long Get3TupleMapIndex(long i, long v_i, long j, long v_j, long k, long v_k);

    long long GetAdd2TupleNum(const std::vector<int> &testcase);
    long long GetAdd3TupleNum(const std::vector<int> &testcase);

    void Init2TupleInfo();
    void Update2TupleInfo();

    void Init3TupleInfo();
    void Update3TupleInfo();

    void InitPbOCCSATSolver();

    void InitSampleWeightByAppearance();
    void UpdateSampleWeightByAppearance();
    void InitSampleWeightByUncoveredCount();
    void UpdateSampleWeightByUncoveredCount();
    void InitSampleWeightUniformly();

    void InitContextAwareFlipPriority();
    void UpdateContextAwareFlipPriority();
    void UpdateContextAwareFlipPriorityBySampleWeight(const std::vector<int> &init_solution);

    void ReduceCNF();

    void Init();
    
    void GenerateTestCaseSet();

    void GenerateTestCaseSetFirstReachGiven2TupleNum(long long tuple_num);

    inline long long GetTupleNum() { return num_tuple_; }
    void Cal2TupleCoverage();
    void Cal3TupleCoverage();
    inline long long GetExactAllTupleNum() { return num_tuple_all_exact_; }
    inline long long GetTupleCoverage() { return coverage_tuple_; }

    void SaveTestcaseSet(std::string result_path);
    
    inline double GetCPUTime(clock_t start, clock_t stop)
    {
	    return ((double)(stop - start) / CLOCKS_PER_SEC);
    }
    
private:
    std::string cnf_file_path_;
    std::string reduced_cnf_file_path_;
    std::string testcase_set_save_path_;
    int seed_;
    int t_wise_;
    double cpu_time_;
    int testcase_set_size_;
    int candidate_set_size_;
    bool flag_use_weighted_sampling_;
    bool flag_use_context_aware_;
    bool flag_use_cnf_reduction_;
    bool flag_reduced_cnf_as_temp_;

    std::mt19937_64 rnd_file_id_gen;

    PbOCCSATSolver *pbo_solver_;

    Mersenne rng_;
    int num_var_;
    int num_generated_testcase_;
    int selected_candidate_index_;

    int *map_tuple_;
    long long num_tuple_;
    long long num_combination_all_possible_;
    long long num_tuple_all_possible_;
    long long num_tuple_all_exact_;
    double coverage_tuple_;

    std::vector<std::vector<int>> testcase_set_;
    std::vector<std::vector<int>> candidate_testcase_set_;
    std::vector<std::vector<int>> candidate_sample_init_solution_set_;
    std::vector<int> var_positive_appearance_count_;
    std::vector<double> var_positive_sample_weight_;
    std::vector<double> context_aware_flip_priority_;

    std::vector<int> count_each_var_positive_uncovered_;
    std::vector<int> count_each_var_negative_uncovered_;

    void (SLSTestcaseSampler::*p_init_context_aware_flip_priority_)();
    void (SLSTestcaseSampler::*p_update_context_aware_flip_priority_)(const std::vector<int> &init_solution);
    void (SLSTestcaseSampler::*p_init_sample_weight_)();
    void (SLSTestcaseSampler::*p_update_sample_weight_)();
    void (SLSTestcaseSampler::*p_reduce_cnf_)();

    long long (SLSTestcaseSampler::*p_get_add_tuple_num_)(const std::vector<int> &testcase);
    void (SLSTestcaseSampler::*p_init_tuple_info_)();
    void (SLSTestcaseSampler::*p_update_tuple_info_)();
    void (SLSTestcaseSampler::*p_calculate_tuple_coverage_)();

    inline void EmptyFunRetVoid() {}
    inline void EmptyFunRetVoid(const std::vector<int>& init_solution) {}

    void RemoveReducedCNFFile();
};