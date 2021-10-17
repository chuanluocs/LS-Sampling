#include "slstestcasesampler.h"
#include <unistd.h>

using namespace std;

const int kMaxPbOCCSATSeed = 10000000;

SLSTestcaseSampler::SLSTestcaseSampler(string cnf_file_path, int seed): rnd_file_id_gen(getpid())
{
    cnf_file_path_ = cnf_file_path;
    seed_ = seed;
    rng_.seed(seed_);
    SetDefaultPara();
}

SLSTestcaseSampler::~SLSTestcaseSampler()
{
    delete pbo_solver_;
    delete[] map_tuple_;
}

void SLSTestcaseSampler::SetDefaultPara()
{
    testcase_set_size_ = 100;
    candidate_set_size_ = 100;

    testcase_set_.resize(testcase_set_size_);
    candidate_sample_init_solution_set_.resize(candidate_set_size_);
    candidate_testcase_set_.resize(candidate_set_size_);

    t_wise_ = 2;
    p_init_tuple_info_ = &SLSTestcaseSampler::Init2TupleInfo;
    p_update_tuple_info_ = &SLSTestcaseSampler::Update2TupleInfo;
    p_get_add_tuple_num_ = &SLSTestcaseSampler::GetAdd2TupleNum;
    p_calculate_tuple_coverage_ = &SLSTestcaseSampler::Cal2TupleCoverage;

    flag_use_weighted_sampling_ = true;
    p_init_sample_weight_ = &SLSTestcaseSampler::InitSampleWeightByAppearance;
    p_update_sample_weight_ = &SLSTestcaseSampler::UpdateSampleWeightByAppearance;

    flag_use_context_aware_ = true;
    p_init_context_aware_flip_priority_ = &SLSTestcaseSampler::InitContextAwareFlipPriority;
    p_update_context_aware_flip_priority_ = &SLSTestcaseSampler::UpdateContextAwareFlipPriorityBySampleWeight;
    flag_use_cnf_reduction_ = true;
    p_reduce_cnf_ = &SLSTestcaseSampler::ReduceCNF;
    int pos = cnf_file_path_.find_last_of('/');

    string cnf_file_name = cnf_file_path_.substr(pos + 1);
    cnf_file_name.replace(cnf_file_name.find(".cnf"), 4, "");

    reduced_cnf_file_path_ = "/tmp/" + cnf_file_name + to_string(getpid()) + to_string(rnd_file_id_gen()) + "_reduced.cnf";
    testcase_set_save_path_ = "./" + cnf_file_name + "_testcase_set.txt";
}

void SLSTestcaseSampler::SetTestcaseSetSize(int testcase_set_size)
{
    testcase_set_size_ = testcase_set_size;
    testcase_set_.resize(testcase_set_size_);
}

void SLSTestcaseSampler::SetCandidateSetSize(int candidate_set_size)
{
    candidate_set_size_ = candidate_set_size;
    candidate_sample_init_solution_set_.resize(candidate_set_size_);
    candidate_testcase_set_.resize(candidate_set_size_);
}

void SLSTestcaseSampler::SetTWise(int t_wise)
{
    t_wise_ = t_wise;
    if (t_wise_ == 2)
    {
        p_init_tuple_info_ = &SLSTestcaseSampler::Init2TupleInfo;
        p_update_tuple_info_ = &SLSTestcaseSampler::Update2TupleInfo;
        p_get_add_tuple_num_ = &SLSTestcaseSampler::GetAdd2TupleNum;
    }
    else if (t_wise_ == 3)
    {
        p_init_tuple_info_ = &SLSTestcaseSampler::Init3TupleInfo;
        p_update_tuple_info_ = &SLSTestcaseSampler::Update3TupleInfo;
        p_get_add_tuple_num_ = &SLSTestcaseSampler::GetAdd3TupleNum;
    }
}

void SLSTestcaseSampler::SetWeightedSamplingMethod(bool use_weighted_sampling)
{
    flag_use_weighted_sampling_ = use_weighted_sampling;
    if (flag_use_weighted_sampling_)
    {
        p_init_sample_weight_ = &SLSTestcaseSampler::InitSampleWeightByAppearance;
        p_update_sample_weight_ = &SLSTestcaseSampler::UpdateSampleWeightByAppearance;
    }
    else
    {
        p_init_sample_weight_ = &SLSTestcaseSampler::InitSampleWeightUniformly;
        p_update_sample_weight_ = &SLSTestcaseSampler::EmptyFunRetVoid;
    }
}

void SLSTestcaseSampler::SetContextAwareMethod(bool use_context_aware)
{
    flag_use_context_aware_ = use_context_aware;
    if (flag_use_context_aware_)
    {
        p_init_context_aware_flip_priority_ = &SLSTestcaseSampler::InitContextAwareFlipPriority;
        p_update_context_aware_flip_priority_ = &SLSTestcaseSampler::UpdateContextAwareFlipPriorityBySampleWeight;
    }
    else
    {
        p_init_context_aware_flip_priority_ = &SLSTestcaseSampler::EmptyFunRetVoid;
        p_update_context_aware_flip_priority_ = &SLSTestcaseSampler::EmptyFunRetVoid;
    }
}

void SLSTestcaseSampler::SetCNFReductionMethod(bool use_cnf_reduction)
{
    flag_use_cnf_reduction_ = use_cnf_reduction;
    if (flag_use_cnf_reduction_)
    {
        p_reduce_cnf_ = &SLSTestcaseSampler::ReduceCNF;
    }
    else
    {
        p_reduce_cnf_ = &SLSTestcaseSampler::EmptyFunRetVoid;
    }
}

void SLSTestcaseSampler::GenerateInitTestcase()
{
    bool is_sat = pbo_solver_->solve();
    num_var_ = pbo_solver_->get_var_num();
    if (is_sat)
    {
        testcase_set_[0] = pbo_solver_->get_sat_solution();
        num_generated_testcase_ = 1;
    }
    else
    {
        cout << "c PbOCCSAT Failed to Find Initial Sat Solution!" << endl;
    }
}

long long SLSTestcaseSampler::Get2TupleMapIndex(long i, long v_i, long j, long v_j)
{
    if (i >= j)
    {
        cout << "c Wrong index order!" << endl;
        return -1;
    }
    else
    {
        long long index_comb = v_j + v_i * 2;
        long long index = index_comb * num_combination_all_possible_ + (2 * num_var_ - i - 1) * i / 2 + j - i - 1;
        return index;
    }
}

long long SLSTestcaseSampler::Get3TupleMapIndex(long i, long v_i, long j, long v_j, long k, long v_k)
{
    if (i >= j || j >= k)
    {
        cout << "c Wrong index order!";
        return -1;
    }
    else
    {
        long long num_var = num_var_;
        long long index_comb = v_k + v_j * 2 + v_i * 4;
        long long index = index_comb * num_combination_all_possible_
        + (num_var * (i * num_var - (i + 2) * i) + i * (i + 1) * (i + 2) / 3) / 2 + (2 * num_var - i - j - 2) * (j - i - 1) / 2 + k - j - 1;
        return index;
    }
}

long long SLSTestcaseSampler::GetAdd2TupleNum(const vector<int> &testcase)
{
    long long num_add_tuple = 0;
    for (int i = 0; i < num_var_ - 1; i++)
    {
        for (int j = i + 1; j < num_var_; j++)
        {
            long long index_tuple = Get2TupleMapIndex(i, testcase[i], j, testcase[j]);
            if (map_tuple_[index_tuple] == 0)
            {
                num_add_tuple++;
            }
        }
    }
    return num_add_tuple;
}

long long SLSTestcaseSampler::GetAdd3TupleNum(const vector<int> &testcase)
{
    long long num_add_tuple = 0;
    for (int i = 0; i < num_var_ - 2; i++)
    {
        for (int j = i + 1; j < num_var_ - 1; j++)
        {
            for (int k = j + 1; k < num_var_; k++)
            {
                long long index_tuple = Get3TupleMapIndex(i, testcase[i], j, testcase[j], k, testcase[k]);
                if (map_tuple_[index_tuple] == 0)
                {
                    num_add_tuple++;
                }
            }
        }
    }
    return num_add_tuple;
}

int SLSTestcaseSampler::GetHammingDistance(const vector<int> &vec_1, const vector<int> &vec_2)
{
    int hamming_dis = 0;
    for (int v = 0; v < num_var_; v++)
    {
        hamming_dis += abs(vec_1[v] - vec_2[v]);
    }
    return hamming_dis;
}

void SLSTestcaseSampler::Init2TupleInfo()
{
    num_combination_all_possible_ = num_var_ * (num_var_ - 1) / 2;
    num_tuple_all_possible_ = num_combination_all_possible_ * 4;
    map_tuple_ = new int[num_tuple_all_possible_]();
    count_each_var_positive_uncovered_.resize(num_var_, (num_var_ - 1) * 2);
    count_each_var_negative_uncovered_.resize(num_var_, (num_var_ - 1) * 2);
    num_tuple_ = 0;
}

void SLSTestcaseSampler::Update2TupleInfo()
{
    int index_testcase = num_generated_testcase_ - 1;
    vector<int> testcase = testcase_set_[index_testcase];
    for (int i = 0; i < num_var_ - 1; i++)
    {
        for (int j = i + 1; j < num_var_; j++)
        {
            long long index_tuple = Get2TupleMapIndex(i, testcase[i], j, testcase[j]);

            if (map_tuple_[index_tuple] == 0)
            {
                num_tuple_++;
                if (testcase[i] == 1)
                {
                    count_each_var_positive_uncovered_[i]--;
                }
                else
                {
                    count_each_var_negative_uncovered_[i]--;
                }
                if (testcase[j] == 1)
                {
                    count_each_var_positive_uncovered_[j]--;
                }
                else
                {
                    count_each_var_negative_uncovered_[j]--;
                }
            }
            map_tuple_[index_tuple]++;
        }
    }
}

void SLSTestcaseSampler::Init3TupleInfo()
{
    long long num_var = num_var_;
    num_combination_all_possible_ = num_var * (num_var - 1) * (num_var - 2) / 6;
    num_tuple_all_possible_ = num_combination_all_possible_ * 8;
    map_tuple_ = new int[num_tuple_all_possible_]();
    count_each_var_positive_uncovered_.resize(num_var, (num_var - 1) * (num_var - 2) * 2);
    count_each_var_negative_uncovered_.resize(num_var, (num_var - 1) * (num_var - 2) * 2);
    num_tuple_ = 0;
}

void SLSTestcaseSampler::Update3TupleInfo()
{
    int index_testcase = num_generated_testcase_ - 1;
    vector<int> testcase = testcase_set_[index_testcase];
    for (int i = 0; i < num_var_ - 2; i++)
    {
        for (int j = i + 1; j < num_var_ - 1; j++)
        {
            for (int k = j + 1; k < num_var_; k++)
            {
                long long index_tuple = Get3TupleMapIndex(i, testcase[i], j, testcase[j], k, testcase[k]);

                if (map_tuple_[index_tuple] == 0)
                {
                    num_tuple_++;
                    if (testcase[i] == 1)
                    {
                        count_each_var_positive_uncovered_[i]--;
                    }
                    else
                    {
                        count_each_var_negative_uncovered_[i]--;
                    }
                    if (testcase[j] == 1)
                    {
                        count_each_var_positive_uncovered_[j]--;
                    }
                    else
                    {
                        count_each_var_negative_uncovered_[j]--;
                    }
                }
                map_tuple_[index_tuple]++;
            }
        }
    }
}

void SLSTestcaseSampler::InitSampleWeightByAppearance()
{
    var_positive_appearance_count_.resize(num_var_);
    var_positive_sample_weight_.resize(num_var_);
}

void SLSTestcaseSampler::UpdateSampleWeightByAppearance()
{
    int new_testcase_index = num_generated_testcase_ - 1;
    vector<int> new_testcase = testcase_set_[new_testcase_index];
    for (int v = 0; v < num_var_; v++)
    {
        var_positive_appearance_count_[v] += new_testcase[v];
        var_positive_sample_weight_[v] = 1. - double(var_positive_appearance_count_[v]) / num_generated_testcase_;
    }
}

void SLSTestcaseSampler::InitSampleWeightByUncoveredCount()
{
    var_positive_sample_weight_.resize(num_var_);
}

void SLSTestcaseSampler::UpdateSampleWeightByUncoveredCount()
{
    for (int v = 0; v < num_var_; v++)
    {
        var_positive_sample_weight_[v] = double(count_each_var_positive_uncovered_[v]) /
                                         (count_each_var_positive_uncovered_[v] + count_each_var_negative_uncovered_[v]);
    }
}

void SLSTestcaseSampler::InitSampleWeightUniformly()
{
    var_positive_sample_weight_.resize(num_var_, 0.5);
}

void SLSTestcaseSampler::InitContextAwareFlipPriority()
{
    context_aware_flip_priority_.resize(num_var_, 0.);
}

void SLSTestcaseSampler::UpdateContextAwareFlipPriority()
{
    vector<int> init_solution = candidate_sample_init_solution_set_[selected_candidate_index_];
    vector<int> sat_testcase = candidate_testcase_set_[selected_candidate_index_];
    vector<int> var_flip_count(num_var_);
    for (int v = 0; v < num_var_; v++)
    {
        var_flip_count[v] = abs(init_solution[v] - sat_testcase[v]);
    }
    for (int v = 0; v < num_var_; v++)
    {
        context_aware_flip_priority_[v] += var_flip_count[v];
    }
    pbo_solver_->set_var_flip_priority_ass_unaware(context_aware_flip_priority_);
}

void SLSTestcaseSampler::UpdateContextAwareFlipPriorityBySampleWeight(const vector<int> &init_solution)
{
    for (int v = 0; v < num_var_; v++)
    {
        if (init_solution[v])
            context_aware_flip_priority_[v] = var_positive_sample_weight_[v];
        else
            context_aware_flip_priority_[v] = 1. - var_positive_sample_weight_[v];
    }

    pbo_solver_->set_var_flip_priority_ass_aware(context_aware_flip_priority_);
}

void SLSTestcaseSampler::ReduceCNF()
{
    string cmd = "./bin/coprocessor -enabled_cp3 -up -subsimp -no-bve -no-bce"
                 " -no-dense -dimacs=" +
                 reduced_cnf_file_path_ + " " + cnf_file_path_;

    int return_val = system(cmd.c_str());

    cnf_file_path_ = reduced_cnf_file_path_;
}

void SLSTestcaseSampler::InitPbOCCSATSolver()
{
    pbo_solver_ = new PbOCCSATSolver(cnf_file_path_, rng_.next(kMaxPbOCCSATSeed));
}

void SLSTestcaseSampler::Init()
{
    (this->*p_reduce_cnf_)();
    InitPbOCCSATSolver();
    GenerateInitTestcase();
    (this->*p_init_tuple_info_)();
    (this->*p_init_sample_weight_)();
    (this->*p_init_context_aware_flip_priority_)();
}

vector<int> SLSTestcaseSampler::GetSatTestcaseWithGivenInitSolution(const vector<int> &init_solution)
{
    int pbo_seed = rng_.next(kMaxPbOCCSATSeed);
    pbo_solver_->set_seed(pbo_seed);

    pbo_solver_->set_init_solution(init_solution);

    (this->*p_update_context_aware_flip_priority_)(init_solution);

    bool is_sat = pbo_solver_->solve();

    if (is_sat)
    {
        return pbo_solver_->get_sat_solution();
    }
    else
    {
        cout << "c PbOCCSAT Failed to Find Sat Solution!" << endl;
    }
}

vector<int> SLSTestcaseSampler::GetWeightedSampleInitSolution()
{
    vector<int> weighted_sample_init_solution(num_var_, 0);
    for (int v = 0; v < num_var_; v++)
    {
        if (rng_.nextClosed() < var_positive_sample_weight_[v])
        {
            weighted_sample_init_solution[v] = 1;
        }
    }
    return weighted_sample_init_solution;
}

void SLSTestcaseSampler::GenerateCandidateTestcaseSet()
{
    for (int i = 0; i < candidate_set_size_; i++)
    {
        candidate_sample_init_solution_set_[i] = GetWeightedSampleInitSolution();
        candidate_testcase_set_[i] = GetSatTestcaseWithGivenInitSolution(candidate_sample_init_solution_set_[i]);
    }
}

int SLSTestcaseSampler::SelectTestcaseFromCandidateSet()
{
    int max_hamming_distance = 0;
    int best_testcase_index = 0;
    for (int i = 0; i < candidate_set_size_; i++)
    {
        int hamming_distance = 0;
        for (int j = 0; j < num_generated_testcase_; j++)
        {
            hamming_distance += GetHammingDistance(candidate_testcase_set_[i], testcase_set_[j]);
        }
        if (hamming_distance > max_hamming_distance)
        {
            best_testcase_index = i;
            max_hamming_distance = hamming_distance;
        }
    }
    return best_testcase_index;
}

int SLSTestcaseSampler::SelectTestcaseFromCandidateSetByTupleNum()
{
    int best_testcase_index = 0;
    vector<long long> new_tuple_num(candidate_set_size_, 0);
    new_tuple_num[0] = (this->*p_get_add_tuple_num_)(candidate_testcase_set_[best_testcase_index]);

    for (int i = 1; i < candidate_set_size_; i++)
    {
        new_tuple_num[i] = (this->*p_get_add_tuple_num_)(candidate_testcase_set_[i]);
        if (new_tuple_num[i] > new_tuple_num[best_testcase_index])
        {
            best_testcase_index = i;
        }
    }
    return best_testcase_index;
}

void SLSTestcaseSampler::GenerateTestcase()
{
    GenerateCandidateTestcaseSet();
    selected_candidate_index_ = SelectTestcaseFromCandidateSetByTupleNum();
    int testcase_index = num_generated_testcase_;
    testcase_set_[testcase_index] = candidate_testcase_set_[selected_candidate_index_];
}

void SLSTestcaseSampler::GenerateTestCaseSet()
{
    clock_t start_time = clock();
    Init();

    for (num_generated_testcase_ = 1; num_generated_testcase_ < testcase_set_size_; num_generated_testcase_++)
    {
        (this->*p_update_tuple_info_)();
        cout << num_generated_testcase_ << ": " << num_tuple_ << endl;
        (this->*p_update_sample_weight_)();
        GenerateTestcase();
    }
    clock_t end_time = clock();
    Update2TupleInfo();
    cpu_time_ = GetCPUTime(start_time, end_time);
    cout << "c Generate testcase set finished!" << endl;
    cout << "c CPU time cost by generating testcase set: " << cpu_time_ << " seconds" << endl;
    SaveTestcaseSet(testcase_set_save_path_);

    cout << "c " << t_wise_ << "-tuple number of generated testcase set: " << num_tuple_ << endl;

    // if (t_wise_ == 2)
    // {
    //     cout << "c Now calculate 3-tuple number of generated testcase set ..." << endl;
    //     Init3TupleInfo();
    //     for (num_generated_testcase_ = 1; num_generated_testcase_ < testcase_set_size_; num_generated_testcase_++)
    //     {
    //         Update3TupleInfo();
    //     }
    //     Update3TupleInfo();
    //     cout << "c 3-tuple number of generated testcase set: " << num_tuple_ << endl;
    // }
}

void SLSTestcaseSampler::GenerateTestCaseSetFirstReachGiven2TupleNum(long long num_2_tuple_given)
{
    cpu_time_ = 0.;
    clock_t start_time = clock();
    Init();
    clock_t end_time = clock();
    cpu_time_ += GetCPUTime(start_time, end_time);

    for (num_generated_testcase_ = 1; num_generated_testcase_ < testcase_set_size_; num_generated_testcase_++)
    {
        start_time = clock();
        Update2TupleInfo();
        if (num_tuple_ >= num_2_tuple_given)
        {
            break;
        }
        (this->*p_update_sample_weight_)();
        GenerateTestcase();
        end_time = clock();
        cpu_time_ += GetCPUTime(start_time, end_time);
    }

    Update2TupleInfo();

    if (num_tuple_ >= num_2_tuple_given)
    {
        cout << "c Generated testcase set has reached given 2-tuple number!" << endl;
        cout << "c Generated testcase number: " << num_generated_testcase_ << endl;
        cout << "c CPU time cost by generating testcase set: " << cpu_time_ << " seconds" << endl;
        cout << "c Given 2-tuple number: " << num_2_tuple_given << endl;
    }
    else
    {
        cout << "c Generated testcase set failed to reached given 2-tuple number!" << endl;
        cout << "c Generated testcase number: " << num_generated_testcase_ << endl;
        cout << "c CPU time cost by generating testcase set: " << cpu_time_ << " seconds" << endl;
        cout << "c Given 2-tuple number: " << num_2_tuple_given << endl;
    }

    SaveTestcaseSet(testcase_set_save_path_);
}

void SLSTestcaseSampler::SaveTestcaseSet(string result_path)
{
    ofstream res_file(result_path);
    for (int i = 0; i < testcase_set_size_; i++)
    {
        for (int v = 0; v < num_var_; v++)
        {
            res_file << testcase_set_[i][v] << " ";
        }
        res_file << endl;
    }
    res_file.close();
    cout << "c Testcase set saved in " << result_path << endl;
}

void SLSTestcaseSampler::Cal2TupleCoverage()
{
    cout << endl
         << "c Calculate all possible 2-tuple number & coverage ..." << endl;

    CaDiCaL::Solver *cadical_solver = new CaDiCaL::Solver;

    cadical_solver->read_dimacs(cnf_file_path_.c_str(), num_var_);

    num_tuple_all_exact_ = num_tuple_all_possible_;

    long long index_tuple;

    for (int i = 0; i < num_var_ - 1; i++)
    {
        for (int j = i + 1; j < num_var_; j++)
        {
            for (int v_i = 0; v_i < 2; v_i++)
            {
                for (int v_j = 0; v_j < 2; v_j++)
                {
                    index_tuple = Get2TupleMapIndex(i, v_i, j, v_j);
                    if (map_tuple_[index_tuple] == 0)
                    {
                        if (v_i)
                        {
                            cadical_solver->assume(i + 1);
                        }
                        else
                        {
                            cadical_solver->assume(-i - 1);
                        }
                        if (v_j)
                        {
                            cadical_solver->assume(j + 1);
                        }
                        else
                        {
                            cadical_solver->assume(-j - 1);
                        }
                        int res = cadical_solver->solve();
                        if (res == 20)
                        {
                            num_tuple_all_exact_--;
                        }
                        cadical_solver->reset_assumptions();
                    }
                }
            }
        }
    }

    coverage_tuple_ = double(num_tuple_) / num_tuple_all_exact_;

    cout << "c All possible 2-tuple number: " << num_tuple_all_exact_ << endl;
    cout << "c 2-tuple coverage: " << coverage_tuple_ << endl;

    delete cadical_solver;
}

void SLSTestcaseSampler::Cal3TupleCoverage()
{
    cout << endl
         << "c Calculate all possible 3-tuple number & coverage ..." << endl;

    CaDiCaL::Solver *cadical_solver = new CaDiCaL::Solver;

    cadical_solver->read_dimacs(cnf_file_path_.c_str(), num_var_);

    num_tuple_all_exact_ = num_tuple_all_possible_;

    long long index_tuple;

    for (int i = 0; i < num_var_ - 2; i++)
    {
        for (int j = i + 1; j < num_var_ - 1; j++)
        {
            for (int k = j + 1; k < num_var_; k++)
            {
                for (int v_i = 0; v_i < 2; v_i++)
                {
                    for (int v_j = 0; v_j < 2; v_j++)
                    {
                        for (int v_k = 0; v_k < 2; v_k++)
                        {
                            index_tuple = Get3TupleMapIndex(i, v_i, j, v_j, k, v_k);
                            if (map_tuple_[index_tuple] == 0)
                            {
                                if (v_i)
                                {
                                    cadical_solver->assume(i + 1);
                                }
                                else
                                {
                                    cadical_solver->assume(-i - 1);
                                }
                                if (v_j)
                                {
                                    cadical_solver->assume(j + 1);
                                }
                                else
                                {
                                    cadical_solver->assume(-j - 1);
                                }
                                if (v_k)
                                {
                                    cadical_solver->assume(k + 1);
                                }
                                else
                                {
                                    cadical_solver->assume(-k - 1);
                                }
                                int res = cadical_solver->solve();
                                if (res == 20)
                                {
                                    num_tuple_all_exact_--;
                                }
                                cadical_solver->reset_assumptions();
                            }
                        }
                    }
                }
            }
        }
    }

    coverage_tuple_ = double(num_tuple_) / num_tuple_all_exact_;

    cout << "c All possible 3-tuple number: " << num_tuple_all_exact_ << endl;
    cout << "c 3-tuple coverage: " << coverage_tuple_ << endl;

    delete cadical_solver;
}

void SLSTestcaseSampler::RemoveReducedCNFFile()
{
    string cmd = "rm " + reduced_cnf_file_path_;
    
    int return_val = system(cmd.c_str());
}