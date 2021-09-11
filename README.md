# LS-Sampling: An Effective Local Search Based Sampling Approach for Achieving High t-wise Coverage

LS-Sampling is an effective local search based sampling approach for solving the t-wise coverage maximum (t-wise CovMax) problem. This repository includes the implementation of LS-Sampling, testing benchmarks and the experimental results. We note that LS-Sampling should be run on an operating system of `GNU/Linux (64-bit)`.

## Developers
- Chuan Luo (<chuanluophd@outlook.com>)
- Binqi Sun (<binqi.sun@tum.de>)


## Reference
- Chuan Luo, Binqi Sun, Bo Qiao, Junjie Chen, Hongyu Zhang, Jinkun Lin, Qingwei Lin, Dongmei Zhang. LS-Sampling: An Effective Local Search based Sampling Approach for Achieving High t-wise Coverage. Proceedings of ESEC/FSE 2021: 1081-1092, 2021.


## How to Build LS-Sampling

```
make
```


## How to Run LS-Sampling

```
./LS-Sampling -input_cnf_path [BENCHMARK_PATH] -seed [SEED] -k [ALLOWED_NUMBER_TEST_CASES] -lambda 100 -use_formula_simplification 1 -use_dynamic_updating_sampling_prob 1 -use_diversity_aware_heuristic_search 1
```


## Example Command of Calling LS-Sampling

```
./LS-Sampling -input_cnf_path cnf_benchmarks/linux.cnf -seed 1 -k 100 -lambda 100 -use_formula_simplification 1 -use_dynamic_updating_sampling_prob 1 -use_diversity_aware_heuristic_search 1
```

- The above example command represents calling LS-Sampling to solve benchmark `cnf_benchmarks/linux.cnf` by setting the random seed to 1 and the allowed number of test cases to 100.


## Implementation of LS-Sampling

- The implementation of LS-Sampling can be found in the directory entitled `src`.


## Testing Benchmarks for Evaluating LS-Sampling

- The directory entitled `cnf_benchmarks/` includes all testing benchmarks.


## Results of 2-wise Coverage achieved by all Competing Approaches with both k=100 and k=1000 on all Testing Benchmarks

- Results of 2-wise coverage achieved by LS-Sampling and its state-of-the-art competitors (i.e., uniform sampling and Baital) with both k=100 and k=1000 on all testing benchmarks are presented in the csv file `experimental_results/Results_of_2-wise_coverage_LS-Sampling_and_its_SOTA_competitors.csv`.

- Results of 2-wise coverage achieved by LS-Sampling and its alternative versions with both k=100 and k=1000 on all testing benchmarks are presented in the csv file `experimental_results/Results_of_2-wise_coverage_LS-Sampling_and_its_alternative_versions.csv`.

- Results of 2-wise coverage achieved by LS-Sampling with different hyper-parameter settings with both k=100 and k=1000 on all testing benchmarks are presented in the csv file `experimental_results/Results_of_2-wise_coverage_LS-Sampling_with_different_hyper-parameter_settings.csv`.


## Results of 3-wise Coverage achieved by all Competing Approaches with both k=100 and k=1000 on all Testing Benchmarks

- Results of 3-wise coverage achieved by LS-Sampling and its state-of-the-art competitors (i.e., uniform sampling and Baital) with both k=100 and k=1000 on all testing benchmarks are presented in the csv file `experimental_results/Results_of_3-wise_coverage_LS-Sampling_and_its_SOTA_competitors.csv`.

- Results of 3-wise coverage achieved by LS-Sampling and its alternative versions with both k=100 and k=1000 on all testing benchmarks are presented in the csv file `experimental_results/Results_of_3-wise_coverage_LS-Sampling_and_its_alternative_versions.csv`.

- Results of 3-wise coverage achieved by LS-Sampling with different hyper-parameter settings with both k=100 and k=1000 on all testing benchmarks are presented in the csv file `experimental_results/Results_of_3-wise_coverage_LS-Sampling_with_different_hyper-parameter_settings.csv`.