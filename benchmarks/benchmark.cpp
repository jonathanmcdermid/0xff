#include <benchmark/benchmark.h>

static void BM_Example(benchmark::State& state)
{
    int x = 0;
    int y = 1;
    for ([[maybe_unused]] auto _ : state)
    {
        int z = x + y;
        benchmark::DoNotOptimize(z);
    }

    state.counters["per_second"] = benchmark::Counter(static_cast<double>(state.iterations()), benchmark::Counter::kIsRate);
}

BENCHMARK(BM_Example);

int main(int argc, char** argv)
{
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    return EXIT_SUCCESS;
}
